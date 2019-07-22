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

    connect(ui->btnRun, &QPushButton::clicked, this, &MainWindow::btnRunClicked);
    connect(ui->btnReady, &QPushButton::clicked, this, &MainWindow::btnReadyClicked);
    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::btnStartClicked);

    connect(ui->cbCMode, &QCheckBox::stateChanged, this, &MainWindow::cbCModeChanged);
    connect(ui->cbJMode, &QCheckBox::stateChanged, this, &MainWindow::cbJModeChanged);
    connect(ui->cbRMode, &QCheckBox::stateChanged, this, &MainWindow::cbRModeChanged);
    ui->btnRun->setDisabled(true);
    ui->btnReady->setDisabled(true);
    ui->cbJMode->setChecked(true);

    connectState = false;

    connect(ui->btnInit1, &QPushButton::clicked, this, &MainWindow::btnInitClicked);
    connect(ui->btnInit2, &QPushButton::clicked, this, &MainWindow::btnInitClicked);
    connect(ui->btnInit3, &QPushButton::clicked, this, &MainWindow::btnInitClicked);
    connect(ui->btnInit4, &QPushButton::clicked, this, &MainWindow::btnInitClicked);
    connect(ui->btnInit5, &QPushButton::clicked, this, &MainWindow::btnInitClicked);
    connect(ui->btnInit6, &QPushButton::clicked, this, &MainWindow::btnInitClicked);

    connect(ui->btnDeinit1, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);
    connect(ui->btnDeinit2, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);
    connect(ui->btnDeinit3, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);
    connect(ui->btnDeinit4, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);
    connect(ui->btnDeinit5, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);
    connect(ui->btnDeinit6, &QPushButton::clicked, this, &MainWindow::btnDeinitClicked);

    cbTorque.push_back(ui->cbTorque1);
    cbTorque.push_back(ui->cbTorque2);
    cbTorque.push_back(ui->cbTorque3);
    cbTorque.push_back(ui->cbTorque4);
    cbTorque.push_back(ui->cbTorque5);
    cbTorque.push_back(ui->cbTorque6);

    txtPosition.push_back(ui->txtJoint1);
    txtPosition.push_back(ui->txtJoint2);
    txtPosition.push_back(ui->txtJoint3);
    txtPosition.push_back(ui->txtJoint4);
    txtPosition.push_back(ui->txtJoint5);
    txtPosition.push_back(ui->txtJoint6);

    QDateTime *date = new QDateTime();
    _mkdir("../logging");
    QString fileName = "../logging/" + date->currentDateTime().toString("yyyy-MM-dd-hh-mm-ss") + ".csv";
    logger = new Logger(this, fileName);


    qDebug() << "Logging Start";
    QString head = "Time,Time Step,IK Time Step,DXL Time Step";
    head += "CurJoint1,CurJoint2,CurJoint3,CurJoint4,CurJoint5,CurJoint6,";
    head += "CurCartX,CurCatY,CurCartZ,CurCartRoll,CurCartPitch,CurCartYaw,";
    head += "DesJoint1,DesJoint2,DesJoint3,DesJoint4,DesJoint5,DesJoint6,";
    head += "DesCartX,DesCartY,DesCartZ,DesCartRoll,DesCartPitch,DesCartYaw\n";
    logger->write(head);
}

MainWindow::~MainWindow()
{
    WriteSettings();
    curJoint.clear();
    curCart.clear();
    desJoint.clear();
    desCart.clear();
    cbTorque.clear();
    delete tcpClient;
    delete ui;
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
        connectState = false;

        ui->btnStart->setText("Start");
        ui->btnRun->setDisabled(true);
        ui->btnReady->setDisabled(true);
    }
}

void MainWindow::onConnectServer(){
    qDebug() << "Connect complete";

    connectState = true;

    for(int i = 0; i < 6; i++){
        txData.append(QByteArray::number(0.0, 'f', 7));
        txData.append(",");
    }
    for(int i = 0; i < 6; i++){
        txData.append(QByteArray::number(0.0, 'f', 7));
        txData.append(",");
    }
    mClientToServer.mode = 10;
    txData.append(QByteArray::number(mClientToServer.mode));
    txData.append(",\n");
    tcpClient->socket->write(txData);
    txData.clear();
}

void MainWindow::readMessage(){
    ui->rbComState->toggle();

    rxData = tcpClient->socket->readAll();
//    qDebug() << rxData;

    char ch[2];
    for (int j = 0; j < 2; j++) {
        ch[j] = rxData.at(j);
    }

    if (ch[0] == 0x0A && ch[1] == 0x03){
        int id = rxData.at(2);
        cbTorque[id - 1]->setChecked(rxData.at(3));

        txtPosition[id - 1]->setText(rxData.mid(4, 4));
    }
    else if(ch[0] == 0x0A && ch[1] == 0x04){
        int id = rxData.at(2);
        cbTorque[id - 1]->setChecked(rxData.at(3));
    }
    else{
        QList<QByteArray> Data = rxData.split('\n');
//        qDebug() << "Data Length : " + QString::number(Data.length());
        if (Data.length() > 0){

            logger->write(rxData);

            QList<QByteArray> Data2 = Data.at(0).split(',');
//            qDebug() << "Data2 Length : " + QString::number(Data2.length());
            if (Data2.length() > 25){
                mServerToClient.t = Data2.at(0).toDouble();
                mServerToClient.step = Data2.at(1).toULongLong();
                mServerToClient.ik_step = Data2.at(2).toULongLong();
                mServerToClient.dxl_step = Data2.at(3).toULongLong();
                for(int i = 4; i <= 9; i++){
                    mServerToClient.curJoint[i - 4] = Data2.at(i).toDouble();
                    mServerToClient.curCart[i - 4] = Data2.at(i + 6).toDouble();
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
                //*/

                for(int i = 0; i < 6; i++){
                    curJoint[i]->setValue(mServerToClient.curJoint[i]*RAD2DEG);
                    curCart[i]->setValue(mServerToClient.curCart[i]*(i < 3 ? 1000 : RAD2DEG));
                }
                ui->txtStepTime->setText(QString::number(mServerToClient.step/1000000.0, 'f', 7));
                ui->txtIKStepTime->setText(QString::number(mServerToClient.ik_step/1000000.0, 'f', 7));
                ui->txtDXLStepTime->setText(QString::number(mServerToClient.dxl_step/1000000.0, 'f', 7));
            }
        }
    }
}

void MainWindow::btnRunClicked(){
    if (ui->cbJMode->isChecked()){
        for(int i = 0; i < 6; i++){
            mClientToServer.desCart[i] = 0;
            mClientToServer.desJoint[i] = desJoint[i]->value();
        }
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
        for(int i = 0; i < 6; i++){
            mClientToServer.desCart[i] = desCart[i]->value();
            mClientToServer.desJoint[i] = 0;
        }
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
        mClientToServer.mode = moveRect;
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        for(int i = 0; i < 6; i++){
            txData.append(QByteArray::number(0.0, 'f', 7));
            txData.append(",");
        }
        txData.append(QByteArray::number(mClientToServer.mode));
        txData.append(",\n");
    }
    if (connectState){
        tcpClient->socket->write(txData);
    }
    txData.clear();

//    for(int i = 0; i < 6; i++){
//        desJoint[i]->setValue(0);
//        desCart[i]->setValue(0);
//    }
}

void MainWindow::btnStartClicked(){
    if (connectState) {
        if (!ui->btnStart->text().compare("Start")){
            for(int i = 0; i < 6; i++){
                txData.append(QByteArray::number(0.0, 'f', 7));
                txData.append(",");
            }
            for(int i = 0; i < 6; i++){
                txData.append(QByteArray::number(0.0, 'f', 7));
                txData.append(",");
            }
            mClientToServer.mode = moveStart;
            txData.append(QByteArray::number(mClientToServer.mode));
            txData.append(",\n");

            tcpClient->socket->write(txData);
            txData.clear();

            ui->btnReady->setEnabled(true);
            ui->btnRun->setEnabled(true);

            ui->btnStart->setText("Stop");
        }
        else{
            for(int i = 0; i < 6; i++){
                txData.append(QByteArray::number(0.0, 'f', 7));
                txData.append(",");
            }
            for(int i = 0; i < 6; i++){
                txData.append(QByteArray::number(0.0, 'f', 7));
                txData.append(",");
            }
            mClientToServer.mode = moveStop;
            txData.append(QByteArray::number(mClientToServer.mode));
            txData.append(",\n");

            tcpClient->socket->write(txData);
            txData.clear();

            ui->btnReady->setDisabled(true);
            ui->btnRun->setDisabled(true);

            ui->btnStart->setText("Start");
        }
    }
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

        ui->btnReady->setEnabled(true);
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

void MainWindow::btnInitClicked(){
    int id = sender()->objectName().split("btnInit").at(1).toInt();
    if (connectState){
        txData.append(QByteArray::fromRawData("\x02\x03", 2));
        txData.append(static_cast<char>(id));
        txData.append(QByteArray::fromRawData("\x0A\x03", 2));
        tcpClient->socket->write(txData);
        txData.clear();
    }
}

void MainWindow::btnDeinitClicked(){
    int id = sender()->objectName().split("btnDeinit").at(1).toInt();
    if (connectState){
        txData.append(QByteArray::fromRawData("\x02\x04", 2));
        txData.append(static_cast<char>(id));
        txData.append(QByteArray::fromRawData("\x0A\x04", 2));
        tcpClient->socket->write(txData);
        txData.clear();
    }
}
