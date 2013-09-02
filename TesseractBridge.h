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
    pair<int,string> process(const Mat& img, Rect& r);
    void close();
};

#endif /* defined(__testopencv__TesseractBridge__) */
