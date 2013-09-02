//
//  ArduinoDriver.cpp
//  TextReading
//
//  Created by roy_shilkrot on 8/28/13.
//
//

#include "ArduinoDriver.h"

#ifndef SSTR
#include <sstream>
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
( std::ostringstream() << x ) ).str()
#endif

ArduinoDriver::~ArduinoDriver() {
    if (port.lineStatus() != 0) {
        port.close();
    }
}

void ArduinoDriver::connectSerial(const std::string& portname) {
    connect(&port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));

    port.setPortName(QString(portname.c_str()));
    port.setBaudRate(BAUD9600);
    if(!port.open(QIODevice::ReadWrite))
        qDebug() << "cannot connect to " << portname.c_str();
}

void ArduinoDriver::send(char val) {
    qDebug() << "Sending " << (int)val;
    port.write(&val, 1);
//    port.write(SSTR(val).c_str());
}
