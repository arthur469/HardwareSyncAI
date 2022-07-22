#include "ControllerZoneListItemWidget.h"
#include "ui_ControllerZoneListItemWidget.h"

ControllerZoneListItemWidget::ControllerZoneListItemWidget(QString zone_name, bool percentage_fill, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ControllerZoneListItemWidget)
{
    ui->setupUi(this);
    ui->zone_name->setText(zone_name);
    ui->percentage_fill->setChecked(percentage_fill);
}

bool ControllerZoneListItemWidget::isPercentageFill()
{
    return ui->percentage_fill->isChecked();
}

void ControllerZoneListItemWidget::setPercentageFill(bool percentage_fill)
{
    ui->percentage_fill->setChecked(percentage_fill);
}

ControllerZoneListItemWidget::~ControllerZoneListItemWidget()
{
    delete ui;
}

void ControllerZoneListItemWidget::showEvent(QShowEvent *event)
{
    int cb_width = ui->percentage_fill->width();
    int p_width = parentWidget()->width();
    int w = p_width - cb_width * 2;

    ui->zone_name->setFixedWidth(w);
}
