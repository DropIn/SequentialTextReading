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

//TODO change this is to a local folder that goes with the executable
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

pair<int,string> TesseractBridge::process(const Mat& img, Mat& out, Rect& r) {
//    imshow("tesseract",img(r));
    Mat tmp; cvtColor(img(r), tmp, CV_BGR2GRAY);
    adaptiveThreshold(tmp, tmp, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 51, 35);
//    imshow("eqhist", tmp);
    
//    cvtColor(tmp, out(Rect(0,0,r.width,r.height)), CV_GRAY2BGR);
    
    char* cstr = api.TesseractRect(tmp.data, tmp.channels(), tmp.cols*tmp.channels(), 0, 0, tmp.cols, tmp.rows);
    
    delete[] cstr;
    
    tesseract::ResultIterator* ri = api.GetIterator();
    tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
    if (ri != 0) {
        const char* word = ri->GetUTF8Text(level);
        
        if(!word) return make_pair(0, "");
        
        float conf = ri->Confidence(level);
        int x1, y1, x2, y2;
        ri->BoundingBox(level, &x1, &y1, &x2, &y2);
//        printf("word: '%s';  \tconf: %.2f; BoundingBox: %d,%d,%d,%d;\n",
//               word, conf, x1, y1, x2, y2);
        string theword(word);
        delete[] word;
        
        bool allnonalphanom = true;
        for (int i=0; i<theword.size(); i++) {
            allnonalphanom = allnonalphanom && !(isalnum(theword[i]) || theword[i] == '"' || theword[i] == '.' || theword[i] == ',' || theword[i] == '?' || theword[i] == '!' || theword[i] == '\'');
        }
        if (allnonalphanom) return make_pair(0, "");
        
        r.x += x1;
        r.y += y1;
        r.width = x2-x1;
        r.height = y2-y1;
        return make_pair((int)conf, theword);
    } else
        return make_pair(0, "");
}