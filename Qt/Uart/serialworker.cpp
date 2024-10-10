#include "serialworker.h"

#include <QDebug>
#include <QThread>
#include <QSerialPortInfo>
#include <QSerialPort>

serialworker::serialworker(QObject *parent)
    : QObject{parent}
{
    // 创建串口对象，并建立信号槽
    mSerialPort = new QSerialPort(this);
}

QStringList serialworker::GetSerialAvailable()
{
    QStringList mPortsList;
    // 自动扫描当前可用串口，返回值追加到字符数组中
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString portinfo = info.portName() + ": " + info.description();
        // 携带有串口设备信息的文本
        mPortsList << portinfo;
    }
    return mPortsList;
}


// 子线程发送数据
void serialworker::doDataSendWork(const QByteArray data)
{
    qDebug() <<  "子线程槽函数发送数据：" << data << "线程ID：" << QThread::currentThreadId();
    mSerialPort->write(data);
}

// 子线程接收数据
void serialworker::doDataRecvWork()
{
    // 1.收到数据
    QByteArray buffer = mSerialPort->readAll();

    // 2.进行数据处理
    QByteArray resultStr = buffer;

    qDebug() <<  "子线程收到数据：" << resultStr << "线程ID：" << QThread::currentThreadId();
    // 3.将结果发送到主线程
    emit sendResultToGui(resultStr);
}

bool serialworker::SerialPort_init(QString mPortName, QString mBaudRate, QString mParity, QString mDataBits, QString mStopBits)
{
    //串口配置
    //串口号
    mSerialPort->setPortName(mPortName);
    //波特率
    if("115200" == mBaudRate)
    {
        mSerialPort->setBaudRate(QSerialPort::Baud115200);
    }
    else if("9600" == mBaudRate)
    {
        mSerialPort->setBaudRate(QSerialPort::Baud9600);
    }
    //校验位
    if("EVEN" == mParity)
    {
        mSerialPort->setParity(QSerialPort::EvenParity);
    }
    else if("ODD" == mParity)
    {
        mSerialPort->setParity(QSerialPort::OddParity);
    }
    else
    {
        mSerialPort->setParity(QSerialPort::NoParity);
    }
    //数据位
    if("5" == mDataBits)
    {
        mSerialPort->setDataBits(QSerialPort::Data5);
    }
    else if("6" == mDataBits)
    {
        mSerialPort->setDataBits(QSerialPort::Data6);
    }
    else if("7" == mDataBits)
    {
        mSerialPort->setDataBits(QSerialPort::Data7);
    }
    else
    {
        mSerialPort->setDataBits(QSerialPort::Data8);
    }
    //停止位
    if("1.5" == mStopBits)
    {
        mSerialPort->setStopBits(QSerialPort::OneAndHalfStop);
    }
    if("2" == mStopBits)
    {
        mSerialPort->setStopBits(QSerialPort::TwoStop);
    }
    else
    {
        mSerialPort->setStopBits(QSerialPort::OneStop);
    }
    qDebug() << "配置串口参数成功";

    // 将reayRead信号绑定到显示接收内容的槽函数，一旦接收到数据，显示槽就开始显示
    connect(mSerialPort, &QSerialPort::readyRead, this, &serialworker::doDataRecvWork);

    return mSerialPort->open(QIODevice::ReadWrite);
}

void serialworker::CloseSerial()
{
    if(mSerialPort->isOpen())
    {
        mSerialPort->clear();
        mSerialPort->close();
    }
    qDebug()<<"串口关闭成功";
}
