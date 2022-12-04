#include "TabHeader.h"
#include "ui_TabHeader.h"
#include "OpenRGBPluginsFont.h"

TabHeader::TabHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TabHeader)
{
    ui->setupUi(this);

    ui->start_stop->setFont(OpenRGBPluginsFont::GetFont());
    ui->close->setFont(OpenRGBPluginsFont::GetFont());

    ui->close->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::close_o));

    ToogleRunningIndicator(false);
}

void TabHeader::ToogleRunningIndicator(bool state)
{
    ui->start_stop->setText(
                state ?
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::toggle_off):
                    OpenRGBPluginsFont::icon(OpenRGBPluginsFont::toggle_on)
                    );

    ui->start_stop->setToolTip(state?"Started":"Stopped");
}

void TabHeader::Rename(QString name)
{
    ui->editor->setText(name);
}

void TabHeader::on_editor_editingFinished()
{
    emit RenameRequest(ui->editor->text());
}

void TabHeader::on_close_clicked()
{
    emit CloseRequest();
}

void TabHeader::on_start_stop_clicked()
{
    emit StartStopRequest();
}
TabHeader::~TabHeader()
{
    delete ui;
}
