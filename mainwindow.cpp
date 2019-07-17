#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ReadSettings();

    QObject::connect(ui->btnConnect, &QPushButton::clicked, this, &MainWindow::btnConnectClicked);
    tcpClient = new TcpClient();

    connect(tcpClient->socket, &QTcpSocket::connected, this, &MainWindow::onConnectServer);
    connect(tcpClient->socket, &QTcpSocket::readyRead, this, &MainWindow::readMessage);

    curJoint.push_back(ui->spJoint1);
    curJoint.push_back(ui->spJoint2);
    curJoint.push_back(ui->spJoint3);
    curJoint.push_back(ui->spJoint4);
    curJoint.push_back(ui->spJoint5);
    curJoint.push_back(ui->spJoint6);

    curCart.push_back(ui->spEndX);
    curCart.push_back(ui->spEndY);
    curCart.push_back(ui->spEndZ);
    curCart.push_back(ui->spEndRoll);
    curCart.push_back(ui->spEndPitch);
    curCart.push_back(ui->spEndYaw);

    desJoint.push_back(ui->spJoint1Cmd);
    desJoint.push_back(ui->spJoint2Cmd);
    desJoint.push_back(ui->spJoint3Cmd);
    desJoint.push_back(ui->spJoint4Cmd);
    desJoint.push_back(ui->spJoint5Cmd);
    desJoint.push_back(ui->spJoint6Cmd);

    desCart.push_back(ui->spEndXCmd);
    desCart.push_back(ui->spEndYCmd);
    desCart.push_back(ui->spEndZCmd);
    desCart.push_back(ui->spEndRollCmd);
    desCart.push_back(ui->spEndPitchCmd);
    desCart.push_back(ui->spEndYawCmd);

    for(int i = 0; i < 6; i++){
        curJoint[i]->setRange(-9999, 9999);
        curCart[i]->setRange(-9999, 9999);
        desJoint[i]->setRange(-9999, 9999);
        desCart[i]->setRange(-9999, 9999);
    }

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::timeout);
    timer->setInterval(100);

    connect(ui->btnRun, &QPushButton::clicked, this, &MainWindow::btnRunClicked);
    connect(ui->btnReady, &QPushButton::clicked, this, &MainWindow::btnReadyClicked);

    connect(ui->cbCMode, &QCheckBox::stateChanged, this, &MainWindow::cbCModeChanged);
    connect(ui->cbJMode, &QCheckBox::stateChanged, this, &MainWindow::cbJModeChanged);
    connect(ui->cbRMode, &QCheckBox::stateChanged, this, &MainWindow::cbRModeChanged);
    ui->btnRun->setDisabled(true);
    ui->cbJMode->setChecked(true);

    connectState = false;
}

MainWindow::~MainWindow()
{
    WriteSettings();
    curJoint.clear();
    curCart.clear();
    delete ui;
    delete tcpClient;
    delete logger;
}

void MainWindow::ReadSettings()
{
    QSettings settings("RobotControllerUI", "EAR");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    QString Address = settings.value("Address").toString();
    QString Port = settings.value("Port").toString();
    ui->txtAddress->setText(Address);
    ui->txtPort->setText(Port);
}

void MainWindow::WriteSettings()
{
    QSettings settings("RobotControllerUI", "EAR");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("Address",ui->txtAddress->text());
    settings.setValue("Port",ui->txtPort->text());
}

void MainWindow::btnConnectClicked(){
    if (!ui->btnConnect->text().compare("Connect")){

        tcpClient->setIpAddress(ui->txtAddress->text());
        tcpClient->setPort(ui->txtPort->text().toUShort());
        emit tcpClient->connectToServer();

        ui->btnConnect->setText("Disconnect");
    }
    else{
        tcpClient->socket->close();
        ui->btnConnect->setText("Connect");
        logger->file->close();
        qDebug() << "Logging Stop";
        timer->stop();
        connectState = false;
        ui->btnRun->setDisabled(true);
    }
}

void MainWindow::onConnectServer(){
    qDebug() << "Connect complete";

    QDateTime *date = new QDateTime();
    _mkdir("../logging");
    QString fileName = "../logging/" + date->currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + ".csv";
    logger = new Logger(this, fileName);

    qDebug() << "Logging Start";
    QString head = "Time,Time Step,IK Time Step,";
    head += "CurJoint1,CurJoint2,CurJoint3,CurJoint4,CurJoint5,CurJoint6,";
    head += "CurCartX,CurCatY,CurCartZ,CurCartRoll,CurCartPitch,CurCartYaw,";
    head += "DesJoint1,DesJoint2,DesJoint3,DesJoint4,DesJoint5,DesJoint6,";
    head += "DesCartX,DesCartY,DesCartZ,DesCartRoll,DesCartPitch,DesCartYaw\n";
    logger->write(head);
    timer->start();

    connectState = true;
}

void MainWindow::readMessage(){
    rxData = tcpClient->socket->readAll();
    logger->write(rxData);

    QList<QByteArray> Data = rxData.split('\n');
//    qDebug() << Data.length();
    if (Data.length() >= 1){
        QList<QByteArray> Data2 = Data.at(0).split(',');
//        qDebug() << Data2.length();
        if (Data2.length() > 10){
            mServerToClient.t = Data2.at(0).toDouble();
            mServerToClient.step = Data2.at(1).toULongLong();
            mServerToClient.ik_step = Data2.at(2).toULongLong();
            for(int i = 3; i <= 8; i++){
                mServerToClient.curJoint[i - 3] = Data2.at(i).toDouble();
                mServerToClient.curCart[i - 3] = Data2.at(i + 6).toDouble();
            }
    /**
            printf("Time : %f\t", Data2.at(0).toDouble());
            printf("step size : %ld.%06ld ms\t",
                   static_cast<unsigned long>(Data2.at(1).toULongLong()) / 1000000,
                   static_cast<unsigned long>(Data2.at(1).toULongLong()) % 1000000);
            printf("ik step size : %ld.%06ld ms\t",
                   static_cast<unsigned long>(Data2.at(2).toULongLong()) / 1000000,
                   static_cast<unsigned long>(Data2.at(2).toULongLong()) % 1000000);
            for(int i = 3; i <= 8; i++){
                printf("%f, ", Data2.at(i).toDouble());
            }
            printf("\t");
            for(int i = 9; i <= 14; i++){
                printf("%f, ", Data2.at(i).toDouble());
            }
            printf("\n");
    /**/
        }
    }
}

void MainWindow::timeout(){
    for(int i = 0; i < 6; i++){
        curJoint[i]->setValue(mServerToClient.curJoint[i]*RAD2DEG);
        curCart[i]->setValue(mServerToClient.curCart[i]*(i < 3 ? 1000 : RAD2DEG));
    }
    ui->txtStepTime->setText(QString::number(mServerToClient.step/1000000.0, 'f', 7));
    ui->txtIKStepTime->setText(QString::number(mServerToClient.ik_step/1000000.0, 'f', 7));

    ui->rbComState->toggle();
}

void MainWindow::btnRunClicked(){
    for(int i = 0; i < 6; i++){
        mClientToServer.desCart[i] = desCart[i]->value()*(i < 3 ? 0.001 : DEG2RAD);
        mClientToServer.desJoint[i] = desJoint[i]->value()*DEG2RAD;
    }
    if (ui->cbJMode->isChecked()){
        mClientToServer.mode = moveJoint;
        for(double q : mClientToServer.desJoint){
            txData.append(QByteArray::number(q, 'f', 7));
            txData.append(",");
        }
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        txData.append(QByteArray::number(mClientToServer.mode));
        txData.append(",\n");
    }
    if (ui->cbCMode->isChecked()){
        mClientToServer.mode = moveCart;
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        for(double pos : mClientToServer.desCart){
            txData.append(QByteArray::number(pos, 'f', 7));
            txData.append(",");
        }
        txData.append(QByteArray::number(mClientToServer.mode));
        txData.append(",\n");
    }
    if (ui->cbRMode->isChecked()){
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        mClientToServer.mode = moveRect;
        txData.append(QByteArray::number(mClientToServer.mode));
        txData.append(",\n");
    }
    tcpClient->socket->write(txData);
    txData.clear();
}

void MainWindow::btnReadyClicked(){
    if (connectState) {
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        mClientToServer.mode = moveReady;
        txData.append(QByteArray::number(mClientToServer.mode));
        txData.append(",\n");
        tcpClient->socket->write(txData);
        txData.clear();

        ui->btnRun->setEnabled(true);
    }
}

void MainWindow::cbCModeChanged(int arg){
    if (arg != 0){
        ui->cbJMode->setChecked(false);
        ui->cbRMode->setChecked(false);
    }
}

void MainWindow::cbJModeChanged(int arg){
    if (arg != 0){
        ui->cbCMode->setChecked(false);
        ui->cbRMode->setChecked(false);
    }
}

void MainWindow::cbRModeChanged(int arg){
    if (arg != 0){
        ui->cbJMode->setChecked(false);
        ui->cbCMode->setChecked(false);
    }
}
