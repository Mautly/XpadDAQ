#include "receiveimagesthread.h"

ReceiveImagesThread::ReceiveImagesThread(int imageNumber, QString fileName, qintptr socketDescriptor):
m_image_number(imageNumber), m_filename(fileName), m_socket_descriptor(socketDescriptor){

    m_tcp_socket = new QTcpSocket();
    m_tcp_socket->setSocketDescriptor(m_socket_descriptor);
}

void ReceiveImagesThread::run(){

    bool receivedImageState = true;

    for(int i=0; i<m_image_number; i++){

        if (i>0)
            m_tcp_socket->waitForReadyRead();

        receivedImageState = this->receiveImage(m_filename, i);

        if (receivedImageState == false){
            break;
        }
    }
}

bool ReceiveImagesThread::receiveImage(QString fileName, int imageNumber){

    quint32 dataSize = 0;
    quint32 LineFinalImage = 0;
    quint32 ColumnFinalImage = 0;
    quint32 bytesReceived = 0;
    QByteArray buffer;

    QDataStream in(m_tcp_socket);
    in.setByteOrder(QDataStream::LittleEndian);


    //cout << "Receiving image number : " << imageNumber << endl;
    //cout << fileName.toStdString() << endl;

    in >> dataSize >> LineFinalImage >> ColumnFinalImage;

    char character = dataSize & 0x000000FF;
    buffer.clear();
    if(dataSize > 0 && character != '*'){

        quint32 bytesReceivedBefore = 0;
        while(bytesReceived < dataSize){
            m_tcp_socket->waitForReadyRead(100);
            buffer.append( m_tcp_socket->read((dataSize - bytesReceived)));
            bytesReceived = buffer.size();
            bytesReceivedBefore = bytesReceived;
            //cout << "\tReceived " << bytesReceived << " out of " << dataSize << " Bytes" << endl;
        }

        QString imageFileName = fileName;

        return true;

        /*if (scanDACLVal<63)
            imageFileName +=  "_DACL_" + QString::number(scanDACLVal+1);

        if (numImages > 1)
            imageFileName += "_" + QString::number(imageNumber+1);

        if (ui->radioButton_Binary->isChecked())
            imageFileName += ".bin";
        else
            imageFileName += ".dat";*/


 /*       QFile file(imageFileName);
        qint32 *values = new qint32[LineFinalImage*ColumnFinalImage];

        if (m_parent->ui->radioButton_Binary->isChecked()){
            file.open(QIODevice::WriteOnly);
            file.write(buffer);
            file.close();
        }
        else{
            file.open(QIODevice::WriteOnly | QIODevice::Text);

            //Written to ASCII file
            QTextStream out(&file);
            QDataStream data(buffer);
            data.setByteOrder(QDataStream::LittleEndian);
            for (int i=0; i<LineFinalImage*ColumnFinalImage; i++)
                data >> values[i];
            for (int j=0; j<LineFinalImage; j++){
                for (int i=0; i<ColumnFinalImage; i++)
                    out << values[j*ColumnFinalImage + i] << " ";
                out << "\n";
            }
            file.close();
        }

        ui->lcdNumber_ImageNumber->display(imageNumber+1);
        ui->lcdNumber_ImageNumber->update();
        QApplication::processEvents();

        if(ui->checkBox_showImageAfterAcquisition->isChecked())
            this->ShowImage(imageFileName, LineFinalImage, ColumnFinalImage);

        QString dataSizeValue = QString::number(dataSize) + "\n";
        tcpSocket->write(dataSizeValue.toStdString().c_str());
        tcpSocket->waitForBytesWritten();

        m_viewer->printImage(ColumnFinalImage, LineFinalImage, buffer);

        delete[] values;
        file.close();

        return true;*/
    }
    else{
        /*QMessageBox::critical(this, tr("DAQ Client"),
                              tr("Receiving empty file... \n\n")
                              + tr("Last image WAS NOT saved."));
        QString dataSizeValue = "\n";
        tcpSocket->write(dataSizeValue.toStdString().c_str());
        tcpSocket->waitForBytesWritten();*/
        return false;
    }
}
