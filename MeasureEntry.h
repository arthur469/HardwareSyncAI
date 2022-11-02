#ifndef MEASUREENTRY_H
#define MEASUREENTRY_H

#include <QWidget>
#include <QTimer>
#include "ui_MeasureEntry.h"
#include "HardwareMeasure.h"
#include "ControllerZone.h"

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
    void Clear();
    void Rename(std::string);
    std::string GetName();

    static MeasureEntry* FromJson(QWidget *parent, json);

private slots:
    void on_refresh_interval_valueChanged(int);
    void on_hardware_currentIndexChanged(int);
    void on_hardware_feature_currentIndexChanged(int);
    void on_start_stop_clicked();
    void on_devices_clicked();
    void on_percentage_fill_toggled(bool);
    void on_fps_valueChanged(int);
    void on_auto_start_stateChanged(int);
    void OnMeasure(double);
    void OnColor(QColor color);
    void on_brightness_valueChanged(int);

signals:
    void Measure(double);
    void Color(QColor);
    void Renamed(std::string);

private:
    Ui::MeasureEntry *ui;
    std::vector<Hardware> devices;
    unsigned int hardware_id;
    unsigned int hardware_feature_id;
    float last_idx = 0;

    std::string name = "Untitled";

    bool auto_start = false;

    bool running = false;

    std::vector<ControllerZone*> assigned_zones;

    std::thread* measure_thread;
    int refresh_interval = 100;
    int fps = 60;

    std::thread* leds_thread;
    int leds_refresh_interval = 20;

    double measure = 0.0;

    void Tick();
    void UpdateLEDsThread();

    int brightness = 100;
};

#endif // MEASUREENTRY_H
