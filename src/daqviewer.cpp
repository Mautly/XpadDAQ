#include "daqviewer.h"
#include "ui_daqviewer.h"

using namespace std;

DAQViewer::DAQViewer(QMainWindow *parent) :
    QMainWindow(parent),
    ui(new Ui::DAQViewer)
{
    ui->setupUi(this);

    m_columns = 576;
    m_lines = 118;

    // configure axis rect:
    ui->customPlot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    //ui->customPlot->axisRect()->setupFullAxesBox(true);
    //ui->customPlot->xAxis->setLabel("X");
    ui->customPlot->xAxis->setVisible(false);
    //ui->customPlot->yAxis->setLabel("Y");
    ui->customPlot->yAxis->setVisible(false);
    ui->customPlot->setMouseTracking(true);


    m_color_map = new QCPColorMap(ui->customPlot->xAxis, ui->customPlot->yAxis);
    m_color_map->setGradient(QCPColorGradient::gpCold);
    ui->customPlot->addPlottable(m_color_map);

    // add a color scale:
    m_color_scale = new QCPColorScale(ui->customPlot);
    //ui->customPlot->plotLayout()->addElement(0, 1, m_color_scale); // add it to the right of the main axis rect
    m_color_scale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    m_color_map->setColorScale(m_color_scale); // associate the color map with the color scale
    //m_color_scale->axis()->setLabel("Photon counting");

    // make sure the axis rect and color scale synchronize their bottom and top margins (so they line up):
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->customPlot);
    ui->customPlot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    m_color_scale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    marginGroup->deleteLater();

    m_imXPAD_color = QColor(25, 156, 146, 255);
    m_imXPAD_green = QColor(125,165,58,255);
    m_imXPAD_red = QColor(220,40,40,255);
    m_imXPAD_orange = QColor(243,159,32,255);
    m_imXPAD_button = QColor(72,79,79,255);
    m_imXPAD_button_pushed = QColor(108,112,113,255);
    m_imXPAD_button_text = QColor(245,242,235,255);
    m_imXPAD_special_button = QColor(108,112,113,255);

    QPalette palette = ui->pushButton_ResetImage->palette();
    palette.setColor(ui->pushButton_ResetImage->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_ResetImage->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_ResetImage->setAutoFillBackground(true);
    ui->pushButton_ResetImage->setPalette(palette);

    palette = ui->pushButton_OpenImage->palette();
    palette.setColor(ui->pushButton_OpenImage->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_OpenImage->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_OpenImage->setAutoFillBackground(true);
    ui->pushButton_OpenImage->setPalette(palette);

    palette = ui->pushButton_SaveAs->palette();
    palette.setColor(ui->pushButton_SaveAs->backgroundRole(), m_imXPAD_button);
    palette.setColor(ui->pushButton_SaveAs->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_SaveAs->setAutoFillBackground(true);
    ui->pushButton_SaveAs->setPalette(palette);

    palette = ui->pushButton_Acquire->palette();
    palette.setColor(ui->pushButton_Acquire->backgroundRole(), m_imXPAD_special_button);
    palette.setColor(ui->pushButton_Acquire->foregroundRole(), m_imXPAD_button_text);
    ui->pushButton_Acquire->setAutoFillBackground(true);
    ui->pushButton_Acquire->setPalette(palette);

    palette = ui->customPlot->palette();
    palette.setColor(ui->customPlot->backgroundRole(), m_imXPAD_special_button);
    palette.setColor(ui->customPlot->foregroundRole(), m_imXPAD_button_text);
    ui->customPlot->setAutoFillBackground(true);
    ui->customPlot->setPalette(palette);

    palette = ui->lcdNumber_ImageNumber->palette();
    //palette.setColor(ui->lcdNumber_ImageNumber->backgroundRole(), m_imXPAD_special_button);
    palette.setColor(ui->lcdNumber_ImageNumber->foregroundRole(), m_imXPAD_button_text);
    ui->customPlot->setAutoFillBackground(true);
    ui->customPlot->setPalette(palette);

    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(updateImageValue(QMouseEvent*)));
    ui->lineEdit_MaxValue->setValidator(new QIntValidator(1, 2147483647, this));
    this->on_pushButton_ResetImage_clicked();

    m_values = new qint32[1];
    m_max_value = -2147483646;
    m_min_value = 2147483647;
    m_first_time = true;
    m_file_name = QDir::homePath() + "/XPAD_DAQ/";
}

DAQViewer::~DAQViewer()
{
    delete[] m_values;
    delete ui;
}

void DAQViewer::resizeEvent(QResizeEvent *e){


    QSize size_frame = this->frameSize();
    int width = size_frame.width() - 100;
    int height = size_frame.height();

    //cout << "Width: " << width + 100 << " Height: " << height << endl;

    height = width*((double)m_lines/m_columns);

    ui->customPlot->setFixedSize(width+30, height+30);
    ui->horizontalLayoutWidget->setFixedSize(ui->customPlot->width()+30, ui->customPlot->height()+80);

    this->setMinimumHeight(ui->horizontalLayoutWidget->height()+20);
    this->setMaximumHeight(ui->horizontalLayoutWidget->height()+20);
    this->setMinimumWidth(295);
    this->setMaximumWidth(QWIDGETSIZE_MAX);


    this->on_pushButton_ResetImage_clicked();
}

void DAQViewer::printImage(int columns, int lines, QByteArray buffer){


    qint32 *values = new qint32[lines*columns];
    if (!ui->checkBox_Hold->isChecked()){
        delete[] m_values;
        m_values = new qint32[lines*columns];
    }

    QDataStream data(buffer);
    data.setByteOrder(QDataStream::LittleEndian);
    for (int i=0; i<lines*columns; i++){
        data >> values[i];
        if (ui->checkBox_Hold->isChecked())
            m_values[i] += values[i];
        else
            m_values[i] = values[i];
    }

    m_columns = columns;
    m_lines = lines;

    this->drawImage(values);

    delete[] values;
}

void DAQViewer::drawImage(qint32 *values){

    m_color_map->data()->clear();
    m_color_map->data()->setRange(QCPRange(0, m_columns), QCPRange(0, m_lines));
    m_color_map->data()->setSize(m_columns, m_lines);

    double x, y, z;
    m_max_value = -2147483646;
    m_min_value = 2147483647;

    int i = 0;
    for (int yIndex=0; yIndex<m_lines; ++yIndex)
        for (int xIndex=0; xIndex<m_columns; ++xIndex){
            m_color_map->data()->cellToCoord(xIndex, yIndex, &x, &y);

            if (ui->checkBox_Hold->isChecked() && values[i] >= 0)
                m_values[i] += values[i];
            else
                m_values[i] = values[i];

            z = m_values[i];

            m_color_map->data()->setCell(xIndex, m_lines -yIndex - 1, m_values[i]);
            i++;
            if ( z > m_max_value)
                m_max_value = z;
            if ( z < m_min_value)
                m_min_value = z;
        }

    if (m_min_value < -4)
        m_min_value = -4;
    ui->horizontalSlider_Value->setMinimum(m_min_value);
    ui->horizontalSlider_Value->setMaximum(m_max_value);

    if (m_first_time == true){
        ui->horizontalSlider_Value->setValue(m_max_value);
        ui->lineEdit_MinValue->setText(QString::number(m_min_value));
        ui->lineEdit_MaxValue->setText(QString::number(m_max_value));
        ui->lineEdit_MinValue->setValidator(new QIntValidator(m_min_value, -2147483648, this));
        ui->lineEdit_MaxValue->setValidator(new QIntValidator(m_min_value, 2147483647, this));
    }

    int width = 0;
    int height = 0;

    if (m_first_time){

        QRect rec = QApplication::desktop()->screenGeometry();

        float width_screen_proportion = 0.5;
        float height_screen_proportion = 0.3;

        if (m_columns > m_lines){
            width = rec.width() - rec.width()*width_screen_proportion;
            height = round(width*m_lines/m_columns);
            if (height > rec.height()){
                height = rec.height() - rec.height()*height_screen_proportion;
                width = round(height*m_columns/m_lines);
            }

        }else{
            height = rec.height() - rec.height()*height_screen_proportion;
            width = round(height*m_columns/m_lines);
            if (width > rec.width()){
                width = rec.width() - rec.width()*width_screen_proportion;
                height = round (width*m_lines/m_columns);
            }
        }

        ui->customPlot->setFixedSize(width+30,height+30);
        ui->horizontalLayoutWidget->setFixedSize(ui->customPlot->width()+30, ui->customPlot->height()+80);
        this->setFixedSize(ui->horizontalLayoutWidget->width()+20,ui->horizontalLayoutWidget->height()+20);

        this->setMinimumSize(0,0);
        ui->horizontalLayoutWidget->setMinimumSize(0,0);
        this->setMaximumSize(QWIDGETSIZE_MAX ,QWIDGETSIZE_MAX );
        ui->horizontalLayoutWidget->setMaximumSize(QWIDGETSIZE_MAX ,QWIDGETSIZE_MAX );

        //rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
        m_color_map->rescaleDataRange();
        m_color_map->setDataRange(QCPRange(m_min_value, ui->horizontalSlider_Value->value()));
        m_color_scale->rescaleDataRange(true);
        m_color_scale->setDataRange(QCPRange(m_min_value, ui->horizontalSlider_Value->value()));
    }

    m_first_time = false;

    this->on_pushButton_ResetImage_clicked();
}

void DAQViewer::updateImageValue(QMouseEvent* event){

    int x_position = ui->customPlot->xAxis->pixelToCoord(event->pos().x());
    int y_position = ui->customPlot->yAxis->pixelToCoord(event->pos().y());

    if (x_position >=0 && y_position >=0 && x_position<m_columns && y_position<m_lines){
        QString message = "Pixel value(" + QString::number(x_position) + ", " + QString::number(m_lines - y_position - 1) + ") = " + QString::number(m_values[(m_lines - y_position - 1)*m_columns + x_position]);
        ui->customPlot->setToolTip(message);
        ui->label_pixelValue->setText(message);
    }
}


void DAQViewer::on_comboBox_LUT_currentIndexChanged(int index){
    switch(index){
    case 0:  m_color_map->setGradient(QCPColorGradient::gpGrayscale); break;
    case 1:  m_color_map->setGradient(QCPColorGradient::gpHot); break;
    case 2:  m_color_map->setGradient(QCPColorGradient::gpCold); break;
    case 3:  m_color_map->setGradient(QCPColorGradient::gpNight); break;
    case 4:  m_color_map->setGradient(QCPColorGradient::gpCandy); break;
    case 5:  m_color_map->setGradient(QCPColorGradient::gpGeography); break;
    case 6:  m_color_map->setGradient(QCPColorGradient::gpIon); break;
    case 7:  m_color_map->setGradient(QCPColorGradient::gpThermal); break;
    case 8:  m_color_map->setGradient(QCPColorGradient::gpPolar); break;
    case 9:  m_color_map->setGradient(QCPColorGradient::gpSpectrum); break;
    case 10:  m_color_map->setGradient(QCPColorGradient::gpJet); break;
    case 11:  m_color_map->setGradient(QCPColorGradient::gpHues); break;
    }
    ui->customPlot->replot();
}

void DAQViewer::on_pushButton_OpenImage_clicked(){
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open File"), QFileInfo(m_file_name).path(), tr("Images (*.bin *.dat)"));

    if (file_name.isEmpty())
        return;

    m_file_name = file_name;

    delete[] m_values;

    if (QFileInfo(file_name).fileName().contains(".bin")){
        QFile file(file_name);
        if(!file.open(QIODevice::ReadOnly)){
            QMessageBox::critical(this, tr("DAQ Client"),
                                  file_name + tr(" could not be open."));
            return;
        }

        QByteArray buffer_data = file.readAll();
        file.close();

        m_columns = QInputDialog::getInt(this,tr("Get image pixel width"),tr("Image width [pixel]: "),m_columns, 0,65536);
        m_lines = QInputDialog::getInt(this,tr("Get image pixel height"),tr("Image height [pixel]: "),m_lines, 0,65536);
        qint32 *data = new qint32[m_columns*m_lines];
        m_values = new qint32[m_columns*m_lines];

        QDataStream values(buffer_data);
        values.setByteOrder(QDataStream::LittleEndian);
        for (int i=0; i<m_columns*m_lines; i++)
            values >> data[i];

        m_first_time = true;
        this->drawImage(data);
        m_first_time = true;

        delete[] data;
    }
    else if (QFileInfo(file_name).fileName().contains(".dat")){
        QFile file(file_name);
        //cout << "Filename = " << file_name.toStdString() << endl;
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
            QMessageBox::critical(this, tr("DAQ Client"),
                                  file_name + tr(" could not be open."));
            return;
        }

        QByteArray buffer_data = file.readLine();
        QString string_data = buffer_data.toStdString().c_str();
        //cout << "Data = " << string_data.toStdString() << endl;
        QStringList data_list = string_data.split(QRegExp("[ \t\n]"), QString::SkipEmptyParts);
        m_columns = data_list.length();
        //cout << "Width = " << m_columns << " ";

        buffer_data = file.readAll();
        string_data = buffer_data.toStdString().c_str();
        data_list.append(string_data.split(QRegExp("[ \t\n]"), QString::SkipEmptyParts));
        m_lines = round (data_list.length()/m_columns);
        //cout << "Height = "  << m_lines << endl;

        file.close();

        qint32 *data = new qint32[m_columns*m_lines];
        m_values = new qint32[m_columns*m_lines];

        for(int i=0; i<m_columns*m_lines; i++)
            data[i] = (quint32) data_list[i].toLongLong();

        m_first_time = true;
        this->drawImage(data);
        m_first_time = true;

        delete[] data;

    }else{
        QMessageBox::critical(this, tr("DAQ Visualizer"),
                              file_name + tr(" could not be open.\n\n")
                              + tr("Image type not supported."));
    }
}

void DAQViewer::on_pushButton_SaveAs_clicked(){
    /*QString file_name = QFileDialog::getSaveFileName(this, tr("Save As"), QFileInfo(m_file_name).path(), tr("Images (*.jpg *.bmp *.png)"));

    if(file_name.isEmpty())
        return;

    m_file_name = file_name;

    //if(!fileName.contains(".jpg",Qt::CaseInsensitive)  && !fileName.contains(".bmp",Qt::CaseInsensitive) && !fileName.contains(".png",Qt::CaseInsensitive))
    //    fileName.append(".jpg");

    if(file_name.contains(".jpg", Qt::CaseInsensitive))
        ui->customPlot->saveJpg(file_name,m_columns*2,m_lines*2);
    else if (file_name.contains(".bmp", Qt::CaseInsensitive))
        ui->customPlot->saveBmp(file_name,m_columns*2,m_lines*2);
    else if (file_name.contains(".png", Qt::CaseInsensitive))
        ui->customPlot->savePng(file_name,m_columns*2,m_lines*2);
    else{
        QMessageBox::warning(this, tr("DAQ Visualizer"),
                             tr("Format could not be recognized.\nImage was not saved!"));
    }*/

    QString selectFilter;
    QString file_name = QFileDialog::getSaveFileName(this, tr("Save As"), QFileInfo(m_file_name).path(), tr("Ascii (*.dat) ;; Binary (*.bin) ;; Images (*.jpg) ;; Images (*.bmp) ;; Images (*.png)"),&selectFilter);

    m_file_name = file_name;
    //cout << "Select Fileter = " << selectFilter.toStdString() << endl;

    if(file_name.isEmpty())
        return;

    if(selectFilter.contains(".jpg", Qt::CaseInsensitive)){
        if(!file_name.contains(".jpg", Qt::CaseInsensitive))
            file_name += ".jpg";

        ui->customPlot->saveJpg(file_name,m_columns*2,m_lines*2);
    }
    else if (selectFilter.contains(".bmp", Qt::CaseInsensitive)){
        if(!file_name.contains(".bmp", Qt::CaseInsensitive))
            file_name += ".bmp";

        ui->customPlot->saveBmp(file_name,m_columns*2,m_lines*2);
    }
    else if (selectFilter.contains(".png", Qt::CaseInsensitive)){
        if(!file_name.contains(".png", Qt::CaseInsensitive))
            file_name += ".png";

        ui->customPlot->savePng(file_name,m_columns*2,m_lines*2);
    }
    else if(selectFilter.contains(".dat", Qt::CaseInsensitive)){
        if(!file_name.contains(".dat", Qt::CaseInsensitive))
            file_name += ".dat";
        FILE *fd = fopen(file_name.toStdString().c_str(),"w+");
        for(int row=0;row<m_lines;row++){
            for(int col=0;col<m_columns;col++){
                fprintf(fd,"%d ",m_values[row*m_columns+col]);
            }
            fprintf(fd,"\n");
        }
        fclose(fd);
    }
    else if(selectFilter.contains(".bin", Qt::CaseInsensitive)){
        if(!file_name.contains(".bin", Qt::CaseInsensitive))
            file_name += ".bin";

        FILE *fd = fopen(file_name.toStdString().c_str(),"w+");
        fwrite(m_values,sizeof(qint32),m_columns*m_lines,fd);
        fclose(fd);
    }
    else{
        QMessageBox::warning(this, tr("DAQ Visualizer"),
                             tr("Format could not be recognized.\nImage was not saved!"));
    }
    m_file_name = file_name;
}

void DAQViewer::on_pushButton_ResetImage_clicked(){
    //QSize size_frame = this->frameSize();
    //ui->horizontalLayoutWidget->resize(size_frame.width()-20, size_frame.height()-40);

    ui->customPlot->xAxis->setRangeLower(0);
    ui->customPlot->xAxis->setRangeUpper(m_columns);
#ifdef __APPLE__
    ui->customPlot->yAxis->setRangeLower(-20);
#else
    ui->customPlot->yAxis->setRangeLower(0);
#endif
    ui->customPlot->yAxis->setRangeUpper(m_lines);
    m_color_map->data()->setSize(m_columns, m_lines); // we want the color map
    ui->customPlot->replot();
    QString message = "Image size = " + QString::number(m_columns) + " x " + QString::number(m_lines) + " ";
    ui->label_imageSize->setText(message);
}

void DAQViewer::on_horizontalSlider_Value_valueChanged(int value){
    ui->lineEdit_MaxValue->setText(QString::number(value));
    m_color_map->setDataRange(QCPRange(m_min_value, value));
    m_color_scale->setDataRange(QCPRange(m_min_value, value));
    ui->customPlot->replot();
}

void DAQViewer::on_lineEdit_MinValue_textChanged(const QString &arg1){
    int value = arg1.toInt();
    m_min_value = value;
    ui->horizontalSlider_Value->setMinimum(value);
    m_color_map->setDataRange(QCPRange(m_min_value, ui->lineEdit_MaxValue->text().toInt()));
    m_color_scale->setDataRange(QCPRange(m_min_value, ui->lineEdit_MaxValue->text().toInt()));
    ui->customPlot->replot();
}

void DAQViewer::on_lineEdit_MaxValue_textChanged(const QString &arg1){
    int value = arg1.toInt();
    m_max_value = value;
    ui->horizontalSlider_Value->setValue(value);
    m_color_map->setDataRange(QCPRange(ui->lineEdit_MinValue->text().toInt(), m_max_value));
    m_color_scale->setDataRange(QCPRange(ui->lineEdit_MinValue->text().toInt(), m_max_value));
    ui->customPlot->replot();
}

void DAQViewer::setAcquireButtonColor(QColor color){
    QPalette palette = ui->pushButton_Acquire->palette();
    palette.setColor(ui->pushButton_Acquire->backgroundRole(),color);
    palette.setColor(ui->pushButton_Acquire->foregroundRole(),m_imXPAD_button_text);
    ui->pushButton_Acquire->setAutoFillBackground(true);
    ui->pushButton_Acquire->setPalette(palette);

    if (color == m_imXPAD_orange)
        ui->pushButton_Acquire->setEnabled(false);
    else{
        ui->pushButton_Acquire->setEnabled(true);
    }
}

void DAQViewer::setLCDNumber(int value){
    ui->lcdNumber_ImageNumber->display(value);
    ui->lcdNumber_ImageNumber->update();
}

void DAQViewer::setFirstTimeOn(){
    m_first_time = true;
}

void DAQViewer::setContinuousAcquisitionCheckBox(bool value){
    ui->checkBox_Continue->setChecked(value);
}

void DAQViewer::on_pushButton_Acquire_clicked(){
    emit acquiredButtonPressed();
}

void DAQViewer::on_checkBox_Continue_clicked(){
    if (ui->checkBox_Continue->isChecked())
        emit setContinuousAcquisitionON();
    else
        emit setContinuousAcquisitionOFF();
}


