//
//  SeqentialTextReader.cpp
//  testopencv
//
//  Created by roy_shilkrot on 7/9/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#include "std.h"

#include "SeqentialTextReader.h"

/**
 * find distance of point to line in (vx,vy,x0,yx) form
 * @param p point
 * @param l line
 * @return the distance
 **/
float pointToLineD(const Point2f& p, const Vec4f& l) {
    // l = (vx, vy, x0, y0)
    Vec2f r(p.x - l[2], p.y - l[3]);
    return fabsf(r.dot(Vec2f(-l[1],l[0])));
}
/**
 * find distance of point from line in (m,b) (y = mx+b) form
 * @param p point
 * @param l line
 * @return the distanc
 **/
float pointToLineD(const Point2f& p, const Point2f& l) {
    // l = (m, b)
    Vec2f r(p.x, p.y - l.y);
    Vec2f normal(-l.x,1);
    float n = norm(normal);
    normal[0]/=n; normal[1]/=n;
    return fabsf(r.dot(normal));
}
/* Draws a line in the image given the line in (slope, intercept) form
 */
void drawLine(Mat img, Point2f lines, Scalar color, int thickness){
    Point p1(0, (int)lines.y);                  // y- intercept
    Point p2(1000, (int)(lines.x*1000 + lines.y));// (1000, ?)
    line(img, p1, p2, color, thickness);
}

void drawLine(Mat img, Vec4f lines, Scalar color, int thickness) {
    Point2f l = CVToMBLine(lines);
    drawLine(img,l,color,thickness);
}

void drawLines(Mat img, vector<Point2f> lines, Scalar color, int thickness){
    for (int i=0; i<lines.size(); i++) drawLine(img, lines[i], color, thickness);
}

void drawLines(Mat img, vector<STRLine> lines, Scalar color, int thickness){
    for (int i=0; i<lines.size(); i++) drawLine(img, lines[i].line, color, thickness);
}

class STDSequentialTextReader: public SequentialTextReader {
    
public:
    virtual void newWordFound(const std::string& str) {cout << "New word: " << str << endl;};
    virtual void endOfLine() { cout << "End of line.\n"; };
    virtual void escapeUp() { cout << "Escape up.\n"; };
    virtual void escapeDown() { cout << "Escape down\n."; };
    virtual void escapeDistance(int d) { cout << "Escape distance.\n"; };
};

int main_STR(int argc, char** argv) {
//    vector<string> images;
//    open_imgs_dir("/Users/roy_shilkrot/Dropbox/Eyering_KAR/Text Reading", images);
    
    STDSequentialTextReader str;
    
    VideoCapture vc;
    vc.open("/Users/roy_shilkrot/Documents/src/TextReading/build/Debug/Capture_20130826_5.avi");
    if(!vc.isOpened()) exit(0);
    
//    ofxFFMPEGVideoWriter writer;
    
//    for (int i=0; i<images.size(); i++) {
//        Mat img = imread(images[i]);
    Mat frame,img;
    vc >> frame;
//    vc.set(CV_CAP_PROP_POS_FRAMES, 100);
    
    
    while (vc.isOpened() && !frame.empty()) {
        vc >> frame;
        if(frame.empty()) break;
        
        frame.copyTo(img);
        
        str.processImage(img);

        imshow("img",img);

//        if (!writer.isInitialized()) {
//            writer.setup("/Users/roy_shilkrot/Desktop/output.gif", img.cols, img.rows);
//        }

//        cvtColor(img, img, CV_BGR2RGB);
//        writer.addFrame(img.data);
//        stringstream ss; ss <<"/Users/roy_shilkrot/Desktop/output/img"<<i<<".jpg";
//        imwrite(ss.str(), img);
        
        int c = waitKey();
        if(c==27) break;
    }
//    writer.close();

    return 1;
}
