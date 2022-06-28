#ifndef HARDWARESYNCMAINPAGE_H
#define HARDWARESYNCMAINPAGE_H

#include <QWidget>
#include "ui_HardwareSyncMainPage.h"
#include "MeasureEntry.h"

namespace Ui {
class HardwareSyncMainPage;
}

class HardwareSyncMainPage : public QWidget
{
    Q_OBJECT

public:
    explicit HardwareSyncMainPage(QWidget *parent = nullptr);
    ~HardwareSyncMainPage();

public slots:
    void Clear();

private slots:
    void on_save_clicked();
    void on_start_stop_all_clicked();
    void AddTabSlot();

private:
    Ui::HardwareSyncMainPage *ui;
    std::vector<MeasureEntry*> measures;
    void AddMeasure(MeasureEntry*);
    bool HasStartedMeasures();
};

#endif // HARDWARESYNCMAINPAGE_H
