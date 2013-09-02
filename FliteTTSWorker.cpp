//
//  FliteTTSWorker.cpp
//  TextReading
//
//  Created by roy_shilkrot on 8/28/13.
//
//

#include "FliteTTSWorker.h"

#include <QDebug>

void FliteTTSWorker::finishedPlaying(QAudio::State state)
{
    switch (state) {
        case QAudio::StoppedState:
            qDebug("Stopped state");
//            if (m_audioOutput->error() != QAudio::NoError) {
                // Perform error handling
//                qDebug("bad stuff happened");
//            } else {
//                m_audioOutput->stop();
                //                inputFile.close();
//                b.close();
//                delete m_audioOutput;
//                m_audioOutput = NULL;
//            }
            break;
        case QAudio::IdleState:
            qDebug("Idle state");
//            m_audioOutput->stop();
            //            inputFile.close();
//            b.close();
//            delete m_audioOutput;
//            m_audioOutput = NULL;
            break;
        case QAudio::ActiveState:
            qDebug("Active state");
            break;
        case QAudio::SuspendedState:
            qDebug("Suspended state");
            break;
        default:
            qDebug("some other thing happened");
    }
}

void FliteTTSWorker::run() {
    cst_wave* w = flite_text_to_wave(text.c_str(), voice);
    
//    qDebug() << "wave info \n\tsamples " << cst_wave_num_samples(w) <<
//    "\n\tfreq " << cst_wave_sample_rate(w) <<
//    "\n\tchannels " << cst_wave_num_channels(w) <<
//    "\n\tsize of sample " << sizeof(typeof(*(w->samples))) <<
//    "\n\ttype" << w->type <<
//    "\n";
    
    m_format.setFrequency(cst_wave_sample_rate(w));
    m_format.setChannels(cst_wave_num_channels(w));
    m_format.setSampleSize(sizeof(typeof(*(w->samples))) * 8); //bits per sample
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);
    
    QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
    if (!info.isFormatSupported(m_format)) {
        std::cerr << "Default format not supported - trying to use nearest";
        m_format = info.nearestFormat(m_format);
    }
    
    QAudioOutput m_audioOutput(m_format, 0);
//    connect(m_audioOutput,SIGNAL(stateChanged(QAudio::State)),this,SLOT(finishedPlaying(QAudio::State)));
    
    int sizeinbytes = cst_wave_num_samples(w) * sizeof(typeof(*(w->samples)));
    b.open(QIODevice::ReadWrite);
    b.write((char*)(cst_wave_samples(w)), sizeinbytes);
    b.seek(0);
    m_audioOutput.start(&b);
    //hold until sound is done
    QEventLoop loop;
    QObject::connect(&m_audioOutput, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
    do {
        loop.exec();
    } while(m_audioOutput.state() == QAudio::ActiveState);
}
