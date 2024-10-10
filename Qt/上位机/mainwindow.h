#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QProcess>
#include <QList>
#include <QLabel>
#include <QThread>
#include <serialworker.h>

// #define device_column 1
// #define id_column 4
// #define sign_column 5

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    QThread workerThread;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


// private slots区内申明，只有当前类可以将信号与之相连接
private slots:
    void on_open_Button_clicked();

    void on_Clear_send_edit_clicked();

    void on_Clear_recv_edit_clicked();

    void on_Send_button_clicked();

    void Qcheck_readyRead();

    void on_select_file_Button_clicked();

    void on_open_file_Button_clicked();

    void on_read_data_Button_clicked();

    void on_hex_recv_Box_stateChanged(int arg1);

    void on_select_sn_Box_stateChanged(int arg1);

    void on_save_data_Box_stateChanged(int arg1);

    QString ByteArrayToHexString(QByteArray data);

    //void on_select_token_Box_stateChanged(int arg1);

    void on_read_excel_date_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    // 串口按钮是否打开标志
    bool mIsOpen;
    // 待烧录Token文件的行数
    int get_row;
    // 待烧录Token文件的列数
    int get_column;
    // 已烧录Token文件的行数
    int get_old_row;
    // 已烧录Token文件的列数
    int get_old_column;
    // 烧录数据的行数
    int sl_row;
    // 烧录数据的列数
    int sl_column;

    serialworker *mserialworker;
    // 发送、接收字节数
    long send_data_num, recv_data_num;
    QLabel *lblSendNum;
    QLabel *lblRecvNum;
    void setNumOnLabel(QLabel *lbl, QString strS, long num);

    // 需读取文件信息
    QList<QList<QString>> data;
    // QList<QString> ProductId;
    // QList<QString> DeviceName;
    //QList<QString> Token;

signals:
    void serialDataSend(const QByteArray data);

// public slots区内声明的槽意味着任何对象都可将信号与之相连接
public slots:
    void SerialPort_readyRead(QByteArray buffer);

};
#endif // MAINWINDOW_H
