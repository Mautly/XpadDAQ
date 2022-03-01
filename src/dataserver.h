#ifndef DATASERVER_H
#define DATASERVER_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QMessageBox>

class DataServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit DataServer(QObject *parent = 0);

protected:
    void incomingConnection(qintptr socketID);

private:
    QTcpSocket  *dataSocket;

};

#endif // DATASERVER_H
