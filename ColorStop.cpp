#include "ColorStop.h"
#include "ColorPicker.h"

#include "ui_ColorStop.h"

ColorStop::ColorStop(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorStop)
{
    ui->setupUi(this);

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

void ColorStop::SetStop(int value)
{
    ui->stop->setValue(value);
}

QColor ColorStop::GetColor()
{
    return stop.second;
}

int ColorStop::GetValue()
{
    return ui->stop->value();
}

void ColorStop::on_stop_valueChanged(int)
{
    emit GradientStopChanged();
}

void ColorStop::on_remove_button_clicked()
{
    emit RemoveRequest();
}

ColorStop::~ColorStop()
{
    delete ui;
}
