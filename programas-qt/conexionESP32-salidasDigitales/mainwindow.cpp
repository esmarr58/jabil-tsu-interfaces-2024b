#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    manejarPuertosSeriales(ui->comboBox);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::enviarDatosSerial(int numeroPin, int estado)
{
    // Verificar que el puerto serial esté abierto
    if (serialActual == nullptr || !serialActual->isOpen()) {
        QMessageBox::warning(this, "Error", "El puerto serial no está abierto.");
        return "Error,El puerto serial no está abierto.";
    }

    // Construir la cadena de texto en formato JSON
    QString jsonString = QString("{\"pin\":%1,\"estado\":%2}\n")
                             .arg(numeroPin)
                             .arg(estado);

    // Convertir la cadena a QByteArray para enviarla a través del puerto serial
    QByteArray datos = jsonString.toUtf8();

    // Enviar los datos al puerto serial
    qint64 bytesEscritos = serialActual->write(datos);

    // Verificar si los datos fueron escritos correctamente
    if (bytesEscritos == -1) {
        QMessageBox::critical(this, "Error", "No se pudieron enviar los datos: " + serialActual->errorString());
    } else {
        qDebug() << "Datos enviados:" << jsonString;
        //QMessageBox::information(this, "Éxito", "Datos enviados exitosamente.");
    }
    return jsonString;
}


void MainWindow::manejarPuertosSeriales(QComboBox *comboBox)
{
    // Limpiar cualquier elemento previo en el ComboBox
    comboBox->clear();

    // Añadir la opción por defecto "Seleccionar puerto"
    comboBox->addItem("Seleccionar puerto");

    // Obtener la lista de puertos seriales disponibles
    const QList<QSerialPortInfo> puertosDisponibles = QSerialPortInfo::availablePorts();

    // Verificar si se encontraron puertos seriales
    if (puertosDisponibles.isEmpty()) {
        QMessageBox::critical(this, "Error", "No se encontraron puertos seriales.");
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

    // Conectar el evento de selección del ComboBox para manejar la conexión
    QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
        // Verificar si el usuario ha seleccionado algo distinto de "Seleccionar puerto"
        if (comboBox->currentIndex() == 0) {
            QMessageBox::warning(this, "Advertencia", "Por favor selecciona un puerto.");
            return;
        }

        // Obtener el texto seleccionado del ComboBox (ejemplo: "COM3 - Silicon Labs CP210x USB to UART Bridge")
        QString seleccion = comboBox->currentText();

        // Separar el nombre del puerto de la descripción
        QStringList partes = seleccion.split(" - ");
        if (partes.size() < 1) {
            QMessageBox::critical(this, "Error", "Formato de selección no válido.");
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
            QMessageBox::critical(this, "Error", "No se pudo abrir el puerto serial: " + serialActual->errorString());
            delete serialActual;
            serialActual = nullptr;
            return;
        }

        QMessageBox::information(this, "Conexión exitosa", "Conexión establecida con éxito en el puerto " + nombrePuerto);
        qDebug() << "Conexión establecida con éxito en el puerto" << nombrePuerto;
        QObject::connect(serialActual, &QSerialPort::readyRead, this, [this]() {
            // Llamar a leerRespuestaSerial y pasar el QTextEdit de la UI como parámetro
            leerRespuestaSerial(ui->textEdit);
        });
    });
}

void MainWindow::on_pushButton_clicked()
{
    bool okPin, okEstado;
    int pin = ui->lineEdit->text().toInt(&okPin);  // Convertir el texto del pin a entero
    int estado = ui->lineEdit_2->text().toInt(&okEstado);  // Convertir el texto del estado a entero

    // Validar que las conversiones a número fueron exitosas
    if (!okPin || !okEstado) {
        QMessageBox::warning(this, "Error", "Por favor, ingresa valores válidos para el pin y el estado.");
        return;
    }

    // Llamar a la función enviarDatosSerial con los valores del pin y estado
    QString mensajeFormado = enviarDatosSerial(pin, estado);
    ui->label_3->setText(mensajeFormado);


    // Obtener la fecha y hora actuales para el log y el nombre del archivo
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString timestamp = currentDateTime.toString("yyyy-MM-dd hh:mm:ss");

    // Usar la fecha actual para nombrar el archivo log
    QString fileName = currentDateTime.toString("yyyy-MM-dd") + ".csv";


    // Guardar en un archivo CSV (ejemplo: 2024-10-23.csv) el timestamp y el mensaje JSON
    QFile file(fileName);


    // Verificar si el archivo ya existe
    bool fileExists = file.exists();

    if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        if (!fileExists) {
            out << "Fecha,Mensaje JSON\n";  // Agregar las cabeceras
        }


        out << timestamp << "," << mensajeFormado.trimmed() << "\n";
        file.close();
    } else {
        QMessageBox::warning(this, "Error", "No se pudo abrir el archivo de log para escribir.");
    }

    // Llamar a la función que limpia logs más antiguos de 1 semana
    limpiarLogsAntiguos();

    qDebug() << "Log guardado en CSV con timestamp:" << timestamp;


}

void MainWindow::leerRespuestaSerial(QTextEdit *textEditRespuesta)
{
    // Verificar que el puerto serial esté abierto
    if (serialActual == nullptr || !serialActual->isOpen()) {
        QMessageBox::warning(this, "Error", "El puerto serial no está abierto.");
        return;
    }

    // Mientras haya una línea completa disponible en el puerto serial
    while (serialActual->canReadLine()) {
        // Leer la línea completa del puerto serial (incluye hasta el carácter '\n')
        QByteArray datosRecibidos = serialActual->readLine().trimmed();  // `trimmed` elimina los saltos de línea al final

        // Convertir los datos a texto usando UTF-8
        QString textoRecibido = QString::fromUtf8(datosRecibidos);

        // Mostrar la línea completa recibida en el QTextEdit
        textEditRespuesta->append(textoRecibido);

        // Depuración en consola
        qDebug() << "Línea completa recibida:" << textoRecibido;
    }
}

void MainWindow::limpiarLogsAntiguos()
{
    // Directorio actual donde se guardan los logs
    QDir dir = QDir::current();

    // Obtener la lista de archivos en el directorio
    QStringList fileList = dir.entryList(QStringList() << "*.csv", QDir::Files);

    // Fecha límite de 1 semana atrás
    QDateTime limite = QDateTime::currentDateTime().addDays(-7);

    for (const QString &fileName : fileList) {
        QFileInfo fileInfo(dir.filePath(fileName));

        // Verificar si el archivo es más antiguo que la fecha límite
        if (fileInfo.lastModified() < limite) {
            // Borrar el archivo si es más antiguo de 1 semana
            QFile::remove(fileInfo.filePath());
            qDebug() << "Log eliminado:" << fileInfo.fileName();
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->textEdit->clear();
}


void MainWindow::on_pushButton_3_clicked()
{
    // Obtener la fecha actual para construir el nombre del archivo de log
    QString fileName = QDateTime::currentDateTime().toString("yyyy-MM-dd") + ".csv";

    // Verificar si el archivo de log existe
    if (QFile::exists(fileName)) {
        // Abrir el archivo usando el visor de texto predeterminado del sistema operativo
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    } else {
        // Mostrar un mensaje de advertencia si el archivo no existe
        QMessageBox::warning(this, "Error", "El archivo de log no existe: " + fileName);
    }
}

