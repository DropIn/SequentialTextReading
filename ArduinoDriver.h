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
#include <QThread>

#include "QExtSerialPort-1.2rc/qextserialport.h"


class ArduinoDriver : public QThread {
    Q_OBJECT
    
    QextSerialPort* thread_port;
    std::string m_portname;

    bool arduinoSetup;
    
public:
    static const char TEXT_FOUND =  58;
    static const char UP =          51;
    static const char DOWN =        52;
    static const char END_OF_LINE = 57;
    static const char SYNC =        22;
    static const char CLEAR =       9;


    ArduinoDriver() {
    	arduinoSetup = false;
    }
    ~ArduinoDriver();

    void run();

    void connectSerial(const std::string&);
    bool isSetup() { return arduinoSetup; }
public slots:
	void sendByte(uchar b);
    void onDataAvailable();
};


#endif /* defined(__TextReading__ArduinoDriver__) */
