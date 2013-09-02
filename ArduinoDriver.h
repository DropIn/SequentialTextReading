//
//  ArduinoDriver.h
//  TextReading
//
//  Created by roy_shilkrot on 8/28/13.
//
//

#ifndef __TextReading__ArduinoDriver__
#define __TextReading__ArduinoDriver__

#include <iostream>
#include <QObject>
#include <QDebug>

#include "QExtSerialPort-1.2rc/qextserialport.h"

class ArduinoDriver : public QObject {
    Q_OBJECT
    
    QextSerialPort port;
    
public:
    ~ArduinoDriver();
    static const char TEXT_FOUND =  58;
    static const char UP =          51;
    static const char DOWN =        52;
    static const char END_OF_LINE = 57;
    static const char SYNC =        22;
    static const char CLEAR =       9;

    void connectSerial(const std::string&);
    void send(char);
    
    public slots:
    void onDataAvailable() {
//        qDebug() << "Port received " << port.readAll();
    }
};

#endif /* defined(__TextReading__ArduinoDriver__) */
