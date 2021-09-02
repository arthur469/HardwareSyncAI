#include "MeasureEntry.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QDialog>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <set>

MeasureEntry::MeasureEntry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeasureEntry)
{
    ui->setupUi(this);

    devices = OpenRGBHardwareSyncPlugin::hm->GetAvailableDevices();

    for(Hardware& device: devices)
    {
        ui->hardware->addItem(QString::fromStdString(device.TrackDeviceName));
    }

    connect(this, SIGNAL(Measure(double)), this, SLOT(OnMeasure(double)));
    connect(this, SIGNAL(Color(QColor)), this, SLOT(OnColor(QColor)));
}

MeasureEntry::~MeasureEntry()
{
    Stop();
    delete ui;
}

void MeasureEntry::Clear()
{
    assigned_zones.clear();
}

void MeasureEntry::Start()
{
    if(running)
    {
        return;
    }

    running = true;
    measure_thread = new std::thread(&MeasureEntry::Tick, this);
    leds_thread = new std::thread(&MeasureEntry::UpdateLEDsThread, this);

    ui->start_stop->setText("Stop");
}

void MeasureEntry::Stop()
{
    if(running)
    {
        running = false;

        measure_thread->join();
        delete measure_thread;

        leds_thread->join();
        delete leds_thread;

        ui->start_stop->setText("Start");
    }
}

void MeasureEntry::on_refresh_interval_valueChanged(int value)
{
    refresh_interval = value;
}

void MeasureEntry::on_hardware_currentIndexChanged(int value)
{
    hardware_id = value;
    ui->hardware_feature->clear();

    for(HardwareFeature& feature: devices[hardware_id].features)
    {
        ui->hardware_feature->addItem(QString::fromStdString(feature.Name));
    }
}

void MeasureEntry::on_hardware_feature_currentIndexChanged(int value)
{
    hardware_feature_id = value;
}

void MeasureEntry::on_remove_clicked()
{
    emit RemoveRequest();
}

void MeasureEntry::on_devices_clicked()
{
    QDialog dialog(this);
    QFormLayout form(&dialog);
    QListView* list = new QListView(this);
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    QStringListModel model;
    QStringList devices;

    std::vector<ControllerZone*> controller_zones = OpenRGBHardwareSyncPlugin::GetControllerZones();

    for(ControllerZone* controller_zone: controller_zones)
    {
        devices << QString::fromStdString(controller_zone->display_name());
    }

    model.setStringList(devices);

    list->setModel(&model);

    form.addRow(list);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    QObject::connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
    QObject::connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

    if (dialog.exec() == QDialog::Accepted) {

        assigned_zones.clear();

        for(unsigned int i = 0; i < controller_zones.size(); i++)
        {
            if(list->selectionModel()->isRowSelected(i, QModelIndex()))
            {
                assigned_zones.push_back(controller_zones[i]);
            }
        }
    }

}

void MeasureEntry::on_start_stop_clicked()
{
    if(running)
    {
        Stop();
    }
    else
    {
        Start();
    }
}

void MeasureEntry::OnMeasure(double measure)
{
    ui->measure->setText(QString::fromStdString(std::to_string(measure)));
}

void MeasureEntry::OnColor(QColor color)
{
    ui->color_preview->setStyleSheet("QLabel {background-color: "+ color.name() + "; border: 1px solid black;}");
}

void MeasureEntry::UpdateLEDsThread()
{
    while(running)
    {
        std::tuple<int, QColor> data = ui->measure_color->GetColor(measure, last_idx);

        last_idx = std::get<0>(data);

        QColor color = std::get<1>(data);

        emit Color(color);

        RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());

        std::set<RGBController*> controllers;

        for(ControllerZone* controller_zone: assigned_zones)
        {
            controller_zone->controller->SetAllZoneLEDs(controller_zone->zone_idx, rgb);
            controllers.insert(controller_zone->controller);
        }

        for(RGBController* controller: controllers)
        {
            controller->UpdateLEDs();
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(leds_refresh_interval));
    }
}

void MeasureEntry::Tick()
{
    while(running)
    {
        measure = OpenRGBHardwareSyncPlugin::hm->GetMeasure(
                    devices[hardware_id],
                    devices[hardware_id].features[hardware_feature_id]);

        emit Measure(measure);

        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_interval));
    }
}

json MeasureEntry::ToJson()
{
    json j;

    j["refresh_interval"] = refresh_interval;
    j["hardware"] = ui->hardware->currentIndex();
    j["hardware_feature"] = ui->hardware_feature->currentIndex();
    j["measure_color"] = ui->measure_color->ToJson();
    j["running"] = running;

    std::vector<json> devices;

    for(ControllerZone* controller_zone: assigned_zones)
    {
        devices.push_back(controller_zone->to_json());
    }

    j["devices"] = devices;

    return j;
}

MeasureEntry* MeasureEntry::FromJson(QWidget *parent, json settings)
{
    MeasureEntry* measure = new MeasureEntry(parent);

    if(settings.contains("refresh_interval"))
        measure->ui->refresh_interval->setValue(settings["refresh_interval"]);

    if(settings.contains("hardware"))
        measure->ui->hardware->setCurrentIndex(settings["hardware"]);

    if(settings.contains("hardware_feature"))
        measure->ui->hardware_feature->setCurrentIndex(settings["hardware_feature"]);

    if(settings.contains("measure_color"))
        measure->ui->measure_color->LoadJson(settings["measure_color"]);

    if(settings.contains("devices"))
    {
        json zones = settings["devices"];

        for(auto j : zones)
        {
            for(ControllerZone* controller_zone: OpenRGBHardwareSyncPlugin::GetControllerZones())
            {
                if(
                        controller_zone->controller->name        == j["name"] &&
                        controller_zone->controller->location    == j["location"] &&
                        controller_zone->controller->serial      == j["serial"] &&
                        controller_zone->controller->description == j["description"] &&
                        controller_zone->controller->version     == j["version"] &&
                        controller_zone->controller->vendor      == j["vendor"] &&
                        controller_zone->zone_idx                == j["zone_idx"]
                        )
                {
                    measure->assigned_zones.push_back(controller_zone);
                    break;
                }
            }
        }
    }

    if(settings.contains("running"))
    {
        bool was_running = settings["running"];

        if(was_running)
        {
            measure->Start();
        }
    }

    return measure;
}

