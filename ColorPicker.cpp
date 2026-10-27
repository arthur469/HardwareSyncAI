#include "ColorPicker.h"
#include "OpenRGBHardwareSyncPlugin.h"
#include "ui_ColorPicker.h"

#include <QColorDialog>

ColorPicker::ColorPicker(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorPicker)
{
    ui->setupUi(this);
    ui->button->setStyleSheet("QPushButton {background-color: #ffffff; border: 1px solid black;}");
}

ColorPicker::~ColorPicker()
{
    delete ui;
}

void ColorPicker::on_button_clicked()
{    
    QPoint button_pos = ui->button->cursor().pos();

    QColorDialog *colorDialog = new QColorDialog(this);
    QColor currentColor;

    #if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
        currentColor=QColor::fromString(ui->button->palette().button().color().name());
    #else
        currentColor.setNamedColor(ui->button->palette().button().color().name());
    #endif
    
    colorDialog->setAttribute(Qt::WA_DeleteOnClose);
    colorDialog->setCurrentColor(currentColor);

    connect(colorDialog, &QColorDialog::colorSelected, colorDialog, [=](const QColor &color){
               SetColor(color);
    });

    colorDialog->move(button_pos.x(), button_pos.y());

    colorDialog->open();
}

void ColorPicker::SetColor(QColor color)
{
    ui->button->setStyleSheet("QPushButton {background-color: "+ color.name() + "; border: 1px solid black;}");
    emit ColorSelected(color);
}
