#ifndef HARDWARESYNCMAINPAGE_H
#define HARDWARESYNCMAINPAGE_H

#include <QWidget>
#include <nlohmann/json.hpp>
#include "ui_HardwareSyncMainPage.h"
#include "MeasureEntry.h"

using json = nlohmann::json;

namespace Ui {
class HardwareSyncMainPage;
}

class HardwareSyncMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit HardwareSyncMainPage(QWidget *parent = nullptr);
    ~HardwareSyncMainPage();

    void Clear();
    std::vector<MeasureEntry*> GetMeasures();
    void LoadMeasures(json measure_list);
    void StopAll();

public slots:
    void InitDevicesList();

private slots:
    void on_start_stop_all_clicked();
    void AddTabSlot();
    void OnStopMeasures();

private:
    Ui::HardwareSyncMainPage *ui;
    std::vector<MeasureEntry*> measures;
    void AddMeasure(MeasureEntry*);
    void CloseMeasure(MeasureEntry* measure_entry_tab);
    bool HasStartedMeasures();
};

#endif // HARDWARESYNCMAINPAGE_H
