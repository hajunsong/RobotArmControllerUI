#include "tcpserver.h"

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent) {
}

void TcpServer::startServer() {
    if (!this->listen(QHostAddress(ip), port))
    {
        qDebug() << "Could not start server";
    }
    else
    {
        qDebug() << "Listening ...";
    }
}

void TcpServer::setting(QString IP, quint16 PORT)
{
    ip = IP;
    port = PORT;
}

void TcpServer::incomingConnection(qintptr socketDescriptor) {
    // We have a new connection
    qDebug() << QString::number(socketDescriptor) + " Connecting...";

    socket = new QTcpSocket();

    // set the ID
    if (!socket->setSocketDescriptor(socketDescriptor))
    {
        // something's wrong, we just emit a signal
        emit error(socket->error());
        return;
    }

    // connect socket and signal
    // note - Qt::DirectConnection is used because it's multithreaded
    //        This makes the slot to be invoked immediately, when the signal is emitted.

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()), Qt::DirectConnection);
}	

void TcpServer::readyRead() {
    QByteArray rxData = socket->readAll();
//    qDebug() << "Receive Data : " + rxData;

    QList<QByteArray> Data = rxData.split(',');
//    qDebug() << rxData.length();
    if (rxData.length() > 2){
        printf("desCart : %f, %f, %f\t", Data.at(0).toDouble(), Data.at(1).toDouble(), Data.at(2).toDouble());
        printf("curJoint : %f, %f, %f\t", Data.at(3).toDouble(), Data.at(4).toDouble(), Data.at(5).toDouble());
        printf("curCart : %f, %f, %f\t", Data.at(6).toDouble(), Data.at(7).toDouble(), Data.at(8).toDouble());
        printf("step size : %ld.%06ld ms\t",
               static_cast<unsigned long>(Data.at(9).toULongLong()) / 10000,
               static_cast<unsigned long>(Data.at(9).toULongLong()) % 10000);
        printf("ik step size : %ld.%06ld ms\t",
               static_cast<unsigned long>(Data.at(10).toULongLong()) / 10000,
               static_cast<unsigned long>(Data.at(10).toULongLong()) % 10000);
        printf("Index : %d\n", Data.at(11).toUInt());
    }

    QByteArray txData;
    txData.append(QByteArray::fromRawData("\x02\x0D", 2));
    socket->write(txData);
}

void TcpServer::disconnected() {
    qDebug() << QString::number(socket->socketDescriptor()) + " Disconnected";
    socket->deleteLater();
    socket->close();
}
