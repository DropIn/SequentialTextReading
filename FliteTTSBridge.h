//
//  FliteTTSBridge.h
//  TextReading
//
//  Created by roy_shilkrot on 8/23/13.
//
//

#ifndef __TextReading__FliteTTSBridge__
#define __TextReading__FliteTTSBridge__

#include <iostream>
#include <QObject>
#include <QThreadPool>

class FliteTTSBridge : public QObject {
    Q_OBJECT

    QThreadPool threadPool;
    
public:
    void init();
    void process(const std::string& text);
    void close();

};

#endif /* defined(__TextReading__FliteTTSBridge__) */
