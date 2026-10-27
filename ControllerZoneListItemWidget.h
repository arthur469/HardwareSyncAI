#ifndef CONTROLLERZONELISTITEMWIDGET_H
#define CONTROLLERZONELISTITEMWIDGET_H

#include <QWidget>
#include "ControllerZone.h"

namespace Ui {
class ControllerZoneListItemWidget;
}

class ControllerZoneListItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControllerZoneListItemWidget(QWidget *parent = nullptr, ControllerZone* controller_zone = nullptr);
    ~ControllerZoneListItemWidget();
    bool isPercentageFill();
    void setPercentageFill(bool);
    bool isReverse();
    void setReverse(bool);

    bool isEnabled();
    void setEnabled(bool);
    void setBrightness(int);
    void showBrightnessSlider(bool);

signals:
    void SelectionChanged();

private slots:
    void on_enabled_toggled(bool);
    void on_percentage_fill_toggled(bool);
    void on_brightness_valueChanged(int);
    void on_reverse_toggled(bool);

private:
    Ui::ControllerZoneListItemWidget *ui;
    ControllerZone* controller_zone;
    void UpdateCheckState();
};

#endif // CONTROLLERZONELISTITEMWIDGET_H
