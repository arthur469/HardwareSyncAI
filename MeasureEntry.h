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
    void OnMeasure(double, QColor);

signals:
    void RemoveRequest();
    void Measure(double, QColor);

private:
    Ui::MeasureEntry *ui;
    std::vector<Hardware> devices;
    int hardware_id;
    int hardware_feature_id;
    int last_idx = 0;

    bool running = false;

    std::vector<ControllerZone*> controller_zones;

    std::thread* thread;
    int refresh_interval = 100;
    void Tick();

};

#endif // MEASUREENTRY_H
