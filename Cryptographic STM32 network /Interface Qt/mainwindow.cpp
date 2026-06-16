#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QToolTip>
#include <QCursor>
#include <QFile>


#include <QTcpServer>
#include <QTcpSocket>

#include <QTimer>
#include <QDateTime>
#include <QTime>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->receiveButton, &QPushButton::clicked, this, &MainWindow::connexion_uart);
    connect(ui->wifiButton, &QPushButton::clicked, this, &MainWindow::setupWiFiServer);

    QTimer *timer = new QTimer(this); // Timer to update every second
    connect(timer, &QTimer::timeout, this, &MainWindow::updateDateTime);
    timer->start(1000);
    updateDateTime();  // Initial update

    setupChart();
    loadDataFromFile();
    //setupWiFiConnection();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainWindow::~MainWindow()
{
    delete ui;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::connexion_uart()
{
    if(!isConnected){
    serial = new QSerialPort(this);
    serial->setPortName("COM14"); // Change to your FTDI COM port
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    if (serial->open(QIODevice::ReadWrite))
    {
        isConnected = true;
        ui->receiveButton->setText("Stop Receiving");
        ui->statusLabel->setText("The object is CONNECTED to the UI");
        qDebug() << "Serial port opened.";
    } else
    {
        serial->write("0"); // tell STM32 to turn LED OFF
        QMessageBox::critical(this, "Connection Error", "Failed to open serial port:\n" + serial->errorString());
        serial->deleteLater();
        serial = nullptr;
        return;  // Abort further setup
    }
    } else
    {
        serial->write("0"); // tell STM32 to turn LED OFF
        isConnected = false;
        ui->receiveButton->setText("Start Receiving");
        ui->statusLabel->setText("The object is DISCONNECTED to the UI");
        serial->close();
        serial->deleteLater();
        qDebug() << "Serial port closed.";
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindow::readSerialData() {
    static QByteArray buffer;

    buffer.append(serial->readAll());

    /*if (buffer.contains('\n')) {
        QString temp = QString(buffer).trimmed();
        ui->temperatureLabel->setText(temp);
        buffer.clear();
    }*/
    // Wait until 60 bytes are accumulated
    if (buffer.size() < 60)
    { serial->write("0"); // tell STM32 to turn LED OFF when the serial port is not receiving a real value
        return;}

    // Extract the plaintext value from buffer
    // Example: the plaintext is in bytes 0 to 6 (first 7 bytes)
    QByteArray plaintext = buffer.mid(0, 6);  // adjust based on where your value is in a specific placement in the buffer

    // this prevente the ui to display any inreadable char like the space when the temp<10
    QString temp;
    for (char c : plaintext) {
        if ((c >= '0' && c <= '9') || c == '.' || c == '-')  // Printable ASCII
            temp.append(c);
    }

    // Display the value directly (without "temperature value:")
    serial->write("1");
    ui->temperatureLabel->setText(temp + " °C"); // degree symbol
    float receivedTemp = parseTemperature(temp);
    updateChart(receivedTemp);

    //qDebug() << "Raw temp string:" << temp;
    //qDebug() << "Parsed float:" << receivedTemp;
    buffer.clear(); // Clear for next frame
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setupWiFiServer()
{
    if (!isConnectedW)
    {
        if (tcpServer != nullptr) {
            qDebug() << "Server already running.";
            return;
        }

        tcpServer = new QTcpServer(this);
        connect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::handleNewConnection);

        if (!tcpServer->listen(QHostAddress::Any, 80)) {
            QMessageBox::critical(this, "WiFi Error", "Failed to start server:\n" + tcpServer->errorString());
            tcpServer->deleteLater();
            tcpServer = nullptr;
            return;
        }

        isConnectedW = true;
        ui->wifiButton->setText("Stop Receiving");
        ui->statusLabelWifi->setText("WiFi server listening");
        qDebug() << "Server listening on port 80";
    }
    else
    {
        isConnectedW = false;
        ui->wifiButton->setText("Start Receiving");
        ui->statusLabelWifi->setText("WiFi server stopped");
        qDebug() << "i'm here 1";

        if (tcpSocket) {
            tcpSocket->disconnectFromHost();
            tcpSocket = nullptr;
            qDebug() << "i'm here 2";
        }

        if (tcpServer) {
            disconnect(tcpServer, &QTcpServer::newConnection, this, &MainWindow::handleNewConnection);
            tcpServer->close();
            tcpServer->deleteLater();
            tcpServer = nullptr;
            qDebug() << "i'm here 3";
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::handleNewConnection()
{
    if (!tcpServer) return;

    tcpSocket = tcpServer->nextPendingConnection();
    if (!tcpSocket) return;

    isConnectedW = true;
    qDebug() << "STM32 connected.";
    ui->statusLabelWifi->setText("WiFi client connected");

    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::readWiFiData);

    connect(tcpSocket, &QTcpSocket::disconnected, this, [this]() {
        ui->statusLabelWifi->setText("WiFi client disconnected");
        tcpSocket->deleteLater();
        tcpSocket = nullptr;
    });
}


/*void MainWindow::handleNewConnection() {
    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::readWiFiData);
    qDebug() << "Client connected from:" << clientSocket->peerAddress().toString();
}*/
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::readWiFiData()
{
    //QByteArray data = tcpSocket->readAll();
    QByteArray data = tcpSocket->readAll();
    qDebug() << "Received data:" << data;

    QString temp;
    for (char c : data) {
        if (c >= 32 && c <= 126)
            temp.append(c);
    }
    ui->temperatureWifiLabel->setText(temp + " °C");
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::updateDateTime()
{
    QDateTime current = QDateTime::currentDateTime();
    QString formatted = current.toString("dddd, dd MMMM yyyy - HH:mm:ss");

    // Example: Monday, 28 July 2025 - 14:41:03
    ui->dateTimeLabel->setText(formatted);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::setupChart()
{
    series = new QLineSeries();

    chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Temperature over last 12 hours");
    chart->legend()->hide();

    axisX = new QValueAxis();
    axisX->setTitleText("Time (h)");
    axisX->setRange(0, 12);  // 0 to 12 hours
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY = new QValueAxis();
    axisY->setTitleText("Temperature (°C)");
    axisY->setRange(0, 100);  // Adjust if needed
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);


    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui->verticalLayout_5->addWidget(chartView);

    series->setName("Temperature");
    series->setPointsVisible(true);
    chart->addSeries(series);
    connect(series, &QLineSeries::hovered,this, &MainWindow::onPointHovered);
}

////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::updateChart(float newTemp)
{
    // Get current time in hours since app start
    static QElapsedTimer timer;
    if (!timer.isValid())
        timer.start();

    float hoursElapsed = timer.elapsed() / (1000.0 * 60.0 * 60.0);  // ms → h

    // Add new point
    tempData.append(QPointF(hoursElapsed, newTemp));
    saveDataToFile(newTemp);

    // Keep only points within last 12 hours
    while (!tempData.isEmpty() && (hoursElapsed - tempData.first().x() > 12.0)) {
        tempData.removeFirst();
    }

    series->replace(tempData);
}
////////////////////////////////////////////////////////////////////////////////////////////////////

float MainWindow::parseTemperature(const QString &data)
{
    bool ok;
    float temp = data.trimmed().toFloat(&ok);
    if (ok)
        return temp;
    else
        return 0.0;  // or handle error
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::onPointHovered(const QPointF &point, bool state)
{
    static QPointF lastPoint;
    static bool lastState = false;

    if (state && (!lastState || point != lastPoint)) {
        lastPoint = point;
        lastState = true;

        QString valueText = QString("Temperature: %1 °C").arg(point.y(), 0, 'f', 2);
        QPoint cursorPos = QCursor::pos();
        QToolTip::showText(cursorPos, valueText);
    } else if (!state) {
        lastState = false;
        QToolTip::hideText();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::saveDataToFile(float value) {
    QFile file("data.csv");
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        qint64 msSinceEpoch = QDateTime::currentDateTime().toMSecsSinceEpoch();
        out << msSinceEpoch << "," << value << "\n";
        file.close();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////

void MainWindow::loadDataFromFile() {
    QFile file("data.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        int x = 0;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() == 2) {
                bool ok;
                float value = parts[1].toFloat(&ok);
                if (ok) {
                    series->append(x++, value);
                }
            }
        }
        file.close();
    }
}
/*void MainWindow::loadDataFromFile() {
    QFile file("data.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        tempData.clear();

        qint64 firstTimestamp = -1;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(",");
            if (parts.size() == 2) {
                bool ok;
                qint64 timestamp = parts[0].toLongLong(&ok);
                float value = parts[1].toFloat(&ok);
                if (ok) {
                    if (firstTimestamp < 0)
                        firstTimestamp = timestamp;

                    double hours = (timestamp - firstTimestamp) / (1000.0 * 60.0 * 60.0);
                    tempData.append(QPointF(hours, value));
                }
            }
        }
        file.close();
        series->replace(tempData);
    }
}*/

