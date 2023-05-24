#include "dialog.h"
#include "ui_dialog.h"
#include <QDateTime>
#include <QTextStream>
#include <QDoubleValidator>
#include <QStyleFactory>
#include <QTextCodec>
#include <QThread>
#include <cstring>
#include <QtMath>
#include <QPoint>
#include <QMessageBox>
#include <QTime>

using namespace std;

void Dialog::plot(char *name){

    short int data[64*2048];
    float data_all[64*2048];
    float data_real[32*2048];
    float data_image[32*2048];
    QVector<double> real;
    QVector<double> image;
    QVector<double> x;
    QVector<double> y;
    QVector<double> ft;

    //
    for(int i=0;i<2048;i++)
        x.push_back(i/10000.0);

    //读数据
    FILE *fid;
    fid = fopen(name, "rb");
    fread(data,sizeof(short int),64*2048,fid);
    fclose(fid);

    //调整幅度
    for(int i=0;i<64*2048;i++)
        data_all[i]=data[i]*3*4.04*1000/65536;


    //UTP

    //分离实虚
    for(int i=0;i<32*2048;i++){
        data_real[i]=data_all[i*2];
        data_image[i]=data_all[i*2+1];
    }

    //取出第一组数据
    for(int i=0;i<2048;i++){
        real.push_back(data_real[32*i]);
        image.push_back(data_image[32*i]);
    }

    //fft的数据处理
    int N=2048;
    complex<double> f[2048];
    complex<double> F[2048];
    for(int i=0;i<2048;i++){
        complex<double> val(data_real[32*i], data_image[32*i]);
        f[i]=val;
    }

    complex<double> *p_f=f;
    complex<double> *p_F=F;
    fft(p_f,N,p_F);


    for(int i=0;i<2048;i++)
        ft.push_back(abs(F[i])/2048);


    double df=10000.0/2048;

    for(int i=0;i<2048;i++){
        y.push_back((i-1024)*df);
    }
    for(int i=0;i<1024;i++){
        double temp=ft[i];
        ft[i]=ft[i+1024];
        ft[i+1024]=temp;
    }
    auto m=max_element(ft.begin(),ft.end());
    double mm = *m;



    //绘制曲线
    QwtPlotCurve *curvep_real =new QwtPlotCurve;
    QwtPlotCurve *curvep_image =new QwtPlotCurve;
    QwtPlotCurve *curvep_fft =new QwtPlotCurve;
    curvep_real->setPen(Qt::black, 1);
    curvep_image->setPen(Qt::black, 1);
    curvep_fft->setPen(Qt::black, 1);

    //清除画布上原有的图像
    ui->plot_real->detachItems();
    ui->plot_image->detachItems();
    ui->plot_fft->detachItems();

    QwtPointArrayData * const line_real = new QwtPointArrayData(x,real);
    QwtPointArrayData * const line_image = new QwtPointArrayData(x,image);
    QwtPointArrayData * const line_fft = new QwtPointArrayData(y,ft);


    //设置图像范围

    ui->plot_real->setTitle("实部 时频图");
    ui->plot_real->setAxisScale(QwtPlot::xBottom,0,0.2048,0.04);
    ui->plot_real->setAxisScale(QwtPlot::yLeft,-2000,2000,1000);
    ui->plot_image->setTitle("虚部 时频图");
    ui->plot_image->setAxisScale(QwtPlot::xBottom,0,0.2048,0.04);
    ui->plot_image->setAxisScale(QwtPlot::yLeft,-2000,2000,1000);
    ui->plot_fft->setTitle("频域图");
    ui->plot_fft->setAxisScale(QwtPlot::xBottom,-5000,5000,1000);
    ui->plot_fft->setAxisScale(QwtPlot::yLeft,-100,mm);


    //设置曲线的数据
    curvep_real->setPen(QColor(Qt::black),2);
    curvep_real->setStyle(QwtPlotCurve::Steps);
    curvep_real->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curvep_real->setData(line_real);
    curvep_real->attach(ui->plot_real);

    curvep_image->setPen(QColor(Qt::black),2);
    curvep_image->setStyle(QwtPlotCurve::Steps);
    curvep_image->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curvep_image->setData(line_image);
    curvep_image->attach(ui->plot_image);

    curvep_fft->setPen(QColor(Qt::black),2);
    curvep_fft->setStyle(QwtPlotCurve::Steps);
    curvep_fft->setCurveAttribute(QwtPlotCurve::Fitted, true);
    curvep_fft->setData(line_fft);
    curvep_fft->attach(ui->plot_fft);


    ui->plot_real->replot();
    ui->plot_image->replot();
    ui->plot_fft->replot();
}


void sleep(unsigned int msec){
//currnentTime 返回当前时间 用当前时间加上我们要延时的时间msec得到一个新的时刻
    QTime reachTime = QTime::currentTime().addMSecs(msec);
    //用while循环不断比对当前时间与我们设定的时间
    while(QTime::currentTime()<reachTime){
    //如果当前的系统时间尚未达到我们设定的时刻，就让Qt的应用程序类执行默认的处理，
    //以使程序仍处于响应状态。一旦到达了我们设定的时刻，就跳出该循环，继续执行后面的语句。
    QApplication::processEvents(QEventLoop::AllEvents,100);
    }
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    socket = new QUdpSocket();
    file = new QFile();
    stream = new QDataStream();

    //plot("data.dat");
    //初始化Udp
    socket->bind(QHostAddress::Any, 1234);  //端口设为1234
    QObject::connect(socket, &QUdpSocket::readyRead, this, &Dialog::socket_Read_Data,Qt::QueuedConnection);

    ui->btnSendParam->setDisabled(false);
    ui->btnWorkModeTest->setDisabled(true);
    ui->btnWorkModeRead->setDisabled(true);
    ui->btnWorkModeNormal->setDisabled(true);
    ui->btnStop->setDisabled(true);


    //设置输入参数的大小
    ui->lineEdit_gain_dB->setValidator(new QIntValidator(20, 80, this));
    ui->lineEdit_pulse_ms->setValidator(new QDoubleValidator(0,1000,2,this));
    ui->lineEdit_endFreq_khz->setValidator(new QDoubleValidator(35,45,2,this));
    ui->lineEdit_start_freq_khz->setValidator(new QDoubleValidator(35,45,2,this));
    ui->lineEdit_rcv_time_ms->setValidator(new QDoubleValidator(0,2000,2,this));

}

Dialog::~Dialog()
{
    delete ui;
}



//是上位机读取返回的数据，这里就不需要改了
void Dialog::socket_Read_Data()
{
    static int flag_file = 0;//文件创建成功标志
    static int file_len = 0;//缓冲区数据长度
    static int file_wr_cnt = 0;//存储数据长度
    static string temp;


    //len：缓冲区数据个数
    int len = socket->bytesAvailable();
    if(len <= 0)
        return;
    ui->plainTextEdit->appendPlainText(
        QString::asprintf("tcp_rcv_len = %dB",len)
    );

    //读取缓冲区数据
    if((len >= 8) &&(flag_file == 0)){
        //读取数据长度
        int data[2];
        socket->read((char*)data,8);//每次读取八个字节的数据
        //首位置存放数据长度，总长度减去int为实际长度
        file_len = data[0] - sizeof(int);
        flag_file = 1;
        file_wr_cnt = 0;
        //这里创建文件
        QDateTime current_date_time =QDateTime::currentDateTime();
        QString current_date =current_date_time.toString("yyyyMMdd-hhmmss");
        QString str = ".\\..\\data\\"  + current_date ;
        temp=str.toStdString()+".dat";
        file->setFileName(str+".dat");
        stream->setDevice(file);
        if(!file->open(QIODevice::WriteOnly))
            qDebug() << "file open is error";
        qDebug() << "文件长度字节数=" << file_len;
    }

    //这里存储文件
    if(flag_file){
        QByteArray byteArray = socket->readAll();
        char* buf = byteArray.data();
        stream->writeRawData(buf,byteArray.length());
        file_wr_cnt += byteArray.length();
        if(file_wr_cnt == file_len){ 
            char *name=const_cast<char *>(temp.data());
            plot(name);

            flag_file = 0;
            file_len = 0;
            file_wr_cnt = 0;
            //这里关闭文件
            file->close();
            qDebug() << "file close";
        }
    }
}


void Dialog::socket_Disconnected()
{
     ui->plainTextEdit->appendPlainText(  "socket_Disconnected!");
     ui->btnWorkModeTest->setDisabled(true);
     ui->btnSendParam->setDisabled(true);
     ui->btnWorkModeRead->setDisabled(true);
     ui->btnWorkModeNormal->setDisabled(true);
}


void Dialog::on_pushButton_clicked()
{
    ui->plainTextEdit->clear();
}

void Dialog::on_btnStop_clicked(){
    Dialog::run=false;
    ui->btnSendParam->setDisabled(false);
    ui->btnStop->setDisabled(true);
}


//如果点击了 水池测试按钮
void Dialog::on_btnWorkModeTest_clicked()
{
    int buf[3];
    buf[0] = 2;
    buf[1] = 0;
    //先传入一个wait模式让下位机等着  枚举对应的值是4  等点击传入参数的时候调整模式
    buf[2] = TASK_MODE_TEST_POOL;
    socket->writeDatagram((char*)buf,sizeof(buf),QHostAddress("192.168.1.10"),7);

    ui->btnWorkModeTest->setDisabled(true);
    ui->btnSendParam->setDisabled(false);
}

bool Dialog::run = false;

//后面扩展的时候再加入新的结构调整 初步解决方法是switch三个模式
void Dialog::on_btnSendParam_clicked()
{
    bool ok;
    Dialog::run=true;
    int gain_dB;
    float pulse_ms,start_freq_khz,endFreq_khz,rcv_time_ms;
    //1帧包含的内容
    //0. 帧头：数据长度（不包括本身） 1.输入模式 2.接收增益 3.起始频率 4.终止频率 5.发射脉宽 6.接收时长 7.发射阵列 8.阵列数据（4组）  总共13*4=48
    ui->btnStop->setDisabled(false);
    ui->btnSendParam->setDisabled(true);

    //增益码
    gain_dB = ui->lineEdit_gain_dB->text().toInt(&ok);
    if(ok != true){
        QMessageBox::critical(this,"输入错误","增益码输入错误");
        return;
    }

    //脉宽
    pulse_ms = ui->lineEdit_pulse_ms->text().toFloat(&ok);
    if(ok != true){
        QMessageBox::critical(this,"输入错误","脉宽输入错误");
        return;
    }


    //发射起始频率
    start_freq_khz = ui->lineEdit_start_freq_khz->text().toFloat(&ok);
    if(ok != true){
        QMessageBox::critical(this,"输入错误","发射起始频率输入错误");
        return;
    }

    //发射结束频率
    endFreq_khz = ui->lineEdit_endFreq_khz->text().toFloat(&ok);
    if(ok != true){
        QMessageBox::critical(this,"输入错误","发射结束频率输入错误");
        return;
    }

    //接收时间长度
    rcv_time_ms = ui->lineEdit_rcv_time_ms->text().toFloat(&ok);
    if(ok != true){
        QMessageBox::critical(this,"输入错误","发射结束频率输入错误");
        return;
    }

    //发射列选择
    int send_select = 0;
    if(ui->checkBox_send_1->isChecked()){
        send_select += 0x0001;
    }
    if(ui->checkBox_send_2->isChecked()){
        send_select += 0x0002;
    }
    if(ui->checkBox_send_3->isChecked()){
        send_select += 0x0004;
    }
    if(ui->checkBox_send_4->isChecked()){
        send_select += 0x0008;
    }
    if(send_select == 0){
        QMessageBox::critical(this,"发射列选择错误","没有1个发射列被选择");
        return;
    }


    //阵列数据   这里拟定实现方法为  每次将四个阵列的数据都要全部传入（默认为零）
    gain_dB = ui->lineEdit_gain_dB->text().toInt(&ok);
    int array0=0;
    int array1=0;
    int array2=0;
    int array3=0;
    if(ui->checkBox_send_1->isChecked())
        array0=ui->lineEdit_delay_1->text().toInt(&ok);
    if(ui->checkBox_send_2->isChecked())
        array1=ui->lineEdit_delay_2->text().toInt(&ok);
    if(ui->checkBox_send_3->isChecked())
        array2=ui->lineEdit_delay_3->text().toInt(&ok);
    if(ui->checkBox_send_4->isChecked())
        array3=ui->lineEdit_delay_4->text().toInt(&ok);


    //1帧包含的内容

    char buf[44];
    char *p=buf;

    //根据选择来输入模式
    int length=44;

    memcpy(p, &length, sizeof(int)); //0-数据长度
    p += sizeof(int);
    memcpy(p, &gain_dB, sizeof(int)); //1-接收增益
    p += sizeof(int);
    memcpy(p, &pulse_ms, sizeof(int));//2-发射脉宽
    p += sizeof(int);
    memcpy(p, &start_freq_khz, sizeof(int));//3-开始频率
    p += sizeof(int);
    memcpy(p, &endFreq_khz, sizeof(int));//4-截止频率
    p += sizeof(int);
    memcpy(p, &rcv_time_ms, sizeof(int));//5-接收时长
    p += sizeof(int);
    memcpy(p, &send_select, sizeof(int));//6-发射阵列选择  0000从高到低分别对应通道3210
    p += sizeof(int);
    memcpy(p, &array0, sizeof(int));//7、8、9、10：发射的延迟
    p += sizeof(int);
    memcpy(p, &array1, sizeof(int));
    p += sizeof(int);
    memcpy(p, &array2, sizeof(int));
    p += sizeof(int);
    memcpy(p, &array3, sizeof(int));



    qDebug() << "on_btnSendParam_clicked";

    //while(run){
        socket->writeDatagram(buf,sizeof(buf),QHostAddress("192.168.1.10"),7);
        //每隔0.5s发送一次数据
        sleep(500);
    //}

}
