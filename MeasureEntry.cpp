#include "MeasureEntry.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include <QStringList>
#include <QStringListModel>
#include <QListView>
#include <QDialog>
#include <QCheckBox>
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

void MeasureEntry::Rename(std::string value)
{
    name = value;
    emit Renamed(value);
}

std::string MeasureEntry::GetName()
{
    return name;
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

void MeasureEntry::on_fps_valueChanged(int value)
{
    fps = value;
}

void MeasureEntry::on_hardware_currentIndexChanged(int value)
{
    hardware_id = value;
    ui->hardware_feature->clear();

    for(HardwareFeature& feature: devices[hardware_id].features)
    {
        ui->hardware_feature->addItem(QString::fromStdString(feature.Name), QString::fromStdString(feature.Identifier));
    }
}

void MeasureEntry::on_hardware_feature_currentIndexChanged(int value)
{
    hardware_feature_id = value;
}

void MeasureEntry::on_percentage_fill_stateChanged(int state)
{
    percentage_fill = state;
}

void MeasureEntry::on_auto_start_stateChanged(int state)
{
    auto_start = state;
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

    if (dialog.exec() == QDialog::Accepted)
    {
        assigned_zones.clear();
        ui->devices_list_view->clear();

        for(unsigned int i = 0; i < controller_zones.size(); i++)
        {
            if(list->selectionModel()->isRowSelected(i, QModelIndex()))
            {
                assigned_zones.push_back(controller_zones[i]);
                ui->devices_list_view->addItem(QString::fromStdString(controller_zones[i]->display_name()));
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
        std::tuple<float, QColor> data = ui->measure_color->GetColor(measure, last_idx, fps);

        last_idx = std::get<0>(data);

        QColor color = std::get<1>(data);

        emit Color(color);

        std::set<RGBController*> controllers;

        if(percentage_fill) {

            int maxRange = ui->measure_color->max_value - ui->measure_color->min_value;

            for(ControllerZone* controller_zone: assigned_zones)
            {
                unsigned int ledsCount = controller_zone->leds_count();
                int ledScale = maxRange / ledsCount;
                int maxLed = (measure - ui->measure_color->min_value) / ledScale;
                for(int i = 0; i < maxLed; ++i) {
                    int val = i * ledScale;
                    int idx = std::max<int>(0, std::min<int>(99, 100 * ((val - ui->measure_color->min_value) / (ui->measure_color->max_value - ui->measure_color->min_value))));
                    std::tuple<float, QColor> data = ui->measure_color->GetColor(val, idx, fps);

                    QColor color = std::get<1>(data);
                    RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());
                    controller_zone->controller->zones[controller_zone->zone_idx].colors[i] = rgb;
                }
                for(int i = maxLed; i < ledsCount; i++) {
                    controller_zone->controller->zones[controller_zone->zone_idx].colors[i] = 0;
                }

                controllers.insert(controller_zone->controller);
            }
        }
        else{
            RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());

            for(ControllerZone* controller_zone: assigned_zones)
            {
                controller_zone->controller->SetAllZoneLEDs(controller_zone->zone_idx, rgb);
                controllers.insert(controller_zone->controller);
            }
        }

        for(RGBController* controller: controllers)
        {
            controller->UpdateLEDs();
        }

        unsigned int sleep = 1000./fps;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep));
    }
}

void MeasureEntry::Tick()
{
    while(running)
    {
        if(hardware_id < devices.size() && hardware_feature_id < devices[hardware_id].features.size())
        {
            measure = OpenRGBHardwareSyncPlugin::hm->GetMeasure(
                        devices[hardware_id],
                        devices[hardware_id].features[hardware_feature_id]);

            emit Measure(measure);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(refresh_interval));
    }
}

json MeasureEntry::ToJson()
{
    json j;

    j["refresh_interval"] = refresh_interval;
    j["hardware"] = ui->hardware->currentIndex();
    j["hardware_feature"] = ui->hardware_feature->currentIndex();
#ifdef _WIN32
    j["hardware_identifier"] = ui->hardware_feature->currentData().toString().toStdString();
#endif
    j["measure_color"] = ui->measure_color->ToJson();
    j["running"] = running;
    j["percentage_fill"] = percentage_fill;
    j["auto_start"] = auto_start;
    j["fps"] = fps;
    j["name"] = name;

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

#ifdef _WIN32
    if (settings.contains("hardware_identifier"))
    {
        measure->ui->hardware->setCurrentIndex(OpenRGBHardwareSyncPlugin::hm->HardwareMeasure::GetHardwareIndex(settings["hardware_identifier"]));
        measure->ui->hardware_feature->setCurrentIndex(OpenRGBHardwareSyncPlugin::hm->HardwareMeasure::GetHardwareFeatureIndex(settings["hardware_identifier"]));
    }
    else
    {
        if(settings.contains("hardware"))
            measure->ui->hardware->setCurrentIndex(settings["hardware"]);

        if(settings.contains("hardware_feature"))
            measure->ui->hardware_feature->setCurrentIndex(settings["hardware_feature"]);
    }
#else
    if(settings.contains("hardware"))
        measure->ui->hardware->setCurrentIndex(settings["hardware"]);

    if(settings.contains("hardware_feature"))
        measure->ui->hardware_feature->setCurrentIndex(settings["hardware_feature"]);
#endif

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

    if(settings.contains("percentage_fill"))
    {
        measure->ui->percentage_fill->setChecked(settings["percentage_fill"]);
    }

    if(settings.contains("fps"))
    {
        measure->ui->fps->setValue(settings["fps"]);
    }

    if(settings.contains("name"))
    {
        measure->name = settings["name"];
    }

    if(settings.contains("auto_start"))
    {       
        measure->ui->auto_start->setChecked(settings["auto_start"]);

        if(measure->auto_start)
        {
            measure->Start();
        }
    }

    for(unsigned int i = 0; i < measure->assigned_zones.size(); i++)
    {
       measure->ui->devices_list_view->addItem(QString::fromStdString(measure->assigned_zones[i]->display_name()));
    }

    return measure;
}

