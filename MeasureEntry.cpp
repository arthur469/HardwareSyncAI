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

    connect(&timer, &QTimer::timeout, this, &MeasureEntry::Tick);
}

MeasureEntry::~MeasureEntry()
{
    timer.stop();
    delete ui;
}

void MeasureEntry::Start()
{
    timer.setInterval(refresh_interval);
    timer.start();
    ui->start_stop->setText("Stop");
}

void MeasureEntry::Stop()
{
    timer.stop();
    ui->start_stop->setText("Start");
}

void MeasureEntry::on_refresh_interval_valueChanged(int value)
{
    refresh_interval = value;
    timer.setInterval(refresh_interval);
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

    for(ControllerZone* controller_zone: OpenRGBHardwareSyncPlugin::controller_zones)
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
        controller_zones.clear();

        for(unsigned int i = 0; i < OpenRGBHardwareSyncPlugin::controller_zones.size(); i++)
        {
            if(list->selectionModel()->isRowSelected(i, QModelIndex()))
            {
                controller_zones.push_back(OpenRGBHardwareSyncPlugin::controller_zones[i]);
            }
        }
    }

}

void MeasureEntry::on_start_stop_clicked()
{
    if(timer.isActive())
    {
        Stop();
    }
    else
    {
        Start();
    }
}

void MeasureEntry::Tick()
{
    double measure = OpenRGBHardwareSyncPlugin::hm->GetMeasure(
                devices[hardware_id],
                devices[hardware_id].features[hardware_feature_id]);

    std::tuple<int, QColor> data = ui->measure_color->GetColor(measure, last_idx);

    last_idx = std::get<0>(data);

    QColor color = std::get<1>(data);

    RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());

    ui->measure->setText(QString::fromStdString(std::to_string(measure)));
    ui->color_preview->setStyleSheet("QLabel {background-color: "+ color.name() + "; border: 1px solid black;}");

    std::set<RGBController*> controllers;

    for(ControllerZone* controller_zone: controller_zones)
    {
        controller_zone->controller->SetAllZoneLEDs(controller_zone->zone_idx, rgb);
        controllers.insert(controller_zone->controller);
    }

    for(RGBController* controller: controllers)
    {
        controller->UpdateLEDs();
    }
}

json MeasureEntry::ToJson()
{
    json j;

    j["refresh_interval"] = refresh_interval;
    j["hardware"] = ui->hardware->currentIndex();
    j["hardware_feature"] = ui->hardware_feature->currentIndex();
    j["measure_color"] = ui->measure_color->ToJson();

    std::vector<json> devices;

    for(ControllerZone* controller_zone: controller_zones)
    {
       devices.push_back(controller_zone->to_json());
    }

    j["devices"] = devices;

    return j;
}
