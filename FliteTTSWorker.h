//
//  FliteTTSWorker.h
//  TextReading
//
//  Created by roy_shilkrot on 8/28/13.
//
//

#ifndef __TextReading__FliteTTSWorker__
#define __TextReading__FliteTTSWorker__

#include <iostream>
#include <QAudioOutput>
#include <QBuffer>
#include <QRunnable>
#include <QThreadPool>
#include <QEventLoop>

#ifdef WIN32
#include <flite.h>
#else
#include <flite/flite.h>
#endif

class FliteTTSWorker : public QObject, public QRunnable {
    Q_OBJECT
    
//    QAudioOutput*    m_audioOutput;
    QAudioFormat     m_format;
    QBuffer          b;
    std::string      text;
    cst_voice *      voice;
    
    QEventLoop       el;
    
public slots:
    void finishedPlaying(QAudio::State state);
    
public:
    void setVoice(cst_voice *v) { voice = v; }
    void setText(const std::string& t) { text = t;}
    void run();
};

#endif /* defined(__TextReading__FliteTTSWorker__) */
