#include "MeasureColor.h"
#include "RGBController.h"
#include <QPainter>

MeasureColor::MeasureColor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeasureColor)
{
    ui->setupUi(this);
    ui->color_stops->setLayout(new QHBoxLayout);

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
}

void MeasureColor::AddColorStop(ColorStop* color_stop)
{
    color_stops.push_back(color_stop);

    ui->color_stops->layout()->addWidget(color_stop);
    ui->color_stops->layout()->setAlignment(color_stop, Qt::AlignLeft | Qt::AlignTop);

    connect(color_stop, &ColorStop::GradientStopChanged,this, &MeasureColor::Update);

    connect(color_stop, &ColorStop::RemoveRequest, [=](){
        ui->color_stops->layout()->removeWidget(color_stop);
        color_stops.erase(std::find(color_stops.begin(), color_stops.end(), color_stop));
        delete color_stop;
        Update();
    });

    Update();
}

void MeasureColor::Update()
{
    image = QImage(100, 1, QImage::Format_RGB32);

    QGradientStops stops;

    max_value = 0;
    min_value = 1000000;

    for(ColorStop* color_stop: color_stops)
    {
        double value = color_stop->GetValue();
        max_value = value > max_value ? value : max_value;
        min_value = value < min_value ? value : min_value;
    }

    if(max_value == 0 || max_value == min_value)
    {
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

    QPointF start_point(0,0);
    QPointF end_point(100,0);

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
    if(max_value == 0 || max_value == min_value)
    {
        return std::tuple<float, QColor>(0, Qt::black);
    }

    float idx = std::max<float>(0, std::min<float>(99, 100 * ((val - min_value) / (max_value - min_value))));

    float delta = 1./fps;

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
    QLayoutItem* item;

    while ((item = ui->color_stops->layout()->takeAt(0)) != NULL)
    {
        delete item->widget();
        delete item;
    }

    color_stops.clear();

    for(const std::tuple<double, RGBColor> entry : j["color_stops"])
    {
        ColorStop* stop = new ColorStop();
        RGBColor rgb = std::get<1>(entry);
        QColor color(RGBGetRValue(rgb), RGBGetGValue(rgb), RGBGetBValue(rgb));
        stop->SetColor(color);
        stop->SetStop(std::get<0>(entry));
        AddColorStop(stop);
    }
}



