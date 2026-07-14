#ifndef MEASURECOLOR_H
#define MEASURECOLOR_H

#include <QWidget>
#include <QImage>
#include <nlohmann/json.hpp>
#include "ColorStop.h"
#include "ui_MeasureColor.h"

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

    std::tuple<float, QColor> GetColor(double, float, int);
    json ToJson();
    void LoadJson(json);

    float min_value;
    float max_value;

private slots:
    void on_add_color_clicked();
    void on_preset_selector_currentIndexChanged(int index);

private:
    Ui::MeasureColor *ui;
    QImage image;
    std::vector<ColorStop*> color_stops;
    void AddColorStop(ColorStop* color);
    void ClearColorStops();
    void ApplyPreset(int preset_index);
    void SetPresetSelectorToCustom();
    void Update();
};

#endif // MEASURECOLOR_H
