#ifndef CONTROLLERZONELISTITEMWIDGET_H
#define CONTROLLERZONELISTITEMWIDGET_H

#include <QWidget>

namespace Ui {
class ControllerZoneListItemWidget;
}

class ControllerZoneListItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ControllerZoneListItemWidget(QString zone_name, bool percentage_fill, QWidget *parent = nullptr);
    ~ControllerZoneListItemWidget();
    bool isPercentageFill();
    void setPercentageFill(bool);

protected:
    void showEvent(QShowEvent *event) override;

private:
    Ui::ControllerZoneListItemWidget *ui;
};

#endif // CONTROLLERZONELISTITEMWIDGET_H
