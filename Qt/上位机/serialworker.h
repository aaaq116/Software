#ifndef SERIALWORKER_H
#define SERIALWORKER_H

#include <QObject>
#include <QSerialPort>

class serialworker : public QObject
{
    Q_OBJECT
public:
    explicit serialworker(QObject *parent = nullptr);
    QStringList GetSerialAvailable();
    void CloseSerial();
    bool SerialPort_init(QString mPortName, QString mBaudRate, QString mParity, QString mDataBits, QString mStopBits);
    // explicit SerialListener(QObject *parent = nullptr) : QThread(parent) {}


public slots:
    void doDataSendWork(const QByteArray data);
    void doDataRecvWork();

private:
    // 串口信息
    QSerialPort *mSerialPort;


signals:
    void sendResultToGui(QByteArray data);
};

#endif // SERIALWORKER_H
