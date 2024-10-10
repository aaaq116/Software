#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <serialworker.h>
#include <QSerialPortInfo>
#include <QSerialPort>
#include <QMessageBox>
#include <QString>
#include <QDebug>
#include <QFileDialog>
#include <QAxObject>
#include <QProcess>
#include <QThread>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mserialworker = new serialworker();

    mserialworker->moveToThread(&workerThread);

    // 初始化按钮状态标志位
    mIsOpen = false;
    QStringList mseriallist = mserialworker->GetSerialAvailable();
    // 获取可用串口
    ui->Cbox_port->addItems(mseriallist);

    // 将reayRead信号绑定到显示接收内容的槽函数，一旦接收到数据，显示槽就开始显示
    connect(mserialworker, SIGNAL(sendResultToGui(QByteArray)), this, SLOT(SerialPort_readyRead(QByteArray)), Qt::QueuedConnection);

    // 界面配置
    QFont font("SimSun",15);
    ui->Cbox_data_row->setEnabled(false);
    ui->Cbox_data_column->setEnabled(false);

    ui->label_6->setText("行数");
    ui->display_label->setText("请选择烧录数据！");
    ui->display_label->setStyleSheet("background-color:rgb(245,245,245);color:rgb(255,0,0);");
    ui->display_label->setFont(font);
    ui->mesg_label->setStyleSheet("background-color:rgb(245,245,245);color:rgb(255,0,0);");

    // 发送接收字节
    senddatanum = 0;
    recvdatanum = 0;
    // 状态栏
    QStatusBar *sBar = statusBar();
    // 状态栏的收、发计数标签
    lblSendNum = new QLabel(this);
    lblRecvNum = new QLabel(this);
    // 设置标签大小
    lblSendNum->setMinimumSize(100, 20);
    lblRecvNum->setMinimumSize(100, 20);
    // 状态栏显示计数值
    setNumOnLabel(lblSendNum, "S: ", senddatanum);
    setNumOnLabel(lblRecvNum, "R: ", recvdatanum);
    // 从右往左依次添加
    sBar->addPermanentWidget(lblSendNum);
    sBar->addPermanentWidget(lblRecvNum);

    // 设置主窗口背景图片
    ui->logo_display_label->setPixmap(QPixmap("D:/Qt/project/Uart/logo.png"));
    ui->logo_display_label->show();

    // HEX接收数据
    connect(ui->hex_send_Box, &QCheckBox::stateChanged, this, &MainWindow::Qcheck_readyRead);

    // 连接信号和槽
    // 线程结束，自动删除对象
    connect(&workerThread, &QThread::finished, mserialworker, &QObject::deleteLater);
    // 主线程数据发送信号
    connect(this, &MainWindow::serialDataSend, mserialworker, &serialworker::doDataSendWork);

    // 启动线程
    workerThread.start();

    QProcess process;
    QString cmd = "C:\\Users\\cgn12\\Desktop\\token.exe";

    QStringList args;
    args<<QString("products/728cLp65V0/devices/Device_001");

    process.start(cmd, args);
    // 打开读取数据的管道
    process.setReadChannel(QProcess::StandardOutput);

    // 读取数据并输出
    while (process.waitForReadyRead())
    {
        QByteArray data = process.readAll();
        qDebug() << data;
    }

    // 等待进程结束
    process.waitForFinished();

}

MainWindow::~MainWindow()
{
    delete ui;
    workerThread.quit();
    workerThread.wait();
}

void MainWindow::SerialPort_readyRead(QByteArray buffer)
{
    qDebug() << buffer;
    // 接收字节计数
    recvdatanum += buffer.size();
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvdatanum);
    // 读取接收框原先存有数据
    QString str = ui->Recv_line_edit->toPlainText();

    if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
    {
        if(ui->time_Box->checkState() == Qt::Checked)
        {
            QDateTime nowtime = QDateTime::currentDateTime();
            str = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] :";
            str += QString(buffer);
        }
        else
        {
            // 如果接收内容非空
            if(!buffer.isEmpty())
            {
                str += QString(buffer);
            }
        }
        ui->Recv_line_edit->insertPlainText(str);
        ui->Recv_line_edit->moveCursor(QTextCursor::End);
    }
    else
    {
        // 16进制显示，并转换为大写
        QByteArray str1 = str.toUtf8().toHex().toUpper();
        QString str2;

        if(ui->time_Box->checkState() == Qt::Checked)
        {
            QDateTime nowtime = QDateTime::currentDateTime();
            str2 = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] :";
        }

        // 增加空格
        for(int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i,2);
            str2 += " ";
        }

        ui->Recv_line_edit->insertPlainText(QString(str2));
        // 移动光标到文本结尾
        ui->Recv_line_edit->moveCursor(QTextCursor::End);
    }
}

// HEX发送
void MainWindow::Qcheck_readyRead()
{
    QString  buffer;
    // 读取发送框数据
    buffer = ui->Send_line_edit->toPlainText();
    // 判断16进制发送框是否选中
    if(ui->hex_send_Box->checkState() == Qt::Checked)
    {
        // 将发送框数据转化为16进制
        QByteArray str1 = buffer.toUtf8().toHex().toUpper();
        QByteArray str2;
        // 增加空格
        for(int i =0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i,2);
            str2 += " ";
        }
        // 发送框清空，显示新文本
        ui->Send_line_edit->clear();
        ui->Send_line_edit->insertPlainText(QString(str2));
        // 移动光标到文本结尾
        ui->Send_line_edit->moveCursor(QTextCursor::End);
    }
    else
    {
        // 未选中16进制发送时，将数据转换回原始数据
        QByteArray str3 = QByteArray::fromHex(buffer.toUtf8());
        // 发送框清空，显示新文本
        ui->Send_line_edit->clear();
        ui->Send_line_edit->insertPlainText(QString(str3));
        // 移动光标到文本结尾
        ui->Send_line_edit->moveCursor(QTextCursor::End);
    }
}

// 打开串口按钮
void MainWindow::on_open_Button_clicked()
{
    QString mPortName = ui->Cbox_port->currentText();
    QString mBaudRate = ui->Cbox_boudrate->currentText();
    QString mParity = ui->Cbox_parity->currentText();
    QString mDataBits = ui->Cbox_databit->currentText();
    QString mStopBits = ui->Cbox_stopbit->currentText();
    // 匹配带有串口设备信息的文本
    mPortName = mPortName.section(":", 0, 0);

    if(ui->open_Button->text() == QString("打开串口"))
    {
        if(true == mserialworker->SerialPort_init(mPortName, mBaudRate, mParity, mDataBits, mStopBits))
        {
            QMessageBox::information(this,"提示","成功");
            qDebug() << "串口打开成功";
            mIsOpen = true;
            ui->open_Button->setText("关闭串口");
            ui->Cbox_port->setEnabled(false);
            ui->Cbox_boudrate->setEnabled(false);
            ui->Cbox_parity->setEnabled(false);
            ui->Cbox_databit->setEnabled(false);
            ui->Cbox_stopbit->setEnabled(false);
            ui->Send_button->setEnabled(mIsOpen);
        }
        else
        {
            qDebug() << "串口打开失败，请重试";
            QMessageBox::information(this,"提示","失败");
        }
    }
    else
    {
        mserialworker->CloseSerial();
        ui->open_Button->setText("打开串口");
        mIsOpen = false;
        //此时可以配置串口
        ui->Cbox_port->setEnabled(true);
        ui->Cbox_boudrate->setEnabled(true);
        ui->Cbox_parity->setEnabled(true);
        ui->Cbox_databit->setEnabled(true);
        ui->Cbox_stopbit->setEnabled(true);
        ui->Send_button->setEnabled(mIsOpen);
    }
}

// 清除发送窗口按钮
void MainWindow::on_Clear_send_edit_clicked()
{
    ui->Send_line_edit->clear();
    // 发送字节计数
    senddatanum = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvdatanum);
}

// 清除接收窗口按钮
void MainWindow::on_Clear_recv_edit_clicked()
{
    ui->Recv_line_edit->clear();
    // 发送、接收字节计数
    senddatanum = 0;
    recvdatanum = 0;
    // 状态栏显示计数值
    setNumOnLabel(lblRecvNum, "R: ", recvdatanum);
    setNumOnLabel(lblSendNum, "S: ", senddatanum);
}

// 发送数据窗口按钮
void MainWindow::on_Send_button_clicked()
{
    QByteArray data;
    if(ui->hex_send_Box->checkState() == Qt::Checked)
    {
        data = QByteArray::fromHex(ui->Send_line_edit->toPlainText().toUtf8()).data();
    }
    else
    {
        data = ui->Send_line_edit->toPlainText().toLocal8Bit().data();
    }

    // 接收字节计数
    senddatanum = data.size();
    setNumOnLabel(lblRecvNum, "S: ", recvdatanum);
    // 在主线程发送
    // mSerialPort->write(data);
    // 在子线程发送
    emit serialDataSend(data);
}

// 选择文件窗口按钮
void MainWindow::on_select_file_Button_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, QStringLiteral("选择文件"), "F:", QStringLiteral("表格(*xls *xlsx *csv);;图片(*jpg *png);"));

    // 将文件路径显示到UI控件
    ui->select_file_path_lineEdit->setText(fileName);
}

// 打开文件窗口按钮
void MainWindow::on_open_file_Button_clicked()
{
    //创建Excel进程
    QAxObject *excel = new QAxObject(this);

    //QString Token;

    //通过进程获取Excel工作簿集
    excel->setControl("Excel.Application");

    //显示窗体看效果,选择ture将会看到excel表格被打开
    excel->setProperty("Visible", false);
    excel->setProperty("DisplayAlerts", true);

    //获取工作簿(excel文件)集
    QAxObject *workbooks = excel->querySubObject("WorkBooks");

    //文件路径
    QString str = ui->select_file_path_lineEdit->text();

    //打开选定的Excel
    workbooks->dynamicCall("Open(const QString&)", str);

    //获取当前活动的工作薄
    QAxObject *workbook = excel->querySubObject("ActiveWorkBook");

    //获取指定的表格sheet
    QAxObject *worksheet = workbook->querySubObject("WorkSheets(int)",1);

    //获取行列数
    //获取表格中的数据范围
    QAxObject *usedRange = worksheet->querySubObject("UsedRange");
    QAxObject *rows = usedRange->querySubObject("Rows");
    //获取行数
    get_row = rows->property("Count").toInt();
    QAxObject *column = usedRange->querySubObject("Columns");
    //获取列数
    get_column = column->property("Count").toInt();
    qDebug("行数为:%d   列数为:%d\n", get_row, get_column);

    //获取设备名数据
    for(int i=2; i<= get_row; i++)
    {
        for(int j=1; j<=get_column; j++)
        {
            // 存储数据
            QString arr;
            QString buffer;
            buffer = worksheet->querySubObject("Cells(int,int)", i, j)->property("Value2").toString();

            if(j == device_column)
            {
                arr = "*#" + buffer + "*.";

                DeviceName.append(arr);
            }
            else if(j ==id_column)
            {
                arr = "*#" + buffer + "*-";

                ProductId.append(arr);
            }
            else if(j ==sign_column)
            {
                arr = "*#" + buffer + "*+";

                Sign.append(arr);
            }
        }
    }
    // 封装三元组数据
    data.append(DeviceName);
    data.append(ProductId);
    data.append(Sign);

    qDebug()<<data;
    // 清空数据
    ProductId.clear();
    DeviceName.clear();
    Sign.clear();

    workbook->dynamicCall("Close(Boolean)", false);  //关闭文件
    excel->dynamicCall("Quit(void)");  //退出
}

// 读取数据窗口按钮
void MainWindow::on_read_data_Button_clicked()
{
    // 获取行列数
    sl_row = ui->Cbox_data_row->currentIndex();
    sl_column = ui->Cbox_data_column->currentIndex();
    if(ui->Cbox_data_row->isEnabled() == true && ui->Cbox_data_column->isEnabled() == true)
    {
        if(sl_row == 0)
        {
            ui->label_6->setText("Device Name");
            ui->display_label->setText("当前读取数据为：Device Name");
        }
        else if(sl_row == 1)
        {
            ui->label_6->setText("Product Id");
            ui->display_label->setText("当前读取数据为：Product Id");
        }
        else if(sl_row == 2)
        {
            ui->label_6->setText("Sign");
            ui->display_label->setText("当前读取数据为：Sign");
        }
    }
    ui->Send_line_edit->setPlainText(data[sl_row][sl_column]);
}

// 行列数选择窗口按钮
void MainWindow::on_select_Box_stateChanged(int arg1)
{
    // 获取行列数
    sl_row = ui->Cbox_data_row->currentIndex();
    sl_column = ui->Cbox_data_column->currentIndex();
    //获取复选框状态，未选为0，选中为2
    if(arg1 == 0)
    {
        ui->display_label->setText("请选择烧录数据！");
        ui->Cbox_data_row->setEnabled(false);
        ui->Cbox_data_column->setEnabled(false);
        ui->label_6->setText("行数");
    }
    else if(arg1 == 2)
    {       
        ui->Cbox_data_row->setEnabled(true);
        ui->Cbox_data_column->setEnabled(true);
    }
}

// HEX进制接收
void MainWindow::on_hex_recv_Box_stateChanged(int arg1)
{
    int status = arg1;
    // 读取接收框原先存有数据
    QString str = ui->Recv_line_edit->toPlainText();
    QString str2;

    if(status == 0)
    {
        if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
        {
            QByteArray str1 = QByteArray::fromHex(str.toUtf8());

            if(ui->time_Box->checkState() == Qt::Checked)
            {
                QDateTime nowtime = QDateTime::currentDateTime();
                str2 = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] :";
                str2 += QString(str1);
                // 清空显示文本
                ui->Recv_line_edit->clear();
                ui->Recv_line_edit->appendPlainText(str1);
                // 移动光标到文本结尾
                ui->Recv_line_edit->moveCursor(QTextCursor::End);
            }
            else
            {
                // 清空显示文本
                ui->Recv_line_edit->clear();
                ui->Recv_line_edit->insertPlainText(str1);
                // 移动光标到文本结尾
                ui->Recv_line_edit->moveCursor(QTextCursor::End);
            }
        }
    }
    else
    {
        if(ui->hex_recv_Box->checkState() == Qt::Checked)
        {
            // 16进制显示，并转换为大写
            QByteArray str1 = str.toUtf8().toHex().toUpper();
            QString str2;

            if(ui->time_Box->checkState() == Qt::Checked)
            {
                QDateTime nowtime = QDateTime::currentDateTime();
                str2 = "[" + nowtime.toString("yyyy-MM-dd hh:mm:ss") + "] :";
            }
            // 增加空格
            for(int i = 0; i < str1.length(); i += 2)
            {
                str2 += str1.mid(i, 2);
                str2 += " ";
            }
            // 清空显示文本
            ui->Recv_line_edit->clear();
            ui->Recv_line_edit->insertPlainText(str2);
            ui->Recv_line_edit->moveCursor(QTextCursor::End);
        }
    }
}

// SN码烧录
void MainWindow::on_select_sn_Box_stateChanged(int arg1)
{
    int status = arg1;
    QString  buffer;
    // 读取发送框数据
    buffer = ui->Send_line_edit->toPlainText();
    if(status == 0)
    {
        if(ui->select_sn_Box->checkState() == Qt::Unchecked)
        {
            QString  str1;
            QString  str2;
            if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
            {

                str1 = buffer.replace("*#", "");
                str2 = str1.replace("**", "");
            }
            else
            {
                str1 = buffer.replace("2A 23", "");
                str2 = str1.replace("2A 2A", "");
            }
            // if(ui->select_Box->checkState() == Qt::Unchecked)
            // {
            //     ui->Cbox_data_row->setEnabled(false);
            //     ui->Cbox_data_column->setEnabled(false);
            // }
            // else
            // {
            //     ui->Cbox_data_row->setEnabled(true);
            //     ui->Cbox_data_column->setEnabled(true);
            // }

            ui->display_label->setText("请选择烧录数据！");
            // 清空显示文本
            ui->Send_line_edit->clear();
            ui->Send_line_edit->insertPlainText(str2);
            // 移动光标到文本结尾
            ui->Send_line_edit->moveCursor(QTextCursor::End);
        }
    }
    else if(status == 2)
    {
        if(ui->select_sn_Box->checkState() == Qt::Checked)
        {
            ui->display_label->setText("当前烧录SN码数据");
            // if(ui->select_Box->checkState() == Qt::Checked)
            // {
            //     ui->Cbox_data_row->setEnabled(true);
            //     ui->Cbox_data_column->setEnabled(true);
            // }
            // else
            // {
            //     ui->Cbox_data_row->setEnabled(false);
            //     ui->Cbox_data_column->setEnabled(false);
            // }

            QString  str1;

            if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
            {
                str1 = "*#" + buffer + "**";
            }
            else
            {
                str1 = "2A 23" + buffer + "2A 2A";
            }

            // 清空显示文本
            ui->Send_line_edit->clear();
            ui->Send_line_edit->insertPlainText(str1);
            // 移动光标到文本结尾
            ui->Send_line_edit->moveCursor(QTextCursor::End);
        }
    }
}

// 保存文件
void MainWindow::on_save_data_Box_stateChanged(int arg1)
{
    int status = arg1;

    if(status == 2)
    {
        QDateTime nowtime = QDateTime::currentDateTime();

        QString fileName = "text_" + nowtime.toString("yyyyMMdd hhmmss") + ".txt" ;

        QFile file(fileName);

        if(file.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream out(&file);
            out <<"北京时间：" << nowtime.toString("yy-yy-MM-dd hh:mm:ss") << "\n";
            out << ui->Recv_line_edit->toPlainText() << "\n";
            file.close();
            QMessageBox::information(this, "提示", "保存成功！");
        }
        else
        {
            QMessageBox::information(this, "警告", "保存失败！");
        }
    }
}

// Token烧录
void MainWindow::on_select_token_Box_stateChanged(int arg1)
{
    int status = arg1;
    QString  buffer;
    // 读取发送框数据
    buffer = ui->Send_line_edit->toPlainText();
    if(status == 0)
    {
        if(ui->select_token_Box->checkState() == Qt::Unchecked)
        {
            QString  str1;
            QString  str2;
            if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
            {

                str1 = buffer.replace("*#", "");
                str2 = str1.replace("##", "");
            }
            else
            {
                str1 = buffer.replace("2A 23", "");
                str2 = str1.replace("23 23", "");
            }

            ui->display_label->setText("请选择烧录数据！");
            // 清空显示文本
            ui->Send_line_edit->clear();
            ui->Send_line_edit->insertPlainText(str2);
            // 移动光标到文本结尾
            ui->Send_line_edit->moveCursor(QTextCursor::End);
        }
    }
    else if(status == 2)
    {
        if(ui->select_token_Box->checkState() == Qt::Checked)
        {
            ui->display_label->setText("当前烧录Token数据");

            QString  str1;

            if(ui->hex_recv_Box->checkState() == Qt::Unchecked)
            {
                str1 = "*#" + buffer + "##";
            }
            else
            {
                str1 = "2A 23" + buffer + "23 23";
            }

            // 清空显示文本
            ui->Send_line_edit->clear();
            ui->Send_line_edit->insertPlainText(str1);
            // 移动光标到文本结尾
            ui->Send_line_edit->moveCursor(QTextCursor::End);
        }
    }
}

void MainWindow::setNumOnLabel(QLabel *lbl, QString strS, long num)
{
    // 标签显示
    QString strN;
    strN.asprintf("%ld", num);
    QString str = strN + strS;
    lbl->setText(str);
}

