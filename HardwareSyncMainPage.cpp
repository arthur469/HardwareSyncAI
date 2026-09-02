#include "HardwareSyncMainPage.h"
#include "TabHeader.h"
#include <QToolButton>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QDialog>

HardwareSyncMainPage::HardwareSyncMainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HardwareSyncMainPage)
{
    ui->setupUi(this);

    /*-----------------------------------------------------*\
    | Remove intial dummy tabs                              |
    \*-----------------------------------------------------*/
    ui->tabs->clear();

    /*-----------------------------------------------------*\
    | Define tab style + settings                           |
    \*-----------------------------------------------------*/
    ui->tabs->setTabsClosable(true);
    ui->tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    QMenu* main_menu = new QMenu(this);

    QLabel* no_measure = new QLabel("You have no measure.\n You can add one by clicking the HWSync button.");
    no_measure->setAlignment(Qt::AlignCenter);

    /*-----------------------------------------------------*\
    | First tab: add button                                 |
    \*-----------------------------------------------------*/
    QPushButton* main_menu_button = new QPushButton();
    main_menu_button->setText("HWSync");
    ui->tabs->addTab(no_measure, QString(""));
    ui->tabs->tabBar()->setTabButton(0, QTabBar::RightSide, main_menu_button);
    ui->tabs->setTabEnabled(0, false);
    main_menu_button->setMenu(main_menu);

    QAction* new_measure = new QAction("New measure", this);
    connect(new_measure, &QAction::triggered, this, &HardwareSyncMainPage::AddTabSlot);
    main_menu->addAction(new_measure);

    for(QWidget *w : QApplication::topLevelWidgets())
    {
        if(QMainWindow* mainWin = qobject_cast<QMainWindow*>(w))
        {
            QAction* actionLightsOff = mainWin->findChild<QAction *>("ActionLightsOff");

            if(actionLightsOff)
            {
                connect(actionLightsOff, SIGNAL(triggered()), this, SLOT(OnStopMeasures()));
                break;
            }
        }
    }
}

HardwareSyncMainPage::~HardwareSyncMainPage()
{
    delete ui;
}

void HardwareSyncMainPage::AddTabSlot()
{
    AddMeasure(new MeasureEntry(this));
}

void HardwareSyncMainPage::StopAll()
{
    for(MeasureEntry* measure: measures)
    {
        measure->Stop();
    }
}

void HardwareSyncMainPage::Clear()
{
    for(MeasureEntry* measure: measures)
    {
        CloseMeasure(measure);
    }
}

void HardwareSyncMainPage::LoadMeasures(json measure_list)
{
    for(json measure : measure_list)
    {
        AddMeasure(MeasureEntry::FromJson(this, measure));
    }
}

std::vector<MeasureEntry*> HardwareSyncMainPage::GetMeasures()
{
    return measures;
}

void HardwareSyncMainPage::InitDevicesList()
{
    for(MeasureEntry* measure: measures)
    {
        measure->InitDevicesList();
    }
}

void HardwareSyncMainPage::RefreshDevicesLists()
{
    /*-----------------------------------------------------*\
    | Save the running state and the enabled devices of     |
    | each measure before refreshing                        |
    \*-----------------------------------------------------*/
    std::vector<bool> was_running;
    std::vector<json> saved_devices;

    for(MeasureEntry* measure: measures)
    {
        was_running.push_back(measure->IsRunning());
        saved_devices.push_back(measure->SaveDevices());
    }
    
    /*-----------------------------------------------------*\
    | Stop all measures temporarily to safely clear and     |
    | reinitialize device lists                             |
    \*-----------------------------------------------------*/
    StopAll();
    
    for(MeasureEntry* measure: measures)
    {
        measure->Clear();
        measure->InitDevicesList();
    }
    
    /*-----------------------------------------------------*\
    | Restore the enabled devices and the running state     |
    | after the list update is complete                     |
    \*-----------------------------------------------------*/
    for(size_t i = 0; i < measures.size(); ++i)
    {
        measures[i]->LoadDevices(saved_devices[i]);

        if(was_running[i])
        {
            measures[i]->Start();
        }
    }
}

void HardwareSyncMainPage::CloseMeasure(MeasureEntry* measure_entry_tab)
{
    int tab_idx = ui->tabs->indexOf(measure_entry_tab);

    ui->tabs->removeTab(tab_idx);

    measures.erase(std::find(measures.begin(), measures.end(), measure_entry_tab));

    delete measure_entry_tab;
}

void HardwareSyncMainPage::AddMeasure(MeasureEntry* measure_entry_tab)
{    
    int tab_size = ui->tabs->count();

    /*-----------------------------------------------------*\
    | Insert at the end                                     |
    \*-----------------------------------------------------*/
    int tab_position = tab_size;

    std::string tab_name = measure_entry_tab->GetName();

    TabHeader* tab_header = new TabHeader();
    tab_header->Rename(QString::fromUtf8(tab_name.c_str()));

    measure_entry_tab->Rename(tab_name);

    ui->tabs->insertTab(tab_position, measure_entry_tab , "");
    ui->tabs->tabBar()->setTabButton(tab_position, QTabBar::RightSide, tab_header);

    connect(measure_entry_tab, &MeasureEntry::MeasureState, [=](bool state){
        tab_header->ToogleRunningIndicator(state);
    });

    ui->tabs->setCurrentIndex(tab_position);

    connect(measure_entry_tab, &MeasureEntry::Renamed, [=](std::string new_name){
        tab_header->Rename(QString::fromUtf8(new_name.c_str()));
    });

    connect(tab_header, &TabHeader::RenameRequest, [=](QString new_name){
        measure_entry_tab->Rename(new_name.toStdString());
    });

    connect(tab_header, &TabHeader::CloseRequest, [=](){
        int tab_idx = ui->tabs->indexOf(measure_entry_tab);

        ui->tabs->removeTab(tab_idx);

        measures.erase(std::find(measures.begin(), measures.end(), measure_entry_tab));

        delete measure_entry_tab;
        delete tab_header;
    });

    connect(tab_header, &TabHeader::StartStopRequest, [=](){

        if(measure_entry_tab->IsRunning())
        {
            measure_entry_tab->Stop();
        }
        else
        {
            measure_entry_tab->Start();
        }
    });

    tab_header->ToogleRunningIndicator(measure_entry_tab->IsRunning());

    ui->tabs->update();

    measures.push_back(measure_entry_tab);
}

void HardwareSyncMainPage::on_start_stop_all_clicked()
{
    if(HasStartedMeasures())
    {
        for(MeasureEntry* measure: measures)
        {
            measure->Stop();
        }
    }

    else
    {
        for(MeasureEntry* measure: measures)
        {
            measure->Start();
        }
    }

}

bool HardwareSyncMainPage::HasStartedMeasures()
{
    for(MeasureEntry* measure: measures)
    {
        if(measure->IsRunning())
            return true;
    }

    return false;
}

void HardwareSyncMainPage::OnStopMeasures()
{
    for(MeasureEntry* measure: measures)
    {
        measure->Stop();
    }
}
