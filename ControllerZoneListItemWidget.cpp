#include "ControllerZoneManager.h"
#include "ControllerZoneListItemWidget.h"
#include "ui_ControllerZoneListItemWidget.h"

ControllerZoneListItemWidget::ControllerZoneListItemWidget(QWidget *parent, ControllerZone* controller_zone) :
    QWidget(parent),
    ui(new Ui::ControllerZoneListItemWidget),
    controller_zone(controller_zone)
{
    ui->setupUi(this);
    ui->zone_name->setText(QString::fromStdString(controller_zone->display_name()));
    ui->percentage_fill->setChecked(controller_zone->is_percentage_fill);
    ui->enabled->setChecked(controller_zone->enabled);
    ui->no_direct_mode_warning->setVisible(!ControllerZoneManager::SupportsDirectMode(controller_zone->controller));
    ui->brightness->setVisible(false);
}

ControllerZoneListItemWidget::~ControllerZoneListItemWidget()
{
    delete ui;
}

bool ControllerZoneListItemWidget::isPercentageFill()
{
    return ui->percentage_fill->isChecked();
}

void ControllerZoneListItemWidget::setPercentageFill(bool percentage_fill)
{
    ui->percentage_fill->setChecked(percentage_fill);
}

bool ControllerZoneListItemWidget::isEnabled()
{
    return ui->enabled->isChecked();
}

void ControllerZoneListItemWidget::setEnabled(bool state)
{
    ui->enabled->setChecked(state);
}

bool ControllerZoneListItemWidget::isReverse()
{
    return ui->reverse->isChecked();
}

void ControllerZoneListItemWidget::setReverse(bool state)
{
    ui->reverse->setChecked(state);
}

void ControllerZoneListItemWidget::setBrightness(int value)
{
    ui->brightness->setValue(value);
}

void ControllerZoneListItemWidget::showBrightnessSlider(bool state)
{
    ui->brightness->setVisible(state);
}

void ControllerZoneListItemWidget::on_enabled_toggled(bool state)
{
    controller_zone->enabled = state;
}

void ControllerZoneListItemWidget::on_percentage_fill_toggled(bool state)
{
    controller_zone->is_percentage_fill = state;
}

void ControllerZoneListItemWidget::on_brightness_valueChanged(int value)
{
    controller_zone->brightness = value;
}

void ControllerZoneListItemWidget::on_reverse_toggled(bool state)
{
    controller_zone->reverse = state;
}
