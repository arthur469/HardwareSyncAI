#ifndef MEASUREENTRY_H
#define MEASUREENTRY_H

#include <thread>
#include <limits>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QPushButton>
#include "ControllerZone.h"
#include "ControllerZoneListItemWidget.h"
#include "HardwareMeasure.h"
#include "ui_MeasureEntry.h"

namespace Ui {
class MeasureEntry;
}

class MeasureEntry : public QWidget
{
    Q_OBJECT

public:
    explicit MeasureEntry(QWidget *parent = nullptr);
    ~MeasureEntry();

    void Start();
    void Stop();
    bool IsRunning();
    json ToJson();
    json Diagnostic();
    void Clear();
    void InitDevicesList();
    json SaveDevices();
    void LoadDevices(json zones);
    void Rename(std::string);
    void ConfigureEffect(const json&);
    std::string GetName();

    static MeasureEntry* FromJson(QWidget *parent, json);

private slots:
    void on_refresh_interval_valueChanged(int);
    void on_hardware_currentIndexChanged(int);
    void on_hardware_feature_currentIndexChanged(int);
    void on_fps_valueChanged(int);
    void on_brightness_valueChanged(int);
    void on_percentage_fill_all_toggled(bool);
    void on_enable_all_toggled(bool);
    void on_toggle_brightness_toggled(bool);
    void on_reverse_all_toggled(bool);

    void OnMeasure(double);
    void OnColor(QColor color);

signals:
    void Measure(double);
    void Color(QColor);
    void Renamed(std::string);
    void MeasureState(bool);

private:
    Ui::MeasureEntry *ui;
    std::vector<Hardware> devices;
    unsigned int hardware_id=0;
    unsigned int hardware_feature_id=0;
    float last_idx = 0;

    std::string name = "Untitled";

    bool running = false;
    QTimer frameTimer;
    QElapsedTimer effectClock;
    qint64 lastFrame=0;
    qint64 lastSample=0;
    QComboBox* effectMode=nullptr;
    QComboBox* effectTrigger;
    QDoubleSpinBox* effectPeriod;
    QDoubleSpinBox* effectFloor;
    QDoubleSpinBox* smoothing;
    QDoubleSpinBox* deadband;
    double paletteTarget=std::numeric_limits<double>::quiet_NaN();
    qint64 lastRenderTime=0;
    void UpdateRenderInterval();
    bool FullySelected(RGBControllerInterface*) const;
    void RefreshEffectChoices();
    std::map<RGBControllerInterface*,int> nativeFallbacks;

    std::map<ControllerZone*, ControllerZoneListItemWidget*> controller_zones_widgets;
    std::vector<ControllerZone*> controller_zones;

    std::thread* measure_thread;
    int refresh_interval = 100;
    int fps = 5;

    std::thread* leds_thread;
    int leds_refresh_interval = 20;

    double measure = 0.0;

    void Tick();
    void UpdateLEDsThread();

    int brightness = 100;
    QTreeWidget* targetTree=nullptr;
    std::map<RGBControllerInterface*,QTreeWidgetItem*> deviceItems;
    QLabel* ruleStateLabel=nullptr;
    QLabel* outputDetails=nullptr;
    json lastOutputs=json::array();
    void SyncAssignments(bool edited=false);
    void CommitDeviceEdits();
    void UpdateTargetStatus();
    json configuredDevices=json::array();
    std::map<ControllerZone*,json> deviceSnapshots;
    std::map<ControllerZone*,size_t> restoredTargets;
    json targetStatus=json::array();
    bool restoringDevices=false;
    QLabel* pendingTargets=nullptr;
    QPushButton* forgetTargets=nullptr;
    void SetState(const std::string& state);
};

#endif // MEASUREENTRY_H
