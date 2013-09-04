//
//  FliteTTSBridge.cpp
//  TextReading
//
//  Created by roy_shilkrot on 8/23/13.
//
//

#include "FliteTTSBridge.h"

//#ifdef WIN32
//#include <flite.h>
//#else
#include <flite/flite.h>
//#endif

#include "FliteTTSWorker.h"

cst_voice *v;

extern "C" {
    cst_voice *register_cmu_us_kal(const char *voxdir);
    void unregister_cmu_us_kal(cst_voice *v);
}

void FliteTTSBridge::init() {
    flite_init();
    
    v = register_cmu_us_kal(NULL);
    
//    threadPool = QThreadPool::globalInstance();
    threadPool.setMaxThreadCount(1);
}

void FliteTTSBridge::process(const std::string& text) {
    FliteTTSWorker* work = new FliteTTSWorker;
    work->setAutoDelete(true);
    work->setText(text);
    work->setVoice(v);
    threadPool.start(work);
}


void FliteTTSBridge::close() {
    unregister_cmu_us_kal(v);
    threadPool.waitForDone();
}
