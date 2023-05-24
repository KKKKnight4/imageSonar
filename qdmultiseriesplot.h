#ifndef QDMULTISERIESPLOT_H
#define QDMULTISERIESPLOT_H


#include <QWidget>
#include <QChart>
#include <QLineSeries>
#include <QValueAxis>
#include <QChartView>
#include <QLineSeries>
#include <QLineEdit>
#include <QPushButton>
#define MAX_QDSERIES_LINE_COUNT 4
#define DEFAULT_SAMPLE_COUNT 2048
QT_CHARTS_USE_NAMESPACE
class QFourierTransformer;
struct QDSeries{
    int channelId;
    int sampleCount;
    QColor color;
    QList<QPointF> buffer;
    QLineSeries* series;
};

class QDMultiSeriesPlot : public QWidget
{
    Q_OBJECT
public:
    explicit QDMultiSeriesPlot(QWidget *parent = nullptr, bool isTimeDomain = true);
    int addSeries(int channelId, QColor color, int sampleCount = DEFAULT_SAMPLE_COUNT);
    bool removeSeries(int channelId);
    void setAxisRange(double xMin, double xMax, double yMin, double yMax);
private:
    qint64 writeData(QDSeries* series, const char *data, qint64 maxSize, int precision);
public slots:
    void dataCaptured(char* buffer, int sampleCount, int precision);
    void seriesColorChanged(int seriesId, QColor color);
    void seriesChannelChanged(int seriesId, int channelId);
private:
    QChart* pChart;
    QChartView* pChartView;
    QDSeries* pSeriesCache[MAX_QDSERIES_LINE_COUNT];
    QLineEdit* xMinValue;
    QLineEdit* xMaxValue;
    QLineEdit* yMinValue;
    QLineEdit* yMaxValue;
    QPushButton* okButton;
    QValueAxis* axisX;
    QValueAxis* axisY;
    bool isTimeDomain;
    QFourierTransformer* fftTool;
    float sampleValues[DEFAULT_SAMPLE_COUNT];
    float fftValues[DEFAULT_SAMPLE_COUNT];
    bool isEnableDb;
signals:

};

#endif // QDMULTISERIESPLOT_H
