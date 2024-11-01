#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

    void on_pushButton_clicked(bool checked);

private:
    Ui::MainWindow *ui;
    QString verdeEncendido =
        "QPushButton{"
        "background-color: #2ecc71;"
        "border-radius: 40px;"
        "border:2px solid #27ae60;"
        "min-width: 40px;"
        "min-height: 40px;"
        "}";
    QString verdeApagado =
        "QPushButton{"
        "background-color: #95a5a6;"
        "border-radius: 40px;"
        "border:2px solid #7f8c8d;"
        "min-width: 40px;"
        "min-height: 40px;"
        "}";
};
#endif // MAINWINDOW_H
