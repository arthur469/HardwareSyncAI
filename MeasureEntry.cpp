#include "ControllerZoneListItemWidget.h"
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

    /*---------------------------------------------------------*\
    | Populate the hardware combobox (measures)                 |
    \*---------------------------------------------------------*/
    devices = OpenRGBHardwareSyncPlugin::hm->GetAvailableDevices();

    for(Hardware& device: devices)
    {
        ui->hardware->addItem(QString::fromStdString(device.TrackDeviceName));
    }

    /*---------------------------------------------------------*\
    | Internal bindings cause of multi-threading                |
    \*---------------------------------------------------------*/
    connect(this, SIGNAL(Measure(double)), this, SLOT(OnMeasure(double)));
    connect(this, SIGNAL(Color(QColor)), this, SLOT(OnColor(QColor)));
}

MeasureEntry::~MeasureEntry()
{
    /*---------------------------------------------------------*\
    | Stop measures in destructor                               |
    \*---------------------------------------------------------*/
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

    /*---------------------------------------------------------*\
    | Start measure and leds update threads                     |
    \*---------------------------------------------------------*/
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

        /*---------------------------------------------------------*\
        | Wait for the threads to finish                            |
        \*---------------------------------------------------------*/
        measure_thread->join();
        delete measure_thread;

        leds_thread->join();
        delete leds_thread;

        ui->start_stop->setText("Start");
    }
}

bool MeasureEntry::IsRunning()
{
    return running;
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
    /*---------------------------------------------------------*\
    | Set the sub features on hardware changed                  |
    \*---------------------------------------------------------*/
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
    /*---------------------------------------------------------*\
    | Update % checkboxes with the global state                 |
    \*---------------------------------------------------------*/
    for(int i = 0; i < ui->devices_list_view->count(); i++)
    {
        ControllerZoneListItemWidget* custom_widget =
                (ControllerZoneListItemWidget*) ui->devices_list_view->itemWidget(ui->devices_list_view->item(i));

        custom_widget->setPercentageFill(state);
    }
}

void MeasureEntry::on_auto_start_stateChanged(int state)
{
    auto_start = state;
}

void MeasureEntry::on_devices_clicked()
{
    /*---------------------------------------------------------*\
    | Open a dialog to choose which RGB devices to assign       |
    \*---------------------------------------------------------*/
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

        /*---------------------------------------------------------*\
        | Create the devices list widgets                           |
        \*---------------------------------------------------------*/
        for(unsigned int i = 0; i < controller_zones.size(); i++)
        {
            if(list->selectionModel()->isRowSelected(i, QModelIndex()))
            {
                assigned_zones.push_back(controller_zones[i]);

                QListWidgetItem* item = new QListWidgetItem();
                ControllerZoneListItemWidget* custom_widget =
                        new ControllerZoneListItemWidget(
                            QString::fromStdString(controller_zones[i]->display_name()),
                            false,
                            ui->devices_list_view
                            );

                item->setSizeHint(custom_widget->sizeHint());

                ui->devices_list_view->addItem(item);
                ui->devices_list_view->setItemWidget(item, custom_widget);
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

        for(unsigned int z = 0; z < assigned_zones.size(); z++)
        {
            ControllerZoneListItemWidget* custom_widget =
                    (ControllerZoneListItemWidget*) ui->devices_list_view->itemWidget(ui->devices_list_view->item(z));

            if(custom_widget->isPercentageFill())
            {
                int maxRange = ui->measure_color->max_value - ui->measure_color->min_value;
                unsigned int ledsCount = assigned_zones[z]->leds_count();
                double ledScale = 1.0 * maxRange / ledsCount;
                double maxLed = (measure - ui->measure_color->min_value) / ledScale;

                for(int i = 0; i < maxLed; ++i)
                {
                    double val = i * ledScale;
                    float idx = std::max<float>(0, std::min<float>(99, 100 * (val / (maxRange))));
                    std::tuple<float, QColor> data = ui->measure_color->GetColor(val, idx, fps);
                    QColor color = std::get<1>(data);
                    RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());
                    assigned_zones[z]->controller->zones[assigned_zones[z]->zone_idx].colors[i] = rgb;
                }

                for(unsigned int i = maxLed; i < ledsCount; i++)
                {
                    assigned_zones[z]->controller->zones[assigned_zones[z]->zone_idx].colors[i] = 0;
                }

                controllers.insert(assigned_zones[z]->controller);
            }

            else
            {
                RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());
                assigned_zones[z]->controller->SetAllZoneLEDs(assigned_zones[z]->zone_idx, rgb);
                controllers.insert(assigned_zones[z]->controller);
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
    j["auto_start"] = auto_start;
    j["fps"] = fps;
    j["name"] = name;

    std::vector<json> devices;

    for(ControllerZone* controller_zone: assigned_zones)
    {
        devices.push_back(controller_zone->to_json());
    }

    j["devices"] = devices;

    std::vector<bool> devices_percentage_fill;

    for(int i = 0; i < ui->devices_list_view->count(); i++)
    {
        ControllerZoneListItemWidget* custom_widget =
                (ControllerZoneListItemWidget*) ui->devices_list_view->itemWidget(ui->devices_list_view->item(i));

        devices_percentage_fill.push_back(custom_widget->isPercentageFill());
    }

    j["devices_percentage_fill"] = devices_percentage_fill;
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
                auto location = j["location"].get<std::string>();
                bool ignore_location = (location.find("HID: ") == 0);
                if(
                        controller_zone->controller->name         == j["name"] &&
                        (
                            ignore_location ||
                            controller_zone->controller->location == j["location"]
                            ) &&
                        controller_zone->controller->serial       == j["serial"] &&
                        controller_zone->controller->description  == j["description"] &&
                        controller_zone->controller->version      == j["version"] &&
                        controller_zone->controller->vendor       == j["vendor"] &&
                        controller_zone->zone_idx                 == j["zone_idx"]
                        )
                {
                    measure->assigned_zones.push_back(controller_zone);

                    break;
                }
            }
        }
    }


    if(settings.contains("fps"))
    {
        measure->ui->fps->setValue(settings["fps"]);
    }

    if(settings.contains("name"))
    {
        measure->name = settings["name"];
    }

    std::vector<bool> devices_percentage_fill;

    if(settings.contains("devices_percentage_fill"))
    {
        for(bool b: settings["devices_percentage_fill"])
        {
            devices_percentage_fill.push_back(b);
        }
    }
    else
    {
        devices_percentage_fill.resize(measure->assigned_zones.size());
    }

    for(unsigned int i = 0; i < measure->assigned_zones.size(); i++)
    {
        QListWidgetItem* item = new QListWidgetItem();
        ControllerZoneListItemWidget* custom_widget =
                new ControllerZoneListItemWidget(
                    QString::fromStdString(measure->assigned_zones[i]->display_name()),
                    devices_percentage_fill[i],
                    measure->ui->devices_list_view
                    );

        item->setSizeHint(custom_widget->sizeHint());

        measure->ui->devices_list_view->addItem(item);
        measure->ui->devices_list_view->setItemWidget(item, custom_widget);
    }

    if(settings.contains("auto_start"))
    {
        measure->ui->auto_start->setChecked(settings["auto_start"]);

        if(measure->auto_start)
        {
            measure->Start();
        }
    }

    return measure;
}

