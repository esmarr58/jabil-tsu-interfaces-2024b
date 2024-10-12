#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    manejarPuertosSeriales(ui->comboBox, ui->lineEdit, ui->pushButton, ui->pushButton_2, ui->textEdit, ui->checkBox);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::manejarPuertosSeriales(QComboBox *comboBox, QLineEdit *lineEdit, QPushButton *botonEnviar, QPushButton *botonLimpiar, QTextEdit *textEditRespuesta, QCheckBox *checkBoxMarcaTiempo)
{
    // Limpiar cualquier elemento previo en el ComboBox
    comboBox->clear();

    // Añadir la opción por defecto "Seleccionar puerto"
    comboBox->addItem("Seleccionar puerto");

    // Obtener la lista de puertos seriales disponibles
    const QList<QSerialPortInfo> puertosDisponibles = QSerialPortInfo::availablePorts();

    // Verificar si se encontraron puertos seriales
    if (puertosDisponibles.isEmpty()) {
        qWarning() << "No se encontraron puertos seriales.";
    } else {
        // Agregar los puertos seriales al ComboBox con el nombre y la descripción
        for (const QSerialPortInfo &info : puertosDisponibles) {
            QString textoCombo = QString("%1 - %2")
            .arg(info.portName())
                .arg(info.description().isEmpty() ? "Sin descripción" : info.description());

            comboBox->addItem(textoCombo);  // Se agrega el texto con nombre y descripción al ComboBox
            qDebug() << "Puerto encontrado:" << info.portName();
            qDebug() << "Descripción:" << info.description();
        }
    }

    // Conectar el evento de selección del ComboBox
    QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboBox, checkBoxMarcaTiempo, textEditRespuesta]() {
        // Verificar si el usuario ha seleccionado algo distinto de "Seleccionar puerto"
        if (comboBox->currentIndex() == 0) {
            qDebug() << "Por favor selecciona un puerto.";
            return;
        }

        // Obtener el texto seleccionado del ComboBox (ejemplo: "COM3 - Silicon Labs CP210x USB to UART Bridge")
        QString seleccion = comboBox->currentText();

        // Separar el nombre del puerto de la descripción
        QStringList partes = seleccion.split(" - ");
        if (partes.size() < 1) {
            qWarning() << "Formato de selección no válido.";
            return;
        }

        QString nombrePuerto = partes[0];  // Esto obtiene "COM3"
        qDebug() << "Conectando al puerto:" << nombrePuerto;

        // Si hay una conexión activa en otro puerto, cerrarla primero
        if (serialActual != nullptr && serialActual->isOpen()) {
            qDebug() << "Desconectando del puerto" << serialActual->portName();
            serialActual->close();
            delete serialActual;
            serialActual = nullptr;
        }

        // Crear un nuevo objeto QSerialPort y configurar el puerto seleccionado
        serialActual = new QSerialPort();
        serialActual->setPortName(nombrePuerto);
        serialActual->setBaudRate(QSerialPort::Baud115200);  // Cambiar al baud rate que necesites
        serialActual->setDataBits(QSerialPort::Data8);
        serialActual->setParity(QSerialPort::NoParity);
        serialActual->setStopBits(QSerialPort::OneStop);
        serialActual->setFlowControl(QSerialPort::NoFlowControl);

        // Intentar abrir el puerto
        if (!serialActual->open(QIODevice::ReadWrite)) {
            qWarning() << "No se pudo abrir el puerto serial:" << serialActual->errorString();
            delete serialActual;
            serialActual = nullptr;
            return;
        }

        qDebug() << "Conexión establecida con éxito en el puerto" << nombrePuerto;

        // Conectar la señal readyRead para recibir una línea completa de datos y mostrarla en el QTextEdit
        QObject::connect(serialActual, &QSerialPort::readyRead, this, [this, checkBoxMarcaTiempo, textEditRespuesta]() {
            while (serialActual->canReadLine()) {
                // Leer una línea completa desde el puerto serial
                QByteArray datosRecibidos = serialActual->readLine().trimmed();  // trim para eliminar \n o \r\n
                QString textoRecibido = QString::fromUtf8(datosRecibidos);

                // Si el checkbox está seleccionado, agregar marca de tiempo
                if (checkBoxMarcaTiempo->isChecked()) {
                    QString marcaDeTiempo = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
                    textEditRespuesta->append(marcaDeTiempo + " - " + textoRecibido);
                } else {
                    textEditRespuesta->append(textoRecibido);
                }

                qDebug() << "Datos recibidos:" << textoRecibido;
            }
        });
    });

    // Conectar el botón para enviar el texto del QLineEdit a través del puerto serial
    QObject::connect(botonEnviar, &QPushButton::clicked, this, [this, lineEdit]() {
        // Verificar que el puerto serial esté abierto
        if (serialActual == nullptr || !serialActual->isOpen()) {
            qWarning() << "El puerto serial no está abierto.";
            return;
        }

        // Obtener el texto del QLineEdit y agregar un salto de línea
        QByteArray datos = (lineEdit->text() + "\n").toUtf8();
        if (datos.isEmpty()) {
            qWarning() << "El texto para enviar está vacío.";
            return;
        }

        // Enviar los datos al puerto serial
        serialActual->write(datos);

        qDebug() << "Texto enviado:" << lineEdit->text();
    });

    // Conectar el botón para limpiar el QTextEdit
    QObject::connect(botonLimpiar, &QPushButton::clicked, this, [textEditRespuesta]() {
        textEditRespuesta->clear();  // Limpiar el QTextEdit
        qDebug() << "Pantalla limpiada.";
    });
}

