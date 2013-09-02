//
//  RecordVideo.cpp
//  testopencv
//
//  Created by roy_shilkrot on 7/9/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#include "RecordVideo.h"

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ofxFFMPEGVideoWriter.h"

using namespace cv;
using namespace std;

typedef Mat_<float> Matf;
typedef Mat_<double> Matd;

int main__(int argc, char** argv) {

    VideoCapture capture;
    capture.open(1);
    if (!capture.isOpened()) {
        cerr << "can't open video"<< endl; exit(0);
    }

    Mat img;
    capture >> img;

    ofxFFMPEGVideoWriter writer;
    writer.setup("output.mp4", img.cols, img.rows);
    
    bool saveVideo = false;
    while (true) {
        capture >> img;
        imshow("img",img);
        int c = waitKey(30);
        
        if (saveVideo) {
            Mat tmp;
            cvtColor(img, tmp, CV_BGR2RGB);
            writer.addFrame(tmp.data);
        }
        
        if (c==27) {
            break;
        } else if(c == ' ') {
            saveVideo = !saveVideo;
        }
    }
    writer.close();
}
