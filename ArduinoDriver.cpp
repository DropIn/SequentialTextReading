//
//  ArduinoDriver.cpp
//
//  Created by roy_shilkrot on 8/28/13.
//
//

#include "ArduinoDriver.h"
#include <unistd.h>

#include <iostream>
#include <sstream>

#ifndef SSTR
#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
( std::ostringstream() << x ) ).str()
#endif

ArduinoDriver::~ArduinoDriver() {
	quit();
	wait();
}

void ArduinoDriver::run() {
	QextSerialPort port;
	thread_port = &port;
    port.setPortName(QString(m_portname.c_str()));
    port.setBaudRate(BAUD9600);

    connect(&port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));

    if(!port.open(QIODevice::ReadWrite)) {
        qDebug() << "cannot connect to " << port.portName();
        return;
    }

    arduinoSetup = true;

    exec();
    //thread ended, cleanup

    if (port.lineStatus() != 0) {
    	qDebug() << "closing port";
        port.close();
    }
}

void ArduinoDriver::onDataAvailable() {
	QByteArray ba = thread_port->readAll();
	qDebug() << ba.constData();
}

void ArduinoDriver::connectSerial(const std::string& portname) {
	if(isRunning()) {
		qDebug() << "thread is running. stopping...";
		quit();
		wait();
		qDebug() << "stopped";
	}
	m_portname = portname;
    start();
}

void ArduinoDriver::sendByte(uchar val) {
	if(isSetup()) {
		qDebug() << "Sending " << (int)val;
		thread_port->write((char*)&val, 1);
		thread_port->waitForBytesWritten(5);
	}
}
