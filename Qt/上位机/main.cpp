#include "mainwindow.h"
#include <Python.h>
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QStandardPaths>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // 初始化python
    Py_Initialize();
    // 判定是否初始化成功
    if(!Py_IsInitialized())
    {
        qDebug() <<"isinitialized error!";
    }
    // 导入sys模块设置模块地址，以及python脚本路径
    PyRun_SimpleString("import sys");
    // 该相对路径是以build..为参考的
    PyRun_SimpleString("sys.path.append('../Uart')");
    // 加载 python 脚本,不带.py
    PyObject *pModule = PyImport_ImportModule("get_token");
    // 脚本加载成功与否
    if(!pModule)
        qDebug()<<"[db:] pModule fail";
    else
        qDebug()<<"[db:] pModule success";

    // 创建函数指针
    PyObject* pFunc= PyObject_GetAttrString(pModule, "read_excel_data");
    // 函数是否创建成功
    if(!pFunc || !PyCallable_Check(pFunc))
        qDebug()<<"[db:] pFunc fail";
    else
        qDebug()<<"[db:] pFunc success";

    // 选择打开文件
    QString filepath = QFileDialog::getOpenFileName();
    QFileInfo fi = QFileInfo(filepath);
    // 传递参数
    PyObject* args =PyTuple_New(2);
    // 烧录文件路径
    QString path = fi.absolutePath();
    qDebug()<<path;
    // 烧录文件名
    QString filename = fi.fileName();
    qDebug()<<filename;
    PyTuple_SetItem(args,0,Py_BuildValue("s",path.toStdString().c_str()));
    PyTuple_SetItem(args,1,Py_BuildValue("s",filename.toStdString().c_str()));

    // 调用函数
    PyObject_CallObject(pFunc, args);

    // 并销毁自上次调用Py_Initialize()以来创建并为被销毁的所有子解释器。
    Py_Finalize();

    MainWindow w;
    w.show();
    return a.exec();
}
