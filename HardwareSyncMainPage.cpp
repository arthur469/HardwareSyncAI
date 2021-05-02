#include "HardwareSyncMainPage.h"

HardwareSyncMainPage::HardwareSyncMainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HardwareSyncMainPage)
{
    ui->setupUi(this);
    ui->measures->setLayout(new QVBoxLayout());
}

HardwareSyncMainPage::~HardwareSyncMainPage()
{
    delete ui;
}

void HardwareSyncMainPage::on_add_measure_clicked()
{
    MeasureEntry* measure = new MeasureEntry(this);
    ui->measures->layout()->addWidget(measure);
    measures.push_back(measure);

    connect(measure, &MeasureEntry::RemoveRequest, [=](){
        ui->measures->layout()->removeWidget(measure);
        measures.erase(std::find(measures.begin(), measures.end(), measure));
        delete measure;
    });
}
