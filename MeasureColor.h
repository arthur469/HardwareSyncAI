#ifndef MEASURECOLOR_H
#define MEASURECOLOR_H

#include <QWidget>
#include <QImage>
#include "ColorStop.h"
#include "ui_MeasureColor.h"
#include "json.hpp"

using json = nlohmann::json;

namespace Ui {
class MeasureColor;
}

class MeasureColor : public QWidget
{
    Q_OBJECT

public:
    explicit MeasureColor(QWidget *parent = nullptr);
    ~MeasureColor();

    std::tuple<unsigned int, QColor> GetColor(double, int);
    json ToJson();

private slots:
    void on_add_color_clicked();

private:
    Ui::MeasureColor *ui;
    QImage image;
    std::vector<ColorStop*> color_stops;
    void AddColorStop(ColorStop* color);
    void Update();

    float min_value;
    float max_value;
};

#endif // MEASURECOLOR_H
