#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QtDebug>
#include <QTimer>
#include <QDoubleSpinBox>
#include <QtMath>
#include <QCheckBox>
#include <QLabel>

#include "Logger/logger.h"
#include "TcpSocket/tcpclient.h"

struct ServerToClient{
    unsigned long long step, ik_step, dxl_step;
    double curJoint[6], curCart[6];
    double t;
};

enum{moveStart = 0, moveReady, moveJoint, moveCart, moveRect, moveStop};
struct ClientToServer{
    double desCart[6], desJoint[6];
    uint mode; // 0 : Start, 1 : Ready, 2 : joint, 3 : Cartesian, 4 : Rect
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
    void btnRunClicked();
    void btnReadyClicked();
    void cbCModeChanged(int arg);
    void cbJModeChanged(int arg);
    void cbRModeChanged(int arg);
    void btnInitClicked();
    void btnDeinitClicked();
    void btnStartClicked();

private:
    Ui::MainWindow *ui;
    TcpClient *tcpClient;
    Logger *logger;
    QByteArray rxData, txData;
    QString txtMessage;
    ServerToClient mServerToClient;
    ClientToServer mClientToServer;
    QVector<QDoubleSpinBox*> curJoint, curCart, desJoint, desCart;
    bool CMode, JMode, RMode;
    bool connectState;
    QVector<QCheckBox*> cbTorque;
    QVector<QLabel*> txtPosition;
};

const double DEG2RAD = M_PI/180.0;
const double RAD2DEG = 180.0/M_PI;

#endif // MAINWINDOW_H
