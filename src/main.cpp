//#include "DAQ.h"
#include "daqclient.h"
#include <QApplication>
#include <QDir>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DAQClient w;
    w.show();
    
    return a.exec();
}
