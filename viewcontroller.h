//
//  viewcontroller.h
//  TextReading
//
//  Created by roy_shilkrot on 8/22/13.
//
//

#ifndef TextReading_viewcontroller_h
#define TextReading_viewcontroller_h

#include <QFrame>
#include <QFileDialog>
#include "FliteTTSBridge.h"
#include "ArduinoDriver.h"

class ViewController : public QFrame {
    Q_OBJECT
    
    FliteTTSBridge ftb;
//    ArduinoDriver ad;

public:
    ViewController(QWidget* parent = 0);
    ~ViewController();
    
public slots:
//    void newFrame(const QImage& pm);
    void newFrame();
    void say();
    void newWordFound(std::string);

    void toggleHalfres(bool);
    void resetTracking();
    void setFocusLocation(int);
    void setFocusSize(int);
    void loadFile();
    void cameraSelect(int);
    void setCameraThresh(int);
    void togglePause(bool b);
    
    void connectPort();
    void fillPortsInfo();

    void textFound();
    void endOfLine();
    void sendUp();
    void sendDown();
    void sendDistance(int val);
    void sendClear();
    void trainFingertip();
};

#endif
