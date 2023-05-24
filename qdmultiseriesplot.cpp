#include "qdmultiseriesplot.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QLineSeries>
#include "qfouriertransformer.h"
#include "globalconfig.h"
QDMultiSeriesPlot::QDMultiSeriesPlot(QWidget *parent, bool isTimeDomain) :
    QWidget(parent),
    pChart(new QChart),
    isTimeDomain(isTimeDomain)

{
    isEnableDb = GlobalConfig::getInstance()->getIsEnableDb();
    if(!isTimeDomain){
        fftTool = new QFourierTransformer;
        fftTool->setSize(DEFAULT_SAMPLE_COUNT);
    }
    for(int i = 0; i < MAX_QDSERIES_LINE_COUNT; i++){
        pSeriesCache[i] = nullptr;
    }
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    //坐标图
    pChartView = new QChartView(pChart);
    pChartView->setMinimumSize(800, 400);
    if(isTimeDomain){
        pChart->setTitle("四通道采样数据展示");
        mainLayout->addWidget(pChartView);
        axisX = new QValueAxis;
        axisX->setRange(0, DEFAULT_SAMPLE_COUNT);
        axisX->setLabelFormat("%g");
        axisX->setTitleText("采样点");
        pChart->addAxis(axisX, Qt::AlignBottom);
        axisY = new QValueAxis;
        axisY->setRange(-1, 1);
        axisX->setTickCount(20);
        axisY->setTitleText("采样电压/V");
        pChart->addAxis(axisY, Qt::AlignLeft);
    }else{
        pChart->setTitle("四通道频域数据展示");
        mainLayout->addWidget(pChartView);
        axisX = new QValueAxis;
        axisX->setRange(0, DEFAULT_SAMPLE_COUNT);
        axisX->setLabelFormat("%g");
        axisX->setTitleText("频率");
        axisX->setTickCount(20);
        pChart->addAxis(axisX, Qt::AlignBottom);
        axisY = new QValueAxis;
        axisY->setRange(-5, 1);
        axisY->setTitleText("能级/dB");
        pChart->addAxis(axisY, Qt::AlignLeft);
    }
}

void QDMultiSeriesPlot::dataCaptured(char* buffer, int sampleCount, int precision)
{
    if(this->isHidden()) return;//当前窗口如果被隐藏，则不进行数据绘制，节省资源
    for(int i = 0; i < MAX_QDSERIES_LINE_COUNT; i++){
        if(pSeriesCache[i] != nullptr){
            int channelId = pSeriesCache[i]->channelId;
            Q_UNUSED(writeData(pSeriesCache[i], buffer + channelId * sampleCount * precision,
                               sampleCount * precision, precision));
        }
    }
}

void QDMultiSeriesPlot::seriesColorChanged(int seriesId, QColor color)
{
    if(seriesId < 0 || seriesId > MAX_QDSERIES_LINE_COUNT) return;
    if(pSeriesCache[seriesId] == nullptr) return;
    if(pSeriesCache[seriesId]->color == color) return;
    pSeriesCache[seriesId]->color = color;
    pSeriesCache[seriesId]->series->setColor(color);
}

void QDMultiSeriesPlot::seriesChannelChanged(int seriesId, int channelId)
{
    if(seriesId < 0 || seriesId > MAX_QDSERIES_LINE_COUNT) return;
    if(pSeriesCache[seriesId] == nullptr) return;
    if(pSeriesCache[seriesId]->channelId == channelId) return;
    pSeriesCache[seriesId]->channelId = channelId;
    if(channelId < 0){
        pSeriesCache[seriesId]->series->setName(QString("通道(未指定)"));
    }else{
        pSeriesCache[seriesId]->series->setName(QString("通道%1").arg(channelId));
    }
}

int QDMultiSeriesPlot::addSeries(int channelId, QColor color, int sampleCount)
{
    bool emptyCacheFound = false;
    int i = 0;
    while(i < MAX_QDSERIES_LINE_COUNT){
        if(pSeriesCache[i] == nullptr){
            emptyCacheFound = true;
            break;
        }
        i++;
    }
    if(!emptyCacheFound) return -1;
    pSeriesCache[i] = new QDSeries();
    pSeriesCache[i]->channelId = channelId;
    pSeriesCache[i]->color = color;
    pSeriesCache[i]->sampleCount = sampleCount;
    QLineSeries* series = new QLineSeries();
    series->setColor(color);
    pChart->addSeries(series);
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    if(channelId < 0){
        series->setName(QString("通道(未指定)"));
    }else{
        series->setName(QString("通道%1").arg(channelId));
    }
    pSeriesCache[i]->series = series;
    return i;
}

qint64 QDMultiSeriesPlot::writeData(QDSeries* qdSeries, const char *data, qint64 maxSize, int precision)
{
    if(isTimeDomain){
        if(qdSeries->channelId < 0){
            qdSeries->series->clear();
            return 0;
        }
        if (qdSeries->buffer.isEmpty()) {
            qdSeries->buffer.reserve(qdSeries->sampleCount);
            for (int i = 0; i < qdSeries->sampleCount; ++i)
                qdSeries->buffer.append(QPointF(i, 0));
        }
        int start = 0;
        const int availableSamples = int(maxSize) / precision;
        if (availableSamples < qdSeries->sampleCount) {
            start = qdSeries->sampleCount - availableSamples;
            for (int s = 0; s < start; ++s)
                qdSeries->buffer[s].setY(qdSeries->buffer.at(s + availableSamples).y());
        }
        for (int s = start; s < qdSeries->sampleCount; ++s, data += precision){
            qdSeries->buffer[s].setY(qreal(*((short*)data)) / 0x8000 * 5.0f);
        }

        qdSeries->series->replace(qdSeries->buffer);
        return (qdSeries->sampleCount - start) * precision;
    }else{
        static int fftResultLength = qdSeries->sampleCount / 2;
        if(qdSeries->channelId < 0){
            qdSeries->series->clear();
            return 0;
        }
        if (qdSeries->buffer.isEmpty()) {
            qdSeries->buffer.reserve(fftResultLength);
            float valueSplit = 85.47e3 * 1.0f / fftResultLength;
            for (int i = 0; i < fftResultLength; ++i)
                qdSeries->buffer.append(QPointF(valueSplit * i, 0));
        }
        int start = 0;
        const int availableSamples = int(maxSize) / precision;
        if (availableSamples < qdSeries->sampleCount) {
            start = qdSeries->sampleCount - availableSamples;
            for (int s = 0; s < start; ++s)
                qdSeries->buffer[s].setY(qdSeries->buffer.at(s + availableSamples).y());
        }
        for (int s = start; s < qdSeries->sampleCount; ++s, data += precision){
            //1. 转换为电压
            sampleValues[s - start] = *((short*)data) * 5.0f / 0x8000;
        }
        //2. 正交解调
        //TODO
        //3. FFT变换
        fftTool->forwardTransform(sampleValues, fftValues);
        for(int i = 1; i < fftResultLength; i++){
            fftValues[i] = sqrt(fftValues[i] * fftValues[i] + fftValues[fftResultLength + i] * fftValues[fftResultLength + i]) / fftResultLength;
            if(isEnableDb && fftValues[i] > 0){
                fftValues[i] = 20.0f * log10f(fftValues[i]);
            }
        }
        for(int s = start; s < fftResultLength; ++s){
            qdSeries->buffer[s].setY(fftValues[s - start]);
        }
        qdSeries->series->replace(qdSeries->buffer);
        return (qdSeries->sampleCount - start) * precision;
    }
}

void QDMultiSeriesPlot::setAxisRange(double xMin, double xMax, double yMin, double yMax)
{
    axisX->setRange(xMin, xMax);
    axisY->setRange(yMin, yMax);
}
