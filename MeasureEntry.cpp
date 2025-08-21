#include "MeasureEntry.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include "ControllerZoneManager.h"
#include "OpenRGBPluginsFont.h"
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

    ui->brightness_label->setFont(OpenRGBPluginsFont::GetFont());
    ui->toggle_brightness->setFont(OpenRGBPluginsFont::GetFont());

    ui->brightness_label->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::sun));
    ui->toggle_brightness->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::sun));

    ui->devices_list->setLayout(new QVBoxLayout(ui->devices_list));

    InitDevicesList();

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

void MeasureEntry::InitDevicesList()
{
    /*---------------------------------------------------------*\
    | Populate the device list                                  |
    \*---------------------------------------------------------*/
    controller_zones = ControllerZoneManager::GetControllerZones();

    for(ControllerZone* controller_zone: controller_zones)
    {
        ControllerZoneListItemWidget* controller_zones_widget = new ControllerZoneListItemWidget(ui->devices_list, controller_zone);
        ui->devices_list->layout()->addWidget(controller_zones_widget);
        controller_zones_widgets[controller_zone] = controller_zones_widget;
    }
}

void MeasureEntry::Clear()
{
    /*---------------------------------------------------------*\
    | Clear all widgets and memory                              |
    \*---------------------------------------------------------*/
    controller_zones_widgets.clear();
    controller_zones.clear();

    QLayoutItem *child;

    while ((child = ui->devices_list->layout()->takeAt(0)) != 0)
    {
        delete child->widget();
    }
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
    /*---------------------------------------------------------*\
    | Make sure the controllers are set to direct mode          |
    \*---------------------------------------------------------*/   
    for(ControllerZone* controller_zone: controller_zones)
    {
        if(controller_zone->enabled)
        {
            for(unsigned int i = 0 ; i < controller_zone->controller->modes.size(); i++)
            {
                if(controller_zone->controller->modes[i].name == "Direct")
                {
                    controller_zone->controller->SetMode(i);
                    break;
                }
            }
        }
    }

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

    emit MeasureState(true);
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

        emit MeasureState(false);
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

void MeasureEntry::on_brightness_valueChanged(int value)
{
    brightness = value;
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

void MeasureEntry::on_percentage_fill_all_toggled(bool state)
{
    /*---------------------------------------------------------*\
    | Update % checkboxes with the global state                 |
    \*---------------------------------------------------------*/
    auto children = ui->devices_list->findChildren<ControllerZoneListItemWidget *>();

    for (auto custom_widget : children)
    {
        custom_widget->setPercentageFill(state);
    }
}

void MeasureEntry::on_enable_all_toggled(bool state)
{
    /*---------------------------------------------------------*\
    | Update enabled checkboxes with the global state           |
    \*---------------------------------------------------------*/
    auto children = ui->devices_list->findChildren<ControllerZoneListItemWidget *>();

    for (auto custom_widget : children)
    {
        custom_widget->setEnabled(state);
    }
}

void MeasureEntry::on_toggle_brightness_toggled(bool state)
{
    /*---------------------------------------------------------*\
    | Update brightness slider visibility                       |
    \*---------------------------------------------------------*/
    auto children = ui->devices_list->findChildren<ControllerZoneListItemWidget *>();

    for (auto custom_widget : children)
    {
        custom_widget->showBrightnessSlider(state);
    }
}

void MeasureEntry::on_reverse_all_toggled(bool state)
{
    /*---------------------------------------------------------*\
    | Update reverse checkboxes with the global state           |
    \*---------------------------------------------------------*/
    auto children = ui->devices_list->findChildren<ControllerZoneListItemWidget *>();

    for (auto custom_widget : children)
    {
        custom_widget->setReverse(state);
    }
}

void MeasureEntry::on_auto_start_stateChanged(int state)
{
    auto_start = state;
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
    /*---------------------------------------------------------*\
    | LEDs thread                                               |
    \*---------------------------------------------------------*/
    while(running)
    {
        std::tuple<float, QColor> data = ui->measure_color->GetColor(measure, last_idx, fps);

        last_idx = std::get<0>(data);

        QColor color = std::get<1>(data);

        emit Color(color);

        std::set<RGBController*> controllers;

        int maxRange = ui->measure_color->max_value - ui->measure_color->min_value;
        float global_brightness = brightness/100.0;

        for(ControllerZone* controller_zone: controller_zones)
        {
            if(!controller_zone->enabled)
            {
                continue;
            }

            float b = global_brightness * controller_zone->brightness * 0.01;

            /*---------------------------------------------------------*\
            | Percentage fill mode                                      |
            \*---------------------------------------------------------*/
            unsigned int startIdx = controller_zone->start_idx();
            unsigned int ledsCount = controller_zone->leds_count();
            unsigned int zoneLedCount =  controller_zone->controller->zones[controller_zone->zone_idx].leds_count;
            if(controller_zone->is_percentage_fill)
            {
                double ledScale = 1.0 * maxRange / ledsCount;
                double maxLed = 0.01 * last_idx * ledsCount;
                bool reverse = controller_zone->reverse;

                for(unsigned int i = 0; i < ledsCount; ++i)
                {

                    RGBColor rgb = 0;
                    int led_id = reverse ? ledsCount - i - 1 : i;

                    if(i <= maxLed)
                    {
                        double val = i * ledScale;
                        float idx = std::max<float>(0, std::min<float>(99, 100 * (val / (maxRange))));
                        std::tuple<float, QColor> data = ui->measure_color->GetColor(val, idx, fps);
                        QColor color = std::get<1>(data);

                        float dimm = i == (unsigned int) maxLed ? maxLed - (unsigned int) maxLed : 1.0;
                        rgb = ToRGBColor((int)(b*dimm*color.red()), (int)(b*dimm*color.green()), (int)(b*dimm*color.blue()));
                    }

                    if (controller_zone->is_segment)
                    {
                        if (startIdx + led_id >= zoneLedCount)
                        {
                            break;
                        }
                        controller_zone->controller->zones[controller_zone->zone_idx].colors[startIdx + led_id] = rgb;
                    }
                    else
                    {
                        controller_zone->controller->zones[controller_zone->zone_idx].colors[led_id] = rgb;
                    }
                }

                controllers.insert(controller_zone->controller);
            }
            /*---------------------------------------------------------*\
            | Normal mode (plain color)                                 |
            \*---------------------------------------------------------*/
            else
            {
                RGBColor rgb = ToRGBColor((int)(b*color.red()), (int)(b*color.green()), (int)(b*color.blue()));
                if (controller_zone->is_segment)
                {
                    for(unsigned int led_id = 0; led_id < ledsCount; ++led_id)
                    {
                        if (startIdx + led_id >= zoneLedCount)
                        {
                            break;
                        }
                        controller_zone->controller->zones[controller_zone->zone_idx].colors[startIdx + led_id] = rgb;
                    }
                }
                else
                {
                    controller_zone->controller->SetAllZoneLEDs(controller_zone->zone_idx, rgb);
                }
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
    /*---------------------------------------------------------*\
    | Measure thread                                            |
    \*---------------------------------------------------------*/
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

    std::vector<json> devices;

    for(ControllerZone* controller_zone: controller_zones)
    {
        if(controller_zone->enabled)
        {
            devices.push_back(controller_zone->to_json());
        }
    }

    j["devices"]            = devices;
    j["refresh_interval"]   = refresh_interval;
    j["hardware"]           = ui->hardware->currentIndex();
    j["hardware_feature"]   = ui->hardware_feature->currentIndex();
    j["measure_color"]      = ui->measure_color->ToJson();
    j["running"]            = running;
    j["auto_start"]         = auto_start;
    j["fps"]                = fps;
    j["name"]               = name;
    j["brightness"]         = brightness;

#ifdef _WIN32
    j["hardware_identifier"] = ui->hardware_feature->currentData().toString().toStdString();
#endif

    return j;
}

MeasureEntry* MeasureEntry::FromJson(QWidget *parent, json settings)
{
    MeasureEntry* measure = new MeasureEntry(parent);

    if(settings.contains("refresh_interval"))   measure->ui->refresh_interval->setValue(settings["refresh_interval"]);

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
    if(settings.contains("hardware"))           measure->ui->hardware->setCurrentIndex(settings["hardware"]);

    if(settings.contains("hardware_feature"))   measure->ui->hardware_feature->setCurrentIndex(settings["hardware_feature"]);
#endif

    if(settings.contains("measure_color"))      measure->ui->measure_color->LoadJson(settings["measure_color"]);

    if(settings.contains("devices"))
    {
        json zones = settings["devices"];

        for(auto j : zones)
        {
            for(ControllerZone* controller_zone: measure->controller_zones)
            {
                auto location = j["location"].get<std::string>();
                bool ignore_location = (location.find("HID: ") == 0);

                if(
                        controller_zone->controller->GetName()          == j["name"] &&
                        (
                            ignore_location ||
                            controller_zone->controller->GetLocation()  == j["location"]
                            ) &&
                        controller_zone->controller->GetSerial()        == j["serial"] &&
                        controller_zone->controller->GetDescription()   == j["description"] &&
                        controller_zone->controller->GetVersion()       == j["version"] &&
                        controller_zone->controller->GetVendor()        == j["vendor"] &&
                        controller_zone->zone_idx                       == j["zone_idx"] &&
                        controller_zone->is_segment                     == j["is_segment"] &&
                        controller_zone->segment_idx                    == j["segment_idx"]
                        )
                {
                    controller_zone->enabled = true;

                    if(j.contains("is_percentage_fill"))
                    {
                        controller_zone->is_percentage_fill = j["is_percentage_fill"];
                    }
                    else
                    {
                        controller_zone->is_percentage_fill = false;
                    }

                    if(j.contains("brightness"))
                    {
                        controller_zone->brightness = j["brightness"];
                    }
                    else
                    {
                        controller_zone->brightness = 100;
                    }

                    if(j.contains("reverse"))
                    {
                        controller_zone->reverse = j["reverse"];
                    }
                    else
                    {
                        controller_zone->reverse = false;
                    }

                    measure->controller_zones_widgets[controller_zone]->setPercentageFill(controller_zone->is_percentage_fill);
                    measure->controller_zones_widgets[controller_zone]->setEnabled(controller_zone->enabled);
                    measure->controller_zones_widgets[controller_zone]->setBrightness(controller_zone->brightness);
                    measure->controller_zones_widgets[controller_zone]->setReverse(controller_zone->reverse);

                    break;
                }
            }
        }
    }

    if(settings.contains("fps"))        measure->ui->fps->setValue(settings["fps"]);
    if(settings.contains("name"))       measure->name = settings["name"];
    if(settings.contains("brightness")) measure->ui->brightness->setValue(settings["brightness"]);

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

