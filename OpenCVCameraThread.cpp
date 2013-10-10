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

	if(vc.isOpened()) {
		vc.release();
	}
	currentCamera = i;
	vc.open(currentCamera);
}
void OpenCVCameraThread::videoFile(const std::string& file) {
	if(vc.isOpened()) {
		vc.release();
	}
	currentCamera = -1;
	qDebug("Open video file %s",file.c_str());
	vc.open(file);
	if(vc.isOpened())
		vc >> frame;
	else
		qDebug("Can't open file");
}


const Mat& 	OpenCVCameraThread::getCurrentFrame() const 		{ return frame_downscale; }
const Mat& 	OpenCVCameraThread::getThresholdedFrame() const 	{ return str.adaptiveForGUI; }
void 		OpenCVCameraThread::stopOcvCamera() 					{ running = false; }
void 		OpenCVCameraThread::setDownscale(bool b)		 		{ downscale = b; }
void 		OpenCVCameraThread::setPaused(bool b)	 				{ paused = b; }

void OpenCVCameraThread::endOfLine() {send(ArduinoDriver::END_OF_LINE);}
void OpenCVCameraThread::textFound() {send(ArduinoDriver::TEXT_FOUND);}
void OpenCVCameraThread::escapeUp() {send(ArduinoDriver::UP);}
void OpenCVCameraThread::escapeDown() {send(ArduinoDriver::DOWN);}
void OpenCVCameraThread::escapeDistance(int d) {send(d);}
void OpenCVCameraThread::send(char c) { arduinoCommandQueue.uniquePush(c); }
void OpenCVCameraThread::connectSerial(const string& port) { arduinoPort = port; arduinoConnect = true; }

