#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QTime>
#include <QDateTime>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QChart>
#include <QToolTip>
#include <QCursor>
#include <QFile>


//using namespace QtCharts;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateChart(float newTemp);
    float parseTemperature(const QString &data);

private slots:
    void readSerialData();
    void connexion_uart();
    //void setupWiFiConnection();
    void readWiFiData();
    void handleNewConnection();
    void setupWiFiServer();
    //void cleanupWifiServer();
    void updateDateTime();
    void setupChart();
    void onPointHovered(const QPointF &point, bool state);
    void loadDataFromFile();
    void saveDataToFile(float value);

private:
    QSerialPort *serial;
    Ui::MainWindow *ui;
    bool isConnected = false;
    bool isConnectedW = false;
    QTcpSocket *tcpSocket = nullptr;
    QTcpServer *tcpServer = nullptr;
    QTcpSocket *clientSocket = nullptr;
    QTimer *connectionTimer = nullptr;

    QLineSeries *series;
    QChart *chart;
    QValueAxis *axisX;
    QValueAxis *axisY;
    QVector<QPointF> tempData;  // Store temperature points (timestamp, value)
    QTimer *chartTimer;         // Optional: if using fixed update intervals

};


#endif // MAINWINDOW_H
