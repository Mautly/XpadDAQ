#include "dataserver.h"

DataServer::DataServer(QObject *parent) :
    QTcpServer(parent)
{
    if (this->listen()) {
       /* QMessageBox::information(this, tr("DAQ Client"),
                              tr("Unable to start the data server: %1.")
                              .arg(this->errorString()));
                              */
        close();
        return;
    }

    QString ipAddress;

    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
}

void DataServer::incomingConnection(qintptr socketID){
    dataSocket = new QTcpSocket(this);
    dataSocket->setSocketDescriptor(socketID);

    dataSocket->waitForBytesWritten();

    dataSocket->readAll();

/*
            connect(s, SIGNAL(readyRead()), this, SLOT(readClient()));
                     connect(s, SIGNAL(disconnected()), this, SLOT(discardClient()));
                     s->setSocketDescriptor(socket);




    unsigned short value_short;
        unsigned int value_int;
        unsigned short *buffer_short = NULL;
        unsigned int *buffer_int = NULL;

        if (imageFormat == TwoBytes)
            buffer_short = (unsigned short *)bptr;

        else if (imageFormat == FourBytes)
            buffer_int = (unsigned int *)bptr;

        for (unsigned int i=0 ; i<readSize; i++){
            if (imageFormat == TwoBytes){
                read(data_port, &value_short, sizeof(unsigned short));
                buffer_short[i] = value_short;
            }
            else if (imageFormat == FourBytes){
                read(data_port, &value_int, sizeof(unsigned int));
                buffer_int[i]= value_int;
            }
        }*/
}
