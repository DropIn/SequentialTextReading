//
//  TesseractBridge.h
//  testopencv
//
//  Created by roy_shilkrot on 7/11/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#ifndef __testopencv__TesseractBridge__
#define __testopencv__TesseractBridge__

#include <iostream>
#include "std.h"
using namespace cv;

class TesseractBridge {
    
    
public:
    void init();
    
    /**
     * Process an image patch with OCR for a single word
     * @param orig The image to process, assumed RGB24
     * @param img An output image to draw on
     * @param r The ROI in the image
     * @return The first word in the image and it's confidence level. May be <0,""> in case of error.
     **/
    pair<int,string> process(const Mat& orig, Mat& img, Rect& r);
    pair<int,string> process(const Mat& orig, Mat& img, Rect& r, float angle);
    pair<int,string> processEx(const Mat& tmp, Rect& r);

    void close();
};

#endif /* defined(__testopencv__TesseractBridge__) */
