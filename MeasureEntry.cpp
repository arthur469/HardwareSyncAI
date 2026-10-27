#include <regex>
#include <set>
#include <limits>
#include <set>
#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QSignalBlocker>
#include <QHeaderView>
#include "TelemetryRuntime.h"
#include "TargetIdentity.h"
#include <QLocale>
#include "TelemetryMath.h"
#include <thread>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QListView>
#include <QStringList>
#include <QStringListModel>
#include "ControllerZoneManager.h"
#include "MeasureEntry.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include "OpenRGBPluginsFont.h"

MeasureEntry::MeasureEntry(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeasureEntry)
{
    ui->setupUi(this);
    setObjectName("measureEditor");
    auto group=new QGroupBox("Effect overlay",this);auto layout=new QFormLayout(group);
    effectMode=new QComboBox(group);effectMode->setObjectName("effectMode");effectMode->addItem("No effect","steady");effectMode->addItem("Breathing","breathing");effectMode->addItem("Pulse","pulse");
    effectTrigger=new QComboBox(group);effectTrigger->setObjectName("effectTrigger");effectTrigger->addItem("Always","always");effectTrigger->addItem("Model GPU","gpu_model");effectTrigger->addItem("Model RAM","ram_model");effectTrigger->addItem("Model CPU","cpu_model");
    effectPeriod=new QDoubleSpinBox(group);effectPeriod->setRange(2,30);effectPeriod->setValue(5);effectPeriod->setSuffix(" s");effectPeriod->setObjectName("effectPeriod");
    effectFloor=new QDoubleSpinBox(group);effectFloor->setRange(10,100);effectFloor->setValue(30);effectFloor->setSuffix(" %");effectFloor->setObjectName("effectFloor");
    layout->addRow("Effect",effectMode);layout->addRow("Condition",effectTrigger);layout->addRow("Period",effectPeriod);layout->addRow("Minimum brightness",effectFloor);ui->gridLayout->addWidget(group,2,0,1,3);
    smoothing=new QDoubleSpinBox(group);smoothing->setRange(.1,30);smoothing->setValue(2);smoothing->setSuffix(" s");layout->addRow("Smoothing",smoothing);
    ui->brightness->setRange(1,100);
    ui->fps->setMaximum(60);ui->fps->setValue(5);
    deadband=new QDoubleSpinBox(group);deadband->setRange(0,10);deadband->setValue(1);deadband->setSuffix(" %");deadband->setObjectName("paletteDeadband");
    layout->addRow(QLocale().language()==QLocale::French?"Seuil de variation de la palette":"Palette change threshold",deadband);
    connect(effectMode,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int){UpdateRenderInterval();const bool native=effectMode->currentData().toString().startsWith("native:");effectPeriod->setEnabled(!native);effectFloor->setEnabled(!native);});
    auto tooltip=[](const char* en,const char* fr){return QString::fromUtf8(QLocale().language()==QLocale::French?fr:en);};
    effectMode->setItemData(0,tooltip("Measurement color, without an overlay. A one-color palette stays fixed.","Couleur de la mesure, sans effet ajouté. Une palette d'une seule couleur reste fixe."),Qt::ToolTipRole);
    effectMode->setItemData(1,tooltip("Brightness rises and falls smoothly over the selected period.","La luminosité monte et descend doucement pendant la période choisie."),Qt::ToolTipRole);
    effectMode->setItemData(2,tooltip("A repeated flash followed by a gradual fade.","Une impulsion répétée, suivie d'une diminution progressive."),Qt::ToolTipRole);
    effectTrigger->setItemData(0,tooltip("Apply the overlay continuously.","Applique l'effet en permanence."),Qt::ToolTipRole);
    effectTrigger->setItemData(1,tooltip("Ollama reports a model in GPU memory. Loaded does not mean actively computing.","Ollama signale un modèle en mémoire GPU. Un modèle chargé ne calcule pas forcément."),Qt::ToolTipRole);
    effectTrigger->setItemData(2,tooltip("Some model memory is estimated in system RAM from Ollama's allocation data. Not total RAM usage.","Une partie du modèle est estimée en RAM à partir des allocations Ollama. Ce n'est pas l'utilisation totale de la RAM."),Qt::ToolTipRole);
    effectTrigger->setItemData(3,tooltip("Estimated CPU or partial allocation from Ollama memory sizes; not a direct CPU activity measurement.","Allocation CPU ou partielle estimée à partir des tailles mémoire Ollama ; ce n'est pas une mesure directe de l'activité CPU."),Qt::ToolTipRole);
    effectClock.start();
    connect(&frameTimer,&QTimer::timeout,this,[this](){Tick();UpdateLEDsThread();});


    ui->brightness_label->setFont(OpenRGBPluginsFont::GetFont());
    ui->toggle_brightness->setFont(OpenRGBPluginsFont::GetFont());

    ui->brightness_label->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::sun));
    ui->brightness_label->setFont(QApplication::font());
    ui->brightness_label->setText("Brightness: 100%");
    ui->brightness->setToolTip("Output brightness; the preview includes this value");
    ui->toggle_brightness->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::sun));

    ui->devices_list->setLayout(new QVBoxLayout(ui->devices_list));
    ui->devices_list->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->devicFrame->setMinimumWidth(400);

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
    outputDetails=new QLabel(this);outputDetails->setObjectName("outputDetails");outputDetails->setWordWrap(true);ui->gridLayout->addWidget(outputDetails,3,0,1,3);
    ruleStateLabel=new QLabel(this);ruleStateLabel->setObjectName("ruleState");ruleStateLabel->setWordWrap(true);ui->gridLayout->addWidget(ruleStateLabel,4,0,1,3);
    pendingTargets=new QLabel(this);pendingTargets->setObjectName("pendingTargets");pendingTargets->setWordWrap(true);
    forgetTargets=new QPushButton(QLocale().language()==QLocale::French?"Oublier les cibles indisponibles":"Forget unavailable targets",this);
    forgetTargets->setObjectName("forgetUnavailableTargets");
    ui->gridLayout->addWidget(pendingTargets,5,0,1,3);ui->gridLayout->addWidget(forgetTargets,6,0,1,3);
    connect(forgetTargets,&QPushButton::clicked,this,[this](){
        std::set<size_t> resolved;for(const auto& entry:restoredTargets)resolved.insert(entry.second);
        json retained=json::array();for(size_t i=0;i<configuredDevices.size();++i)if(resolved.count(i))retained.push_back(configuredDevices[i]);
        LoadDevices(retained);
    });
    UpdateTargetStatus();
    frameTimer.start(200);
}

MeasureEntry::~MeasureEntry()
{
    /*---------------------------------------------------------*\
    | Stop measures in destructor                               |
    \*---------------------------------------------------------*/
    Stop();
    frameTimer.stop();
    Clear();
    telemetryRuntime().forget(name);
    delete ui;
}

void MeasureEntry::InitDevicesList()
{
    /*---------------------------------------------------------*\
    | Populate the device list                                  |
    \*---------------------------------------------------------*/
    controller_zones = ControllerZoneManager::GetControllerZones();

    targetTree=new QTreeWidget(ui->devices_list);targetTree->setObjectName("targetTree");targetTree->setHeaderHidden(true);targetTree->header()->setSectionResizeMode(QHeaderView::Stretch);targetTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);targetTree->setMinimumHeight(200);targetTree->setIndentation(14);targetTree->setUniformRowHeights(false);
    ui->devices_list->layout()->addWidget(targetTree);
    for(ControllerZone* controller_zone: controller_zones)
    {
        deviceSnapshots[controller_zone]=controller_zone->to_json();
        auto c=controller_zone->controller;auto& parent=deviceItems[c];
        if(!parent){parent=new QTreeWidgetItem(targetTree);auto name=c->GetDisplayName();if(name.empty())name=c->GetName();parent->setText(0,QString::fromStdString(name));parent->setToolTip(0,QString::fromStdString(c->GetName()));parent->setFlags(parent->flags()|Qt::ItemIsUserCheckable);parent->setCheckState(0,Qt::Unchecked);}
        auto item=new QTreeWidgetItem(parent);
        ControllerZoneListItemWidget* controller_zones_widget = new ControllerZoneListItemWidget(targetTree, controller_zone);
        targetTree->setItemWidget(item,0,controller_zones_widget);item->setSizeHint(0,controller_zones_widget->sizeHint());
        controller_zones_widgets[controller_zone] = controller_zones_widget;
        connect(controller_zones_widget,&ControllerZoneListItemWidget::SelectionChanged,this,[this](){SyncAssignments(true);});
    }
    connect(targetTree,&QTreeWidget::itemChanged,this,[this](QTreeWidgetItem* item,int){for(auto entry:deviceItems)if(entry.second==item){if(restoringDevices)return;bool enabled=item->checkState(0)==Qt::Checked;restoringDevices=true;for(auto zone:controller_zones)if(zone->controller==entry.first)controller_zones_widgets[zone]->setEnabled(enabled);restoringDevices=false;SyncAssignments(true);break;}});
    connect(targetTree,&QTreeWidget::itemExpanded,this,[this](QTreeWidgetItem* item){if(item->parent())return;for(auto entry:deviceItems)if(entry.second!=item)entry.second->setExpanded(false);});
}

void MeasureEntry::Clear()
{
    /*---------------------------------------------------------*\
    | Clear all widgets and memory                              |
    \*---------------------------------------------------------*/
    telemetryRuntime().release(this);
    controller_zones_widgets.clear();deviceItems.clear();deviceSnapshots.clear();restoredTargets.clear();nativeFallbacks.clear();targetTree=nullptr;

    QLayoutItem *child;

    while ((child = ui->devices_list->layout()->takeAt(0)) != 0)
    {
        delete child->widget();
        delete child;
    }
    for(auto zone:controller_zones)delete zone;
    controller_zones.clear();
}

json MeasureEntry::SaveDevices()
{
    // Durable intent, independent of the currently detected topology and pointer lifetimes.
    return configuredDevices;
}

void MeasureEntry::LoadDevices(json zones)
{
    if(!zones.is_array())throw std::runtime_error("Invalid device assignments");
    configuredDevices=zones;
    restoredTargets.clear();targetStatus=json::array();restoringDevices=true;
    for(auto zone:controller_zones)controller_zones_widgets[zone]->setEnabled(false);
    std::map<RGBControllerInterface*,json> controllers;
    for(const auto& item:deviceSnapshots)controllers.emplace(item.first->controller,item.second);
    for(size_t index=0;index<zones.size();++index) {
        const auto& saved=zones[index];
        std::vector<RGBControllerInterface*> devices;
        for(const auto& item:controllers)if(TargetIdentity::sameDevice(saved,item.second))devices.push_back(item.first);
        std::vector<ControllerZone*> candidates;
        if(devices.size()==1)for(auto zone:controller_zones)
            if(zone->controller==devices[0] && TargetIdentity::sameZone(saved,deviceSnapshots.at(zone)))candidates.push_back(zone);
        std::string reason=devices.empty()?"device_absent":devices.size()>1?"device_ambiguous":candidates.empty()?"zone_unavailable":candidates.size()>1?"zone_ambiguous":"resolved";
        if(reason=="resolved" && restoredTargets.count(candidates[0]))reason="duplicate_assignment";
        if(reason=="resolved") {
            auto zone=candidates[0];restoredTargets[zone]=index;
            zone->is_percentage_fill=saved.value("is_percentage_fill",false);
            zone->brightness=std::max(0,std::min(100,saved.value("brightness",100)));
            zone->reverse=saved.value("reverse",false);
            auto widget=controller_zones_widgets.at(zone);
            widget->setPercentageFill(zone->is_percentage_fill);widget->setBrightness(zone->brightness);widget->setReverse(zone->reverse);widget->setEnabled(true);
        }
        targetStatus.push_back({{"index",index},{"state",reason}});
    }
    restoringDevices=false;SyncAssignments();UpdateTargetStatus();RefreshEffectChoices();
}

void MeasureEntry::CommitDeviceEdits()
{
    // Explicit UI edits replace only resolved rows. Missing/ambiguous rows remain durable.
    std::map<size_t,ControllerZone*> resolved;
    for(const auto& entry:restoredTargets)resolved[entry.second]=entry.first;
    json updated=json::array();std::map<ControllerZone*,size_t> bindings;
    auto snapshot=[this](ControllerZone* zone){
        auto row=deviceSnapshots.at(zone);
        row["brightness"]=zone->brightness;row["reverse"]=zone->reverse;row["is_percentage_fill"]=zone->is_percentage_fill;
        return row;
    };
    for(size_t i=0;i<configuredDevices.size();++i) {
        auto found=resolved.find(i);
        if(found==resolved.end())updated.push_back(configuredDevices[i]);
        else if(found->second->enabled){bindings[found->second]=updated.size();updated.push_back(snapshot(found->second));}
    }
    for(auto zone:controller_zones)if(zone->enabled && !restoredTargets.count(zone)) {
        bindings[zone]=updated.size();updated.push_back(snapshot(zone));
    }
    configuredDevices=updated;restoredTargets=bindings;
    targetStatus=json::array();std::set<size_t> bound;
    for(const auto& item:bindings)bound.insert(item.second);
    for(size_t i=0;i<configuredDevices.size();++i)targetStatus.push_back({{"index",i},{"state",bound.count(i)?"resolved":"unavailable_or_ambiguous"}});
    UpdateTargetStatus();
}

void MeasureEntry::UpdateTargetStatus()
{
    if(!pendingTargets || !forgetTargets)return;
    auto count=configuredDevices.size()-restoredTargets.size();
    pendingTargets->setVisible(count>0);forgetTargets->setVisible(count>0);
    const auto text=QLocale().language()==QLocale::French?
        "%1 cible(s) mémorisée(s), indisponible(s) ou ambiguë(s). Aucune attribution automatique.":
        "%1 saved target(s) unavailable or ambiguous. No automatic reassignment.";
    pendingTargets->setText(QString(text).arg(static_cast<qulonglong>(count)));
}

void MeasureEntry::Rename(std::string value)
{
    telemetryRuntime().forget(name);
    name = value;
    SyncAssignments();
    emit Renamed(value);
}

std::string MeasureEntry::GetName()
{
    return name;
}

void MeasureEntry::Start(){
    if(running)return;
    running=true;SyncAssignments();lastFrame=QDateTime::currentMSecsSinceEpoch();frameTimer.start();UpdateRenderInterval();emit MeasureState(true);
    for(auto zone:controller_zones)if(zone->enabled&&zone->leds_count()){deviceItems[zone->controller]->setExpanded(true);break;}
}
void MeasureEntry::Stop(){
    telemetryRuntime().release(this);running=false;if(ruleStateLabel)SetState("Stopped; preview only");emit MeasureState(false);
}
void MeasureEntry::ConfigureEffect(const json& effect){
    auto mode=effect.value("mode",std::string("steady"));auto trigger=effect.value("trigger",std::string("always"));
    const auto threshold=effect.value("deadband",1.0);if(!std::isfinite(threshold)||threshold<0||threshold>10)throw std::runtime_error("Invalid palette threshold");deadband->setValue(threshold);
    int m=effectMode->findData(QString::fromStdString(mode)),t=effectTrigger->findData(QString::fromStdString(trigger));
    if(m<0 && mode.rfind("native:",0)==0){effectMode->addItem(QString::fromStdString("Unavailable: "+mode.substr(7)),QString::fromStdString(mode));m=effectMode->count()-1;}
    if(m<0||t<0)throw std::runtime_error("Unsupported effect or condition");
    double period=effect.value("period",5.0),floor=effect.value("floor",.3);
    if(!std::isfinite(period)||period<2||period>30||!std::isfinite(floor)||floor<.1||floor>1)throw std::runtime_error("Invalid effect period or floor");
    double smooth=effect.value("smoothing",2.0);if(!std::isfinite(smooth)||smooth<.1||smooth>30)throw std::runtime_error("Invalid smoothing");smoothing->setValue(smooth);
    effectMode->setCurrentIndex(m);effectTrigger->setCurrentIndex(t);effectPeriod->setValue(period);effectFloor->setValue(floor*100);
}
bool MeasureEntry::IsRunning()
{
    return running;
}

void MeasureEntry::on_refresh_interval_valueChanged(int value)
{
    refresh_interval = value;
}

void MeasureEntry::UpdateRenderInterval(){
    if(!effectMode)return;
    effectTrigger->setEnabled(effectMode->currentData().toString()!="steady");
    auto mode=effectMode->currentData().toString();
    int rate=(mode=="breathing"||mode=="pulse")?std::max(30,fps):std::max(1,fps);
    frameTimer.setInterval(std::max(16,1000/rate));
}

void MeasureEntry::on_fps_valueChanged(int value)
{
    fps = std::max(1,std::min(60,value));
    UpdateRenderInterval();
}

void MeasureEntry::on_brightness_valueChanged(int value)
{
    brightness = value;
    ui->brightness_label->setText(QString("Brightness: %1%").arg(value));
}

void MeasureEntry::on_hardware_currentIndexChanged(int value)
{
    /*---------------------------------------------------------*\
    | Set the sub features on hardware changed                  |
    \*---------------------------------------------------------*/
    hardware_id = value;
    ui->hardware_feature->clear();

    if(value<0||static_cast<unsigned>(value)>=devices.size())return;
    for(HardwareFeature& feature: devices[hardware_id].features)
    {
        ui->hardware_feature->addItem(QString::fromStdString(feature.Name), QString::fromStdString(feature.Identifier));
    }
}

void MeasureEntry::on_hardware_feature_currentIndexChanged(int value)
{
    hardware_feature_id = value;
    lastSample=0;
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

void MeasureEntry::OnMeasure(double measure)
{
    if(std::isnan(measure) || std::isinf(measure))
    {
        ui->measure->setText("Unavailable");
        return;
    }
    ui->measure->setText(QString::asprintf("%.1f", measure));
}

void MeasureEntry::OnColor(QColor color)
{
    ui->color_preview->setStyleSheet("QLabel {background-color: "+ color.name() + "; border: 1px solid black;}");
}

void MeasureEntry::SyncAssignments(bool edited){
    if(restoringDevices)return;
    if(edited){CommitDeviceEdits();RefreshEffectChoices();}
    std::vector<std::pair<RGBControllerInterface*,unsigned>> selected;
    for(auto zone:controller_zones)if(zone->enabled&&zone->leds_count()>0)selected.push_back({zone->controller,zone->zone_idx});
    if(running)telemetryRuntime().assign(this,name,selected);else telemetryRuntime().release(this);
    if(targetTree){QSignalBlocker blocker(targetTree);for(auto entry:deviceItems){unsigned total=0,enabled=0;for(auto zone:controller_zones)if(zone->controller==entry.first&&zone->leds_count()){total++;if(zone->enabled)enabled++;}entry.second->setCheckState(0,enabled==0?Qt::Unchecked:enabled==total?Qt::Checked:Qt::PartiallyChecked);}}
}
bool MeasureEntry::FullySelected(RGBControllerInterface* controller) const {
    bool populated=false;
    for(unsigned i=0;i<controller->GetZoneCount();++i)if(controller->GetZoneLEDsCount(i)>0) {
        populated=true;bool selected=false;
        for(auto zone:controller_zones)if(zone->controller==controller&&zone->zone_idx==i&&!zone->is_segment&&zone->enabled)selected=true;
        if(!selected)return false;
    }
    return populated;
}

void MeasureEntry::RefreshEffectChoices(){
    const auto previous=effectMode->currentData();QSignalBlocker blocker(effectMode);
    while(effectMode->count()>3)effectMode->removeItem(effectMode->count()-1);
    std::set<RGBControllerInterface*> selected;
    for(auto zone:controller_zones)if(zone->enabled)selected.insert(zone->controller);
    std::map<std::string,std::string> common;bool first=true;
    for(auto controller:selected) {
        std::map<std::string,std::string> modes;
        if(!FullySelected(controller)){common.clear();break;}
        for(unsigned i=0;i<controller->GetModeCount();++i) {
            auto name=controller->GetModeName(i);auto key=QString::fromStdString(name).toLower().toStdString();
            if(TelemetryMath::supportedMode(name,controller->GetModeFlags(i))||key=="off"||key.empty())continue;
            modes[key]=name;
        }
        if(first){common=modes;first=false;}else for(auto it=common.begin();it!=common.end();)if(!modes.count(it->first))it=common.erase(it);else ++it;
    }
    for(const auto& mode:common)effectMode->addItem(QString::fromStdString("Device: "+mode.second),QString::fromStdString("native:"+mode.first));
    int index=effectMode->findData(previous);
    if(index<0 && previous.toString().startsWith("native:")){effectMode->addItem("Unavailable: "+previous.toString().mid(7),previous);index=effectMode->count()-1;}
    effectMode->setCurrentIndex(std::max(0,index));UpdateRenderInterval();
    const bool native=effectMode->currentData().toString().startsWith("native:");effectPeriod->setEnabled(!native);effectFloor->setEnabled(!native);
}

void MeasureEntry::SetState(const std::string& state){telemetryRuntime().record(name,state);if(ruleStateLabel){ruleStateLabel->setText(QString::fromStdString(state));ruleStateLabel->setStyleSheet(state.rfind("Conflict",0)==0?"color: #9a5200; font-weight: bold":"");}}
json MeasureEntry::Diagnostic(){json effects=json::array();for(int i=0;i<effectMode->count();++i)effects.push_back(effectMode->itemData(i).toString().toStdString());return {{"available_effects",effects},{"frame_interval_ms",frameTimer.interval()},{"palette_target",std::isfinite(paletteTarget)?json(paletteTarget):json(nullptr)},{"configuration",ToJson()},{"assignments",targetStatus},{"value",std::isfinite(measure)?json(measure):json(nullptr)},{"state",telemetryRuntime().state(name)},{"outputs",lastOutputs}};}
void MeasureEntry::UpdateLEDsThread(){
    auto& runtime=telemetryRuntime();lastOutputs=json::array();
    const auto renderTime=effectClock.elapsed();
    double elapsed=lastRenderTime?std::clamp((renderTime-lastRenderTime)/1000.0,.001,.25):1.0/std::max(1,fps);lastRenderTime=renderTime;
    const auto range=std::abs(ui->measure_color->max_value-ui->measure_color->min_value);
    const auto span=range>0?range:100.0;
    if(std::isfinite(measure) && (!std::isfinite(paletteTarget) || std::abs(measure-paletteTarget)>span*deadband->value()/100.0))paletteTarget=measure;
    const auto alpha=1-std::exp(-elapsed/smoothing->value());
    auto data=ui->measure_color->GetColor(paletteTarget,last_idx,std::max(1,static_cast<int>(std::round(1/alpha))));
    last_idx=std::get<0>(data);auto color=std::get<1>(data);
    const auto mode=effectMode->currentData().toString().toStdString(),trigger=effectTrigger->currentData().toString().toStdString();
    double scale=1;bool aiKnown=true;const bool nativeEffect=mode.rfind("native:",0)==0;
    double flag=trigger=="always"?1:runtime.value(trigger);
    if(mode!="steady"){aiKnown=std::isfinite(flag);if(aiKnown&&flag>0)scale=TelemetryMath::envelope(mode,effectClock.elapsed()/1000.0,effectPeriod->value(),effectFloor->value()/100.0);}
    auto rendered=[](QColor selected,double dim){int red=std::round(selected.red()*dim),green=std::round(selected.green()*dim),blue=std::round(selected.blue()*dim);if(red==0&&green==0&&blue==0)blue=1;return QColor(red,green,blue);};
    int zoneBrightness=100;QString previewZone="no zone selected";for(auto zone:controller_zones)if(zone->enabled&&zone->leds_count()){zoneBrightness=zone->brightness;previewZone=QString::fromStdString(zone->display_name());break;}
    auto preview=rendered(color,brightness*.01*zoneBrightness*.01*scale);OnColor(preview);
    if(outputDetails)outputDetails->setText(QString("Palette %1 | Output preview %2 | Brightness %3%, zone %4% (%5)").arg(color.name(),preview.name()).arg(brightness).arg(zoneBrightness).arg(previewZone));
    if(!running){SetState("Stopped; preview only");return;}
    const auto now=QDateTime::currentMSecsSinceEpoch();
    if(now-lastFrame>5000)runtime.setPause(true,"Sleep/resume or delayed frame; explicit resume required");lastFrame=now;
    SyncAssignments();
    std::map<RGBControllerInterface*,std::vector<ControllerZone*>> plans;
    unsigned total=0;
    for(auto zone:controller_zones)if(zone->enabled&&zone->leds_count()>0){
      auto other=runtime.conflict(zone->controller,zone->zone_idx,this);
      if(!other.empty()){SetState("Conflict: "+zone->display_name()+" is also selected by "+other+". Deselect it in one rule.");return;}
      plans[zone->controller].push_back(zone);total++;
    }
    if(!total){SetState("No zones selected; choose a device or zone on the right");return;}
    if(nativeEffect)for(const auto& entry:plans) {
        auto c=entry.first;bool supported=false;
        if(FullySelected(c))for(unsigned i=0;i<c->GetModeCount();++i)if("native:"+QString::fromStdString(c->GetModeName(i)).toLower().toStdString()==mode)supported=true;
        if(!supported){SetState("Native effect unavailable or device not fully selected; output frozen");return;}
    }
    if(!runtime.beginFrame()){SetState("Waiting for control lock");return;}
    struct End{TelemetryRuntime& r;~End(){r.endFrame();}} end{runtime};
    if(runtime.paused){SetState("Paused; preview only ("+std::to_string(total)+" zones selected)");return;}
    if(!std::isfinite(measure)){SetState("Telemetry unavailable; output frozen");return;}
    if(!aiKnown){SetState("AI state unknown; output frozen");return;}
    unsigned active=0;std::string protectedNames;
    for(auto& entry:plans){
      auto c=entry.first;
      int nativeIndex=-1;
      if(nativeEffect && FullySelected(c))for(unsigned i=0;i<c->GetModeCount();++i)if("native:"+QString::fromStdString(c->GetModeName(i)).toLower().toStdString()==mode)nativeIndex=i;
      if(nativeEffect && nativeIndex<0){protectedNames+="Selected device effect unavailable; ";continue;}
      if(!runtime.eligible(c,nativeEffect||nativeFallbacks.count(c))){protectedNames+=c->GetName()+"; ";continue;}
      if(nativeEffect && flag>0) {
          if(!nativeFallbacks.count(c))nativeFallbacks[c]=c->GetActiveMode();
          if(c->GetActiveMode()!=nativeIndex){c->SetActiveMode(nativeIndex);runtime.remember(c);}
          for(auto zone:entry.second){lastOutputs.push_back({{"controller",c->GetName()},{"zone",zone->display_name()},{"native_effect",c->GetModeName(nativeIndex)}});active++;}
          continue;
      }
      if(nativeFallbacks.count(c)) {
          const auto fallback=nativeFallbacks.at(c);nativeFallbacks.erase(c);
          if(fallback>=0 && static_cast<unsigned>(fallback)<c->GetModeCount()){c->SetActiveMode(fallback);runtime.remember(c);}
          if(!TelemetryMath::supportedMode(c->GetModeName(c->GetActiveMode()),c->GetModeFlags(c->GetActiveMode())))continue;
      }
      for(auto zone:entry.second){
        double b=brightness*.01*zone->brightness*.01*scale;
        unsigned n=zone->leds_count();double fill=std::clamp(static_cast<double>(last_idx)/99.0,0.0,1.0);
        for(unsigned i=0;i<n;i++){
          QColor selected=color;double dim=b;
          if(zone->is_percentage_fill){if(i>=static_cast<unsigned>(std::ceil(fill*n)))dim=0;else selected=std::get<1>(ui->measure_color->GetColor(ui->measure_color->min_value+(ui->measure_color->max_value-ui->measure_color->min_value)*i/std::max(1u,n-1),100.0*i/std::max(1u,n-1),fps));}
          auto actual=rendered(selected,dim);unsigned index=zone->reverse?n-1-i:i;if(zone->is_segment)index+=zone->start_idx();
          if(index<c->GetZoneLEDsCount(zone->zone_idx))c->SetZoneColor(zone->zone_idx,index,ToRGBColor(actual.red(),actual.green(),actual.blue()));
        }
        lastOutputs.push_back({{"controller",c->GetName()},{"zone",zone->display_name()},{"color",rendered(color,b).name().toStdString()}});active++;
      }
      c->UpdateLEDs();runtime.remember(c);
    }
    if(active==total)SetState("Active: "+std::to_string(active)+" zones");
    else SetState("Active: "+std::to_string(active)+"/"+std::to_string(total)+" zones. Black/OFF, manual change or incompatible mode protected: "+protectedNames);
}
void MeasureEntry::Tick(){
    auto now=QDateTime::currentMSecsSinceEpoch();if(lastSample&&now-lastSample<refresh_interval)return;lastSample=now;
    measure=std::numeric_limits<double>::quiet_NaN();
    if(hardware_id<devices.size()&&hardware_feature_id<devices[hardware_id].features.size())measure=OpenRGBHardwareSyncPlugin::hm->GetMeasure(devices[hardware_id],devices[hardware_id].features[hardware_feature_id]);
    OnMeasure(measure);
}
json MeasureEntry::ToJson()
{
    json j;

    j["devices"]            = SaveDevices();
    j["refresh_interval"]   = refresh_interval;
    j["hardware"]           = ui->hardware->currentIndex();
    j["hardware_feature"]   = ui->hardware_feature->currentIndex();
    j["measure_color"]      = ui->measure_color->ToJson();
    j["running"]            = running;
    j["fps"]                = fps;
    j["name"]               = name;
    j["brightness"]         = brightness;
    j["effect"]={{"mode",effectMode->currentData().toString().toStdString()},{"trigger",effectTrigger->currentData().toString().toStdString()},{"period",effectPeriod->value()},{"floor",effectFloor->value()/100.0},{"smoothing",smoothing->value()},{"deadband",deadband->value()}};

#if defined(_WIN32) || defined(HARDWARE_SYNC_AI_COLLECTOR)
    j["hardware_identifier"] = ui->hardware_feature->currentData().toString().toStdString();
#endif

    return j;
}

MeasureEntry* MeasureEntry::FromJson(QWidget *parent, json settings)
{
    MeasureEntry* measure = new MeasureEntry(parent);

    if(settings.contains("refresh_interval"))   measure->ui->refresh_interval->setValue(settings["refresh_interval"]);

#if defined(_WIN32) || defined(HARDWARE_SYNC_AI_COLLECTOR)
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
        measure->LoadDevices(settings["devices"]);
    }

    if(settings.contains("fps"))        measure->ui->fps->setValue(settings["fps"]);
    if(settings.contains("name"))       measure->name = settings["name"];
    if(settings.contains("brightness")) measure->ui->brightness->setValue(settings["brightness"]);

    if(settings.contains("effect"))measure->ConfigureEffect(settings["effect"]);
    if(settings.contains("running"))
    {
        if(settings["running"])
        {
            measure->Start();
        }
    }

    return measure;
}

