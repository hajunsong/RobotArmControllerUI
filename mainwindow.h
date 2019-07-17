#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QDoubleSpinBox>
#include <QtMath>

#include "Logger/logger.h"
#include "TcpSocket/tcpclient.h"

struct ServerToClient{
    unsigned long long step, ik_step;
    double curJoint[6], curCart[6];
    double t;
};

enum{moveReady=0, moveJoint, moveCart, moveRect};
struct ClientToServer{
    double desCart[6], desJoint[6];
    uint mode; // 1 : joint, 2 : Cartesian, 3 : Rect
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void ReadSettings();
    void WriteSettings();

public slots:
    void btnConnectClicked();
    void onConnectServer();
    void readMessage();
    void timeout();
    void btnRunClicked();
    void btnReadyClicked();
    void cbCModeChanged(int arg);
    void cbJModeChanged(int arg);
    void cbRModeChanged(int arg);

private:
    Ui::MainWindow *ui;
    TcpClient *tcpClient;
    Logger *logger;
    QByteArray rxData, txData;
    QString txtMessage;
    ServerToClient mServerToClient;
    ClientToServer mClientToServer;
    QVector<QDoubleSpinBox*> curJoint, curCart, desJoint, desCart;
    QTimer *timer;
    bool CMode, JMode, RMode;
    bool connectState;
};

const double DEG2RAD = M_PI/180.0;
const double RAD2DEG = 180.0/M_PI;

#endif // MAINWINDOW_H
