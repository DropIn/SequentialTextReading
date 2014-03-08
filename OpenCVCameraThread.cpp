//
//  OpenCVCameraThread.cpp
//  TextReading
//
//  Created by roy_shilkrot on 8/22/13.
//
//

#include "OpenCVCameraThread.h"

void OpenCVCameraThread::cameraSelect(int i) {
	if(i == currentCamera) return;

    PROTECT(vc_mtx, if(vc.isOpened()) { vc.release(); } )
	currentCamera = i;
	PROTECT(vc_mtx, vc.open(currentCamera); )
}
void OpenCVCameraThread::videoFile(const std::string& file) {
    PROTECT(vc_mtx, if(vc.isOpened()) { vc.release(); } )
	
    currentCamera = -1;
	qDebug("Open video file %s",file.c_str());
    
	PROTECT(vc_mtx,vc.open(file);)
	if(vc.isOpened()) {
		PROTECT(vc_mtx,vc >> frame;)
	} else
		qDebug("Can't open file");
}


const Mat& 	OpenCVCameraThread::getCurrentFrame() const 		{ return frame_downscale; }
const Mat& 	OpenCVCameraThread::getThresholdedFrame() const 	{ return str.adaptiveForGUI; }
void 		OpenCVCameraThread::stopOcvCamera() 					{ running = false; }
void 		OpenCVCameraThread::setDownscale(bool b)		 		{ downscale = b; }
void 		OpenCVCameraThread::setPaused(bool b)	 				{ paused = b; }

void OpenCVCameraThread::endOfLine() {send(ArduinoDriver::END_OF_LINE); emit signalEndOfLine();}
void OpenCVCameraThread::textFound() {send(ArduinoDriver::TEXT_FOUND); emit signalTextFound();}
void OpenCVCameraThread::escapeUp() {send(ArduinoDriver::UP);}
void OpenCVCameraThread::escapeDown() {send(ArduinoDriver::DOWN);}
void OpenCVCameraThread::escapeDistance(int d, float a) { send(d); emit signalEscapeDistance(d,a);}
void OpenCVCameraThread::send(char c) { arduinoCommandQueue.uniquePush(c); }
void OpenCVCameraThread::connectSerial(const string& port) { arduinoPort = port; arduinoConnect = true; }

