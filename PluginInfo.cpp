#include "PluginInfo.h"
#include "ui_PluginInfo.h"
#include "OpenRGBHardwareSyncPlugin.h"

#include <QDesktopServices>
#include <QUrl>

PluginInfo::PluginInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PluginInfo)
{
    ui->setupUi(this);

    ui->builddate_string->setText(BUILDDATE_STRING);
    ui->version_string->setText(VERSION_STRING);
    ui->git_commit_id->setText(GIT_COMMIT_ID);
    ui->git_commit_date->setText(GIT_COMMIT_DATE);
    ui->git_branch->setText(GIT_BRANCH);
}

void PluginInfo::on_open_plugin_folder_clicked()
{
    std::string config_dir = OpenRGBHardwareSyncPlugin::RMPointer->GetConfigurationDirectory() + "plugins";
    QUrl url = QUrl::fromLocalFile(QString::fromStdString(config_dir));

    QDesktopServices::openUrl(url);
}

void PluginInfo::on_download_latest_clicked()
{
    std::string url_string = LATEST_BUILD_URL;
    QUrl url(QString::fromStdString(url_string));
    QDesktopServices::openUrl(url);
}

PluginInfo::~PluginInfo()
{
    delete ui;
}
