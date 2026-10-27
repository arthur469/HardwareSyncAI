#include "MeasureColor.h"
#include "RGBControllerInterface.h"
#include <QPainter>

/*---------------------------------------------------------*\
| A single color stop within a preset gradient.  The value  |
| is the measurement (temperature, %, etc) at which the     |
| color is fully applied.                                   |
\*---------------------------------------------------------*/
struct preset_color_stop
{
    double          value;
    unsigned char   red;
    unsigned char   green;
    unsigned char   blue;
};

/*---------------------------------------------------------*\
| A named gradient preset made up of two or more color      |
| stops.                                                    |
\*---------------------------------------------------------*/
struct gradient_preset
{
    std::string                     name;
    std::vector<preset_color_stop>  stops;
};

/*---------------------------------------------------------*\
| Curated gradient presets.  These are offered in the       |
| preset selector in addition to the "Custom" entry, which  |
| is always index 0 and leaves the stops untouched.         |
\*---------------------------------------------------------*/
static const std::vector<gradient_preset> presets =
{
    {
        "Thermal (Cool to Hot)",
        {
            {  30,   0,   0, 255 },     /* Blue                         */
            {  50,   0, 255,   0 },     /* Green                        */
            {  70, 255, 255,   0 },     /* Yellow                       */
            {  80, 255, 165,   0 },     /* Orange                       */
            {  90, 255,   0,   0 },     /* Red                          */
        }
    },
    {
        "Classic (Green to Red)",
        {
            {   0,   0, 255,   0 },     /* Green                        */
            {  50, 255, 255,   0 },     /* Yellow                       */
            { 100, 255,   0,   0 },     /* Red                          */
        }
    },
    {
        "Cool to Warm (Blue to Red)",
        {
            {   0,   0,   0, 255 },     /* Blue                         */
            { 100, 255,   0,   0 },     /* Red                          */
        }
    },
};

MeasureColor::MeasureColor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeasureColor)
{
    ui->setupUi(this);
    ui->color_stops->setLayout(new QHBoxLayout);

    /*-----------------------------------------------------*\
    | Populate the preset selector.  Index 0 is always      |
    | "Custom"; the curated presets follow.  Block signals  |
    | while filling so the default stops below are not      |
    | overwritten during construction.                      |
    \*-----------------------------------------------------*/
    ui->preset_selector->blockSignals(true);
    ui->preset_selector->addItem("Custom");

    for(unsigned int i = 0; i < presets.size(); i++)
    {
        ui->preset_selector->addItem(QString::fromStdString(presets[i].name));
    }

    ui->preset_selector->blockSignals(false);

    ColorStop* green = new ColorStop(this);
    green->SetColor(Qt::green);
    green->SetStop(0);

    ColorStop* yellow = new ColorStop(this);
    yellow->SetColor(Qt::yellow);
    yellow->SetStop(50);

    ColorStop* red = new ColorStop(this);
    red->SetColor(Qt::red);
    red->SetStop(100);

    AddColorStop(green);
    AddColorStop(yellow);
    AddColorStop(red);

}

MeasureColor::~MeasureColor()
{
    delete ui;
}

void MeasureColor::on_add_color_clicked()
{
    ColorStop* stop = new ColorStop(this);
    stop->SetColor(Qt::white);
    stop->SetStop(0);
    AddColorStop(stop);

    /*-----------------------------------------------------*\
    | Adding a stop by hand makes this a custom gradient    |
    \*-----------------------------------------------------*/
    SetPresetSelectorToCustom();
}

void MeasureColor::on_preset_selector_currentIndexChanged(int index)
{
    /*-----------------------------------------------------*\
    | Index 0 is "Custom"; leave the existing stops alone.  |
    | Preset entries follow, so subtract one to index into  |
    | the presets table.                                    |
    \*-----------------------------------------------------*/
    if(index <= 0)
    {
        return;
    }

    ApplyPreset(index - 1);
}

void MeasureColor::AddColorStop(ColorStop* color_stop)
{
    color_stops.push_back(color_stop);

    ui->color_stops->layout()->addWidget(color_stop);
    ui->color_stops->layout()->setAlignment(color_stop, Qt::AlignLeft | Qt::AlignTop);

    connect(color_stop, &ColorStop::GradientStopChanged,this, &MeasureColor::Update);

    /*-----------------------------------------------------*\
    | Editing a stop's color or value turns the gradient    |
    | into a custom one.  The stop's initial color/value    |
    | are set before this connection is made (in the        |
    | constructor and ApplyPreset), so applying a preset    |
    | does not trip this and reset the selector.            |
    \*-----------------------------------------------------*/
    connect(color_stop, &ColorStop::GradientStopChanged, this, &MeasureColor::SetPresetSelectorToCustom);

    connect(color_stop, &ColorStop::RemoveRequest, [=](){
        if(color_stops.size()<=1)return;
        ui->color_stops->layout()->removeWidget(color_stop);
        color_stops.erase(std::find(color_stops.begin(), color_stops.end(), color_stop));
        delete color_stop;
        SetPresetSelectorToCustom();
        Update();
    });

    Update();
}

void MeasureColor::ClearColorStops()
{
    QLayoutItem* item;

    while((item = ui->color_stops->layout()->takeAt(0)) != NULL)
    {
        delete item->widget();
        delete item;
    }

    color_stops.clear();
}

void MeasureColor::ApplyPreset(int preset_index)
{
    if(preset_index < 0 || preset_index >= (int)presets.size())
    {
        return;
    }

    ClearColorStops();

    const gradient_preset& preset = presets[preset_index];

    for(unsigned int i = 0; i < preset.stops.size(); i++)
    {
        const preset_color_stop& s = preset.stops[i];

        ColorStop* stop = new ColorStop(this);
        stop->SetColor(QColor(s.red, s.green, s.blue));
        stop->SetStop(s.value);
        AddColorStop(stop);
    }
}

void MeasureColor::SetPresetSelectorToCustom()
{
    /*-----------------------------------------------------*\
    | Block signals so resetting the selector to "Custom"   |
    | does not re-trigger ApplyPreset.                      |
    \*-----------------------------------------------------*/
    ui->preset_selector->blockSignals(true);
    ui->preset_selector->setCurrentIndex(0);
    ui->preset_selector->blockSignals(false);
}

void MeasureColor::Update()
{
    image = QImage(100, 1, QImage::Format_RGB32);
    for(auto stop:color_stops)stop->SetRemoveEnabled(color_stops.size()>1);

    QGradientStops stops;

    max_value = 0;
    min_value = 1000000;

    for(ColorStop* color_stop: color_stops)
    {
        double value = color_stop->GetValue();
        max_value = value > max_value ? value : max_value;
        min_value = value < min_value ? value : min_value;
    }

    if(color_stops.empty())
    {
        image.fill(Qt::black);
        return;
    }
    if(max_value == min_value)
    {
        image.fill(color_stops.back()->GetColor());
        ui->preview->setPixmap(QPixmap::fromImage(image));
        return;
    }

    for(ColorStop* color_stop: color_stops)
    {
        QGradientStop stop;
        stop.first = std::max<float>(0, std::min<float>(1, ((color_stop->GetValue() - min_value) / (max_value - min_value))));
        stop.second = color_stop->GetColor();
        stops << stop;
    }

    QGradient::Spread spread = QGradient::PadSpread;

    QPointF start_point(.5,0);
    QPointF end_point(99.5,0);

    QLinearGradient grad(start_point , end_point);

    grad.setSpread(spread);
    grad.setStops(stops);

    QBrush brush(grad);

    QRectF rect(0, 0, 100, 1);

    QPainter painter(&image);
    painter.fillRect(rect, brush);

    ui->preview->setPixmap(QPixmap::fromImage(image));
}

std::tuple<float, QColor> MeasureColor::GetColor(double val, float last_idx, int fps)
{
    if(std::isnan(val) || std::isinf(val))
    {
        val = min_value;
    }

    if(color_stops.empty())
    {
        return std::tuple<float, QColor>(0, Qt::black);
    }
    if(max_value == min_value)return std::tuple<float,QColor>(0,image.pixelColor(0,0));

    float idx = std::max<float>(0, std::min<float>(99, 100 * ((val - min_value) / (max_value - min_value))));

    float delta = 1./std::max(1,fps);

    float gap = fabs(last_idx - idx);

    if(idx < last_idx)
    {
        idx = last_idx - gap * delta;
    }
    else if(idx > last_idx)
    {
        idx = last_idx + gap * delta;
    }

    //printf("idx=%f, last_idx=%f, gap=%f, delta=%f\n", idx, last_idx,gap, delta);

    return std::tuple<float, QColor>(idx, image.pixelColor((unsigned int)idx, 0));
}

json MeasureColor::ToJson()
{
    json j;

    std::vector<std::tuple<double, RGBColor>> tmp;

    for(ColorStop* stop: color_stops)
    {
        QColor color = stop->GetColor();
        RGBColor rgb = ToRGBColor(color.red(), color.green(), color.blue());
        tmp.push_back(std::tuple<double, RGBColor>(
                          stop->GetValue(), rgb
                          ));
    }

    j["color_stops"] = tmp;

    return j;
}

void MeasureColor::LoadJson(json j)
{
    ClearColorStops();

    for(const std::tuple<double, RGBColor> entry : j["color_stops"])
    {
        ColorStop* stop = new ColorStop();
        RGBColor rgb = std::get<1>(entry);
        QColor color(RGBGetRValue(rgb), RGBGetGValue(rgb), RGBGetBValue(rgb));
        stop->SetColor(color);
        stop->SetStop(std::get<0>(entry));
        AddColorStop(stop);
    }

    /*-----------------------------------------------------*\
    | A loaded gradient is an arbitrary set of stops, so    |
    | show it as "Custom" rather than any named preset.     |
    \*-----------------------------------------------------*/
    SetPresetSelectorToCustom();
}



