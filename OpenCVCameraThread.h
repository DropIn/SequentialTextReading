//
//  OpenCVCameraThread.h
//  TextReading
//
//  Created by roy_shilkrot on 8/22/13.
//
//

#ifndef __TextReading__OpenCVCameraThread__
#define __TextReading__OpenCVCameraThread__

#include <iostream>

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QMetaType>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

#include "QTSequentialTextReader.h"
#include "ArduinoDriver.h"

class OpenCVCameraThread : public QThread, public SequentialTextReader::Handler
{
    Q_OBJECT

    VideoCapture    vc;
    Mat             frame,frame_downscale,frame_rgba;
    int             currentCamera;
    bool            running;
    bool            downscale;
    bool            paused;
//    QEventLoop      el;
    
protected:
    void run() {
        if(!vc.isOpened()) {
            vc.open(currentCamera);
        }
        if(!str.isInitialized()) {
        	str.setHandler(this);
        	str.init();
        }
        
        running = true;
        while (running) {
            if(vc.isOpened()) {
                if(!paused)
                    vc >> frame;
                
                if(!frame.empty()) {
                    frame_mutex.lock();
                    
                    if(downscale)
                        resize(frame, frame_downscale, Size(),0.5,0.5);
                    else
                        frame.copyTo(frame_downscale);
                    
                    cvtColor(frame_downscale, frame_rgba, CV_RGB2RGBA);
                    
                    frame_mutex.unlock();
                    
                    if(!paused) {
                        str_mutex.lock();
                        str.processImage(frame_downscale);
                        str_mutex.unlock();
                    } else {
                        rectangle(frame_downscale, str.getFocusArea(), Scalar(255));
                    }
//                    newFrame(QImage(frame_rgba.data, frame_rgba.cols, frame_rgba.rows,QImage::Format_RGB32));
                    newFrame();
                }
            }
            msleep(15);
//            el.processEvents(QEventLoop::AllEvents, 30);
        }
    }
    
public:
    QMutex frame_mutex,str_mutex;
//    QTSequentialTextReader qtstr;
    SequentialTextReader str;
    ArduinoDriver* ad;
    
    OpenCVCameraThread():QThread(),currentCamera(0),running(false),paused(false) {

        qRegisterMetaType<std::string>("std::string");
    }
    
    void cameraSelect(int i) {
        if(i == currentCamera) return;
        
        if(vc.isOpened()) {
            vc.release();
        }
        currentCamera = i;
        vc.open(currentCamera);
    }
    void videoFile(const std::string& file) {
        if(vc.isOpened()) {
            vc.release();
        }
        currentCamera = -1;
        vc.open(file);
        if(vc.isOpened())
            vc >> frame;
    }

    const Mat& getCurrentFrame() const { return frame_downscale; }
    const Mat& getThresholdedFrame() const { return str.adaptiveForGUI; }
    void stopOcvCamera() { running = false; }
    void setDownscale(bool b) { downscale = b; }
    void setPaused(bool b) { paused = b; }
    
signals:
//    void newFrame(const QImage& pm);
    void newFrame();
    
    void newWordFound(std::string str);

public:
    void endOfLine() {/*ad->send(ArduinoDriver::END_OF_LINE);*/}
    void textFound() {/*ad->send(ArduinoDriver::TEXT_FOUND);*/}
    void escapeUp() {/*ad->send(ArduinoDriver::UP);*/}
    void escapeDown() {/*ad->send(ArduinoDriver::DOWN);*/}
    void escapeDistance(int d) {/*ad->send(d);*/}

};


#endif /* defined(__TextReading__OpenCVCameraThread__) */
