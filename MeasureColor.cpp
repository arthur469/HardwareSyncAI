#include "MeasureColor.h"
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
        int value = color_stop->GetValue();
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

std::tuple<unsigned int, QColor> MeasureColor::GetColor(double val, int last_idx)
{
    if(max_value == 0 || max_value == min_value)
    {
        std::tuple<int, QColor>(0, Qt::black);
    }

    int idx = std::max<int>(0, std::min<int>(99, 100 * ((val - min_value) / (max_value - min_value))));

    if(idx < last_idx)
    {
        idx++;
    }
    else if(idx > last_idx)
    {
        idx--;
    }

    return std::tuple<int, QColor>(idx, image.pixelColor(idx, 0));
}
