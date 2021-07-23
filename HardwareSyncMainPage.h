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

private slots:
    void on_add_measure_clicked();
    void on_save_clicked();

private:
    Ui::HardwareSyncMainPage *ui;
    std::vector<MeasureEntry*> measures;
    void AddMeasure(MeasureEntry*);
};

#endif // HARDWARESYNCMAINPAGE_H
