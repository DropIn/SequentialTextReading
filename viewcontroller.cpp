//
//  viewcontroller.cpp
//  TextReading
//
//  Created by roy_shilkrot on 8/22/13.
//
//

#include "viewcontroller.h"

#include "OpenCVCameraThread.h"
#include "QTSequentialTextReader.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include "QExtSerialPort-1.2rc/qextserialenumerator.h"
#include <QDebug>
#include <QTextEdit>
#include <QPushButton>

OpenCVCameraThread ocvt;
//QTSequentialTextReader str;
Mat tmpframe,tmpframe_rgba;

ViewController::ViewController(QWidget* parent):QFrame(parent) {
//    connect(&ocvt, SIGNAL(newFrame(const QImage&)), this, SLOT(newFrame(const QImage&)));
    connect(&ocvt, SIGNAL(newFrame()), this, SLOT(newFrame()));
    connect(&ocvt,SIGNAL(newWordFound(std::string)), this, SLOT(newWordFound(std::string)));
    connect(&ocvt,SIGNAL(textFound()), this, SLOT(textFound()));
    connect(&ocvt,SIGNAL(endOfLine()), this, SLOT(endOfLine()));
    connect(&ocvt,SIGNAL(escapeDistance(int)), this, SLOT(sendDistance(int)));

    ocvt.setDownscale(false);
    ocvt.start();
    
    ftb.init();
    
    
//    fillPortsInfo();
}

ViewController::~ViewController() {
    ocvt.stopOcvCamera();
    ocvt.wait();
    
    ftb.close();
}

void ViewController::connectPort() {
    QComboBox* cb = parentWidget()->findChild<QComboBox*>("comboBox_comport");
    if(cb) {
        qDebug() << "Will connect to " << cb->currentText();
        ad.connectSerial(cb->currentText().toStdString());
    }
}

void ViewController::fillPortsInfo()
{
    QComboBox* cb = parentWidget()->findChild<QComboBox*>("comboBox_comport");
    if(cb) {
        if(cb->count() == 1) {
            cb->clear();
            
            foreach (const QextPortInfo &info, QextSerialEnumerator::getPorts()) {
                QStringList list;
                list << info.portName
                << info.friendName
                << info.physName
                << (info.vendorID ? QString::number(info.vendorID, 16) : QString())
                << (info.productID ? QString::number(info.productID, 16) : QString());
                
                cb->addItem(list.first(), list);
            }
        }
    }
}

void ViewController::cameraSelect(int i) { ocvt.cameraSelect(i); }
void ViewController::setCameraThresh(int t) { ocvt.str_mutex.lock(); ocvt.str.setThresh(t); ocvt.str_mutex.unlock(); }
void ViewController::resetTracking() {
    ocvt.str_mutex.lock(); ocvt.str.reset(); ocvt.str_mutex.unlock();
    QTextEdit* te = parentWidget()->findChild<QTextEdit*>("textEdit_cameraText");
    te->setPlainText("");
}
void ViewController::setFocusLocation(int v) { ocvt.str_mutex.lock(); ocvt.str.setFocusLocation(v); ocvt.str_mutex.unlock(); }
void ViewController::setFocusSize(int v) { ocvt.str_mutex.lock(); ocvt.str.setFocusSize(v); ocvt.str_mutex.unlock(); }
void ViewController::togglePause(bool b) { ocvt.setPaused(b); }


void ViewController::loadFile() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Open Video"), "", tr("Video Files (*.avi *.mov *.mp4)"));
    if(filename.length()>0) {
        resetTracking();
        ocvt.videoFile(filename.toStdString());
    }
}

void ViewController::toggleHalfres(bool b) {
    ocvt.setDownscale(b);
}

void ViewController::newFrame() {
    ocvt.frame_mutex.lock();
    ocvt.getCurrentFrame().copyTo(tmpframe);
//    ocvt.getThresholdedFrame().copyTo(tmpframe);
    ocvt.frame_mutex.unlock();
    
    if(tmpframe.channels() == 3)
        cvtColor(tmpframe, tmpframe_rgba, CV_RGB2RGBA);
    else if (tmpframe.channels() == 1)
        cvtColor(tmpframe, tmpframe_rgba, CV_GRAY2RGBA);

    QLabel* l = parentWidget()->findChild<QLabel*>("label_camInput");
    QPixmap pxmp; pxmp.convertFromImage(QImage(tmpframe_rgba.data, tmpframe_rgba.cols, tmpframe_rgba.rows,QImage::Format_RGB32));
    l->setPixmap(pxmp);
}

void ViewController::say() {
    QLineEdit* le = parentWidget()->findChild<QLineEdit*>("lineEdit_cameraText");
    std::cout << " saying " << le->text().toStdString() << std::endl;
    ftb.process(le->text().toStdString());
}

void ViewController::newWordFound(std::string s) {
    QLineEdit* le = parentWidget()->findChild<QLineEdit*>("lineEdit_cameraText");
    le->setText(QString::fromUtf8(s.c_str()).trimmed());
    
    QTextEdit* te = parentWidget()->findChild<QTextEdit*>("textEdit_cameraText");
    QString txt = te->toPlainText();
    QString newtxt = QString::fromUtf8(s.c_str()).trimmed() + " ";
    
    if(!txt.endsWith(newtxt)) {
        te->setPlainText(txt + newtxt);
        ftb.process(QString::fromUtf8(s.c_str()).trimmed().toStdString());
    }

    QPushButton* pb = parentWidget()->findChild<QPushButton*>("pushButton_endOfLine");
    pb->setStyleSheet("");
}

void ViewController::textFound() {ad.send(ArduinoDriver::TEXT_FOUND);};
void ViewController::endOfLine() {
    QPushButton* pb = parentWidget()->findChild<QPushButton*>("pushButton_endOfLine");
    pb->setStyleSheet("color: red;");
    ad.send(ArduinoDriver::END_OF_LINE);
};
void ViewController::sendUp() {ad.send(ArduinoDriver::UP);};
void ViewController::sendDown() {ad.send(ArduinoDriver::DOWN);};
void ViewController::sendDistance(int val) {ad.send(val);};
void ViewController::sendClear() {ad.send(ArduinoDriver::CLEAR);};
