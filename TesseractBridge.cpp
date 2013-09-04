//
//  TesseractBridge.cpp
//  testopencv
//
//  Created by roy_shilkrot on 7/11/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#include "TesseractBridge.h"

#include <tesseract/baseapi.h>
#include <tesseract/strngs.h>

#include <opencv2/imgproc/imgproc.hpp>

tesseract::TessBaseAPI  api;
string tessdata_dir = "/usr/local/Cellar/tesseract/3.02.02/share/tessdata/";

void TesseractBridge::init() {
    int rc = api.Init(tessdata_dir.c_str(), NULL);
    if (rc) {
        cerr << "Could not initialize tesseract.\n";
        exit(1);
    }
    api.End();

    rc = api.Init(tessdata_dir.c_str(), "eng", tesseract::OEM_DEFAULT);
    if (rc) {
        cerr << "Could not initialize tesseract.\n";
        exit(1);
    }

    api.SetPageSegMode(tesseract::PSM_SINGLE_LINE);
    
	printf("Tesseract Open Source OCR Engine v%s with Leptonica\n",
           tesseract::TessBaseAPI::Version());

}

void TesseractBridge::close() {
    api.End();
}

pair<int,string> TesseractBridge::process(const Mat& img, Rect& r) {
//    imshow("tesseract",img(r));
    Mat tmp; cvtColor(img(r), tmp, CV_BGR2GRAY);
//     cv::equalizeHist(tmp, tmp);
    adaptiveThreshold(tmp, tmp, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, 35);
//    imshow("eqhist", tmp);
    
//    char* cstr = api.TesseractRect(img.data, img.channels(), img.cols*img.channels(), r.x, r.y, r.width, r.height);
    char* cstr = api.TesseractRect(tmp.data, tmp.channels(), tmp.cols*tmp.channels(), 0, 0, tmp.cols, tmp.rows);
    
    delete[] cstr;
    
    tesseract::ResultIterator* ri = api.GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    if (ri != 0) {
//        do {
            const char* word = ri->GetUTF8Text(level);
        
        if(!word) return make_pair(0, "");
        
            float conf = ri->Confidence(level);
            int x1, y1, x2, y2;
            ri->BoundingBox(level, &x1, &y1, &x2, &y2);
//            printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
//                   word, conf, x1, y1, x2, y2);
            string theword(word);
            delete[] word;
        
        bool allnonalphanom = true;
        for (int i=0; i<theword.size(); i++) {
            allnonalphanom = allnonalphanom && !isalnum(theword[i]);
        }
        if (allnonalphanom) {
            return make_pair(0, "");
        }
            
//            if (conf > 80.0) {
                r.x += x1;
                r.y += y1;
                r.width = x2-x1;
                r.height = y2-y1;
                return make_pair((int)conf, theword);
//            } else {
//                return make_pair(0, "");
//            }
        
//        } while (ri->Next(level));
    }
/*
    
    int* confs = api.AllWordConfidences();
    int conf = 0;
    for(int i=0;;i++) {
        if(confs[i] == -1) break;
//        cout << confs[i] << endl;
        conf = confs[i];
    }
    delete[] confs;
    int len = strlen(cstr);
    if(len <= 0) return make_pair(0, "");
    
    string txt(cstr,len-2);
//    std::stringstream trimmer; trimmer << cstr; trimmer >> txt;
    
//    cout << "tesseract: " << txt << endl;
    
    return make_pair(conf, txt);
 */
}