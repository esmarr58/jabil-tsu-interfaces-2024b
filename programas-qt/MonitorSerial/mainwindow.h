#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:

    void manejarPuertosSeriales(QComboBox *comboBox, QLineEdit *lineEdit, QPushButton *botonEnviar, QPushButton *botonLimpiar, QTextEdit *textEditRespuesta, QCheckBox *checkBoxMarcaTiempo);
private:
    Ui::MainWindow *ui;
    QSerialPort *serialActual = nullptr;

};
#endif // MAINWINDOW_H
