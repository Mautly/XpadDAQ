#ifndef RECEIVEIMAGESTHREAD_H
#define RECEIVEIMAGESTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QFile>
#include <QDataStream>


class ReceiveImagesThread : public QThread
{
    Q_OBJECT

public:
    ReceiveImagesThread(int imageNumber, QString fileName, qintptr socketDescriptor);

private slots:
    void run();
    bool receiveImage(QString fileName, int imageNumber);

private:
    int             m_image_number;
    QString         m_filename;
    qintptr         m_socket_descriptor;
    QTcpSocket      *m_tcp_socket;
};

#endif // RECEIVEIMAGESTHREAD_H
