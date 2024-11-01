#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton->setStyleSheet(verdeApagado);
    ui->pushButton_2->setStyleSheet(verdeApagado);

    ui->pushButton_3->setStyleSheet(verdeApagado);
    ui->pushButton_4->setStyleSheet(verdeApagado);

}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked(bool checked)
{
    if(checked){
        ui->pushButton->setStyleSheet(verdeEncendido);

    }
    else{
        ui->pushButton->setStyleSheet(verdeApagado);

    }
}

