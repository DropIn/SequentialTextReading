//
//  QTSequentialTextReader.h
//  TextReading
//
//  Created by roy_shilkrot on 8/27/13.
//
//

#ifndef TextReading_QTSequentialTextReader_h
#define TextReading_QTSequentialTextReader_h

#include "SeqentialTextReader.h"

#include <QObject>

class QTSequentialTextReader : public QObject, public SequentialTextReader::Handler {
    Q_OBJECT
    SequentialTextReader str;
    
public:
    QTSequentialTextReader() {
        str.setHandler(this);
    }
signals:
     void newWordFound(const std::string& str);
     void endOfLine();
     void textFound();
     void escapeUp();
     void escapeDown();
     void escapeDistance(int d);
};

#endif
