#include "ControllerZoneManager.h"
#include "ControllerZoneListItemWidget.h"
#include "ui_ControllerZoneListItemWidget.h"
#include "OpenRGBPluginsFont.h"

ControllerZoneListItemWidget::ControllerZoneListItemWidget(QWidget *parent, ControllerZone* controller_zone) :
    QWidget(parent),
    ui(new Ui::ControllerZoneListItemWidget),
    controller_zone(controller_zone)
{
    ui->setupUi(this);

    ui->reverse->setFont(OpenRGBPluginsFont::GetFont());
    ui->no_direct_mode_warning->setFont(OpenRGBPluginsFont::GetFont());

    ui->enabled->setText("Use");
    ui->enabled->setMinimumWidth(52);
    ui->enabled->setMaximumWidth(64);
    ui->enabled->setToolTip("Include this zone in the current rule");
    ui->reverse->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::arrows_exchange));
    ui->no_direct_mode_warning->setText(OpenRGBPluginsFont::icon(OpenRGBPluginsFont::danger));

    ui->zone_name->setText(QString::fromStdString(controller_zone->display_name()));
    ui->percentage_fill->setChecked(controller_zone->is_percentage_fill);
    ui->enabled->setChecked(controller_zone->enabled);
    ui->no_direct_mode_warning->setVisible(!ControllerZoneManager::SupportsDirectMode(controller_zone->controller));
    ui->brightness->setVisible(false);
    if(controller_zone->leds_count()==0){ui->enabled->setEnabled(false);setToolTip("Empty zone; no LEDs configured in OpenRGB");}
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
    controller_zone->enabled=state&&controller_zone->leds_count()>0;
    ui->enabled->setChecked(state&&controller_zone->leds_count()>0);
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
    UpdateCheckState();
    emit SelectionChanged();
}

void ControllerZoneListItemWidget::on_percentage_fill_toggled(bool state)
{
    controller_zone->is_percentage_fill = state;
    emit SelectionChanged();
}

void ControllerZoneListItemWidget::on_brightness_valueChanged(int value)
{
    controller_zone->brightness = value;
    emit SelectionChanged();
}

void ControllerZoneListItemWidget::on_reverse_toggled(bool state)
{
    controller_zone->reverse = state;
    emit SelectionChanged();
}

void ControllerZoneListItemWidget::UpdateCheckState()
{
    ui->enabled->setText("Use");
}
