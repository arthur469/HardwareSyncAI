#include "ColorStop.h"
#include "ColorPicker.h"

#include "ui_ColorStop.h"

ColorStop::ColorStop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorStop)
{
    ui->setupUi(this);
    ui->remove_button->setText("Remove");
    ui->remove_button->setFlat(false);
    ui->remove_button->setMinimumSize(72,28);
    ui->remove_button->setMaximumSize(16777215,32);
    ui->remove_button->setAccessibleName("Remove color stop");
    ui->remove_button->setToolTip("Remove this color stop");

    //default values
    stop.second = Qt::white;

    connect(ui->color_picker, &ColorPicker::ColorSelected, [=](QColor color){
        stop.second = color;
        emit GradientStopChanged();
    });

}

void ColorStop::SetColor(QColor color)
{
    ui->color_picker->SetColor(color);
}

void ColorStop::SetStop(double value)
{
    ui->stop->setValue(value);
}

QColor ColorStop::GetColor()
{
    return stop.second;
}

double ColorStop::GetValue()
{
    return ui->stop->value();
}

void ColorStop::on_stop_valueChanged(double)
{
    emit GradientStopChanged();
}

void ColorStop::on_remove_button_clicked()
{
    emit RemoveRequest();
}

void ColorStop::SetRemoveEnabled(bool enabled)
{
    ui->remove_button->setEnabled(enabled);
    ui->remove_button->setToolTip(enabled?"Remove this color stop":"Keep at least one color; one color gives a constant output");
}

ColorStop::~ColorStop()
{
    delete ui;
}
