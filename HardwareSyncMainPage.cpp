#include "HardwareSyncMainPage.h"
#include "MeasureSettings.h"
#include "PluginInfo.h"
#include "TabHeader.h"
#include <QToolButton>

HardwareSyncMainPage::HardwareSyncMainPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HardwareSyncMainPage)
{
    ui->setupUi(this);

    // remove intial dummy tabs
    ui->tabs->clear();

    // define tab style + settings
    ui->tabs->setTabsClosable(true);
    ui->tabs->setStyleSheet("QTabBar::close-button{image:url(:images/close.png);}");
    ui->tabs->tabBar()->setStyleSheet("QTabBar::tab:hover {text-decoration: underline;}");

    // First tab: add button
    QToolButton *new_measure_button = new QToolButton();
    new_measure_button->setText("New measure");
    ui->tabs->addTab(new PluginInfo(), QString(""));
    ui->tabs->tabBar()->setTabButton(0, QTabBar::RightSide, new_measure_button);
    ui->tabs->setTabEnabled(0, false);

    // 2nd tab: plugin info
    QToolButton *dummy_button = new QToolButton();
    dummy_button->setText("");
    ui->tabs->addTab(new PluginInfo(), QString("Plugin info"));
    ui->tabs->tabBar()->setTabButton(1, QTabBar::RightSide, dummy_button);
    dummy_button->setFixedWidth(0);
    dummy_button->setFixedHeight(0);
    dummy_button->hide();

    connect(new_measure_button, SIGNAL(clicked()), this, SLOT(AddTabSlot()));

    // Make sure to wait a bit before all other plugins are loaded
    QTimer::singleShot(2000, [=](){
        json measure_list = MeasureSettings::Load();

        for(json measure : measure_list)
        {
            AddMeasure(MeasureEntry::FromJson(this, measure));
        }
    });

}

HardwareSyncMainPage::~HardwareSyncMainPage()
{
    delete ui;
}

void HardwareSyncMainPage::AddTabSlot()
{
    AddMeasure(new MeasureEntry(this));
}


void HardwareSyncMainPage::Clear()
{
    for(MeasureEntry* measure: measures)
    {
        measure->Stop();
        measure->Clear();
    }
}

void HardwareSyncMainPage::AddMeasure(MeasureEntry* measure_entry_tab)
{    
    int tab_size = ui->tabs->count();

    // insert at the end
    int tab_position = tab_size;

    std::string tab_name = measure_entry_tab->GetName();

    //VirtualControllerTab* tab = new VirtualControllerTab();
    TabHeader* tab_header = new TabHeader();
    tab_header->Rename(QString::fromUtf8(tab_name.c_str()));

    measure_entry_tab->Rename(tab_name);

    ui->tabs->insertTab(tab_position, measure_entry_tab , "");
    ui->tabs->tabBar()->setTabButton(tab_position, QTabBar::RightSide, tab_header);

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

    ui->tabs->update();

    measures.push_back(measure_entry_tab);
}

void HardwareSyncMainPage::on_save_clicked()
{
    std::vector<json> v;

    printf("[HardwareSyncMainPage] preparing json structure...\n");

    for(MeasureEntry* measure: measures)
    {
        v.push_back(measure->ToJson());
    }

    printf("[HardwareSyncMainPage] json ready to be saved\n");

    json j = v;

    MeasureSettings::Save(j);
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

