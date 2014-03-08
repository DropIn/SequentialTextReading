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
    
    int freq = cst_wave_sample_rate(w);
    int numchannels = cst_wave_num_channels(w);
    int samplesize_bytes = sizeof(typeof(*(w->samples)));
    int samplesize = samplesize_bytes * 8;
    short* buf = (short*)(cst_wave_samples(w));
    int numsamples = cst_wave_num_samples(w);

    //downsample 16khz to 8khz (take every second sample, no filter)
    if(cst_wave_sample_rate(w)==16000) {
    	short* newwave = new short[numsamples/2+1];
    	int ii = 0;
    	int step = 2*samplesize_bytes;
    	for (int i = 0; i < numsamples; i+=2) {
    		if(i>=numsamples) break;
			newwave[ii] = buf[i];
			ii++;
		}
    	memset(buf,0,numsamples*samplesize_bytes);
    	memcpy(buf,newwave,numsamples * samplesize_bytes);
    	delete newwave;

    	numsamples = cst_wave_num_samples(w)/2;
    	freq = 8000;
    }

    m_format.setFrequency(freq);
    m_format.setChannels(numchannels);
    m_format.setSampleSize(samplesize); //bits per sample
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::SignedInt);
    

    if (!info->isFormatSupported(m_format)) {
        std::cerr << "Default format not supported - trying to use nearest";
        m_format = info->nearestFormat(m_format);
    }
    
    QAudioOutput m_audioOutput(m_format, 0);
//    connect(m_audioOutput,SIGNAL(stateChanged(QAudio::State)),this,SLOT(finishedPlaying(QAudio::State)));
    
    int sizeinbytes = numsamples * samplesize_bytes;
    b.open(QIODevice::ReadWrite);
    b.write((char*)buf, sizeinbytes);
    b.seek(0);
    m_audioOutput.start(&b);
    //hold until sound is done
    QEventLoop loop;
    QObject::connect(&m_audioOutput, SIGNAL(stateChanged(QAudio::State)), &loop, SLOT(quit()));
    do {
        loop.exec();
    } while(m_audioOutput.state() == QAudio::ActiveState);
}
