#ifndef DAQVIEWER_H
#define DAQVIEWER_H

#include <QMainWindow>
#include <QDataStream>
#include <QPalette>
#include "qcustomplot.h"

#include <iostream>

namespace Ui {
class DAQViewer;
}

class DAQViewer : public QMainWindow
{
    Q_OBJECT

public:
    explicit DAQViewer(QMainWindow *parent = 0);
    ~DAQViewer();

    void resizeEvent(QResizeEvent *event);
    void printImage(int columns, int lines, QByteArray buffer);
    void setAcquireButtonColor(QColor color);
    void setLCDNumber(int value);
    void setFirstTimeOn();
    void setContinuousAcquisitionCheckBox(bool value);

private slots:
    void drawImage(qint32 *values);
    void updateImageValue(QMouseEvent* event);

    void on_comboBox_LUT_currentIndexChanged(int index);
    void on_pushButton_OpenImage_clicked();
    void on_pushButton_SaveAs_clicked();
    void on_pushButton_ResetImage_clicked();

    void on_horizontalSlider_Value_valueChanged(int value);
    void on_lineEdit_MinValue_textChanged(const QString &arg1);
    void on_lineEdit_MaxValue_textChanged(const QString &arg1);
    void on_pushButton_Acquire_clicked();
    void on_checkBox_Continue_clicked();  

signals:
    void acquiredButtonPressed();
    void setContinuousAcquisitionON();
    void setContinuousAcquisitionOFF();

private:
    Ui::DAQViewer       *ui;
    QCPColorMap         *m_color_map;
    QCPColorScale       *m_color_scale;
    int                 m_columns;
    int                 m_lines;
    qint32              *m_values;
    qint32              m_max_value;
    qint32              m_min_value;
    bool                m_first_time;    
    QString             m_file_name;

    QColor              m_imXPAD_color;
    QColor              m_imXPAD_green;
    QColor              m_imXPAD_red;
    QColor              m_imXPAD_orange;
    QColor              m_imXPAD_button;
    QColor              m_imXPAD_button_pushed;
    QColor              m_imXPAD_button_text;
    QColor              m_imXPAD_special_button;
};

#endif // DAQVIEWER_H
