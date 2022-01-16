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
    json ToJson();
    void Clear();

    static MeasureEntry* FromJson(QWidget *parent, json);

private slots:
    void on_refresh_interval_valueChanged(int);
    void on_hardware_currentIndexChanged(int);
    void on_hardware_feature_currentIndexChanged(int);
    void on_start_stop_clicked();
    void on_devices_clicked();
    void on_remove_clicked();
    void OnMeasure(double);
    void OnColor(QColor color);

signals:
    void RemoveRequest();
    void Measure(double);
    void Color(QColor);

private:
    Ui::MeasureEntry *ui;
    std::vector<Hardware> devices;
    int hardware_id;
    int hardware_feature_id;
    int last_idx = 0;

    bool percentage_fill = false;

    bool running = false;

    std::vector<ControllerZone*> assigned_zones;

    std::thread* measure_thread;
    int refresh_interval = 100;

    std::thread* leds_thread;
    int leds_refresh_interval = 20;

    double measure = 0.0;

    void Tick();
    void UpdateLEDsThread();

};

#endif // MEASUREENTRY_H
