#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QUdpSocket>
#include <QList>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

#define TIMER_NUM 20000

enum task_mode_type {
    TASK_MODE_NORMAL=0,
    TASK_MODE_DATA_READBACK=1,
    TASK_MODE_SIM=2,
    TASK_MODE_TEST_POOL=3,
    TASK_MODE_WAIT=4};


class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = nullptr);
    ~Dialog();
    static bool run;

private slots:
    void on_btnSendParam_clicked();
    void socket_Read_Data();
    void socket_Disconnected();

    void on_btnWorkModeTest_clicked();
    void on_btnStop_clicked();
    void on_pushButton_clicked();


private:
    Ui::Dialog *ui;

    QUdpSocket *socket;

    QFile *file;
    QDataStream *stream;

};
#endif // DIALOG_H
