#pragma once

#include <QTcpServer>
#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <iostream>
#include <QtMath>

class TcpServer : public QTcpServer
{
	Q_OBJECT
public:
	explicit TcpServer(QObject *parent = nullptr);
	void startServer();
    void setting(QString IP, quint16 port);

signals:
	void error(QTcpSocket::SocketError socketerror);

public slots:
	void readyRead();
	void disconnected();

protected:
	void incomingConnection(qintptr socketDescriptor);

private:
	QTcpSocket *socket;
    QString ip;
    quint16 port;
};

