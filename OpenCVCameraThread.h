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
#include <QQueue>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

#include "QTSequentialTextReader.h"
#include "ArduinoDriver.h"

#define PROTECT(m,o) m.lock(); o m.unlock();

template<class T> class QAsyncQueue {
public:
	QAsyncQueue(unsigned int max_ = -1) :_max(max_) {}
	~QAsyncQueue() {clean();}

	bool isFull() {
		if (-1 == _max) return false;
		PROTECT(mtx,int count = _queue.count();)
		return count >= _max;
	}
	uint count() {PROTECT(mtx,int count = _queue.count();) return count;}
	bool isEmpty() {PROTECT(mtx,bool empty = _queue.isEmpty();) return empty;}
	void clean() {PROTECT(mtx,_queue.clear();)}
	void push(const T& t) {PROTECT(mtx,_queue.enqueue(t);)}
	void uniquePush(const T& t) {PROTECT(mtx,if(_queue.isEmpty()||_queue.head()!=t)_queue.enqueue(t);)}
	T pull() {PROTECT(mtx,T i = _queue.dequeue();) return i;}
private:
	QQueue<T> _queue;
	QMutex mtx;
	int _max;
};

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
    QAsyncQueue<char> arduinoCommandQueue;
    bool 			arduinoConnect;
    string			arduinoPort;
    QMutex          vc_mtx;
    
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
                if(!paused) {
                    PROTECT(vc_mtx,vc >> frame;)
                }
                
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
            if(arduinoConnect) {
            	ad.connectSerial(arduinoPort);
            	arduinoConnect = false;
            }
            while(!arduinoCommandQueue.isEmpty()) {
            	ad.sendByte(arduinoCommandQueue.pull());
            }
            msleep(15);
//            el.processEvents(QEventLoop::AllEvents, 30);
        }
    }
    
public:
    QMutex frame_mutex,str_mutex;
//    QTSequentialTextReader qtstr;
    SequentialTextReader str;
    ArduinoDriver ad;
    
    OpenCVCameraThread():QThread(),currentCamera(0),running(false),paused(false),arduinoConnect(false),arduinoPort("") {
        qRegisterMetaType<std::string>("std::string");
    }
    
    void cameraSelect(int i);
    void videoFile(const std::string& file);
    const Mat& getCurrentFrame() const;
    const Mat& getThresholdedFrame() const;
    void stopOcvCamera();
    void setDownscale(bool b);
    void setPaused(bool b);
    
signals:
//    void newFrame(const QImage& pm);
    void newFrame();
    
    void newWordFound(std::string str);
    void signalEndOfLine();
    void signalTextFound();
    void signalEscapeDistance(int d);

public:
    void endOfLine();
    void textFound();
    void escapeUp();
    void escapeDown();
    void escapeDistance(int d);
    void send(char c);
    void connectSerial(const string& port);
};


#endif /* defined(__TextReading__OpenCVCameraThread__) */
