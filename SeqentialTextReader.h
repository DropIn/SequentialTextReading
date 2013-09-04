//
//  SeqentialTextReader.h
//  testopencv
//
//  Created by roy_shilkrot on 7/9/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#ifndef __testopencv__SeqentialTextReader__
#define __testopencv__SeqentialTextReader__

#include <iostream>
#include <iterator>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#include <boost/smart_ptr.hpp>

#include "MathGLTools.h"

#include "AbstractAlgorithm.h"
#include "TesseractBridge.h"

#undef ENABLE_PROFILE

#ifdef ENABLE_PROFILE
#define CV_PROFILE_MSG(msg,code)	\
{\
std::cout << msg << " ";\
double __time_in_ticks = (double)cv::getTickCount();\
{ code }\
std::cout << "DONE " << ((double)cv::getTickCount() - __time_in_ticks)/cv::getTickFrequency() << "s" << std::endl;\
}

#define CV_PROFILE(code)	\
{\
std::cout << #code << " ";\
double __time_in_ticks = (double)cv::getTickCount();\
{ code }\
std::cout << "DONE " << ((double)cv::getTickCount() - __time_in_ticks)/cv::getTickFrequency() << "s" << std::endl;\
}
#else
#define CV_PROFILE_MSG(msg,code) code
#define CV_PROFILE(code) code
#endif


/* Computes the line between two points.
 returns the line in slope intercept form Point2f (slope, intercept)
 */
template<typename V, typename T1, typename T2>
Point_<V> getLine(Point_<T1> point1, Point_<T2> point2){
    V x = (point2.y-point1.y)/V(point2.x - point1.x);
    V y = point2.y - x*point2.x;
    return Point_<V>(x,y);
}

/**
 * find distance of point to line in (vx,vy,x0,yx) form
 * @param p point
 * @param l line
 * @return the distance
 **/
float pointToLineD(const Point2f& p, const Vec4f& l);

/**
 * find distance of point from line in (m,b) (y = mx+b) form
 * @param p point
 * @param l line
 * @return the distanc
 **/
float pointToLineD(const Point2f& p, const Point2f& l);

/* Draws a line in the image given the line in (slope, intercept) form
 */
void drawLine(Mat img, Point2f lines, Scalar color, int thickness = 1);
void drawLine(Mat img, Vec4f lines, Scalar color, int thickness = 1);
void drawLines(Mat img, vector<Point2f> lines, Scalar color, int thickness = 1);

inline Point2f CVToMBLine(const Vec4f& l) {
    float m = l[1]/l[0];
    return Point2f(m,l[3]-m*l[2]);
}
inline Vec4f MBToCVLine(const Point2f& l) {
    Vec2f v(1.0f,l.x);
    float n = norm(v);
    v[0] /= n; v[1] /= n;
    return Vec4f(v[0],v[1],0,l.y);
}


typedef vector<Point> Character;
typedef vector<Character> Characters;

struct STRLine {
    vector<Point2f> support;
    Characters characters;
    Point2f line;
    
    STRLine() {}
    //    STRLine(STRLine& l) { support = l.support; characters = l.characters; line = l.line; }
    
    STRLine(const Point2f& l, const Characters& cs) {
        line = l;
        characters = cs;
        
        for (int c=0; c<characters.size(); c++) {
            vector<Point> contour = characters[c];
            Point2f bottomp = *(std::max_element(contour.begin(), contour.end(), sortpointsbyy<int>));
            support.push_back(bottomp);
        }
    }
    
    friend std::ostream& operator<< (std::ostream& stream, const STRLine& l) {
        stream << l.line << ": ";
        copy(l.support.begin(), l.support.end(), ostream_iterator<cv::Point2f >(cout,", "));
        return stream;
    }
};

        void drawLines(Mat img, vector<STRLine> lines, Scalar color, int thickness = 1);

        
typedef pair<int,string> TextConf;

struct STRWord {
    vector<Mat>                 patches;
    TextConf                    text;
    vector<pair<Point,Mat> > 	chars;
    int                         times_seen;
    bool                        sent_to_tts;
    
    STRWord(const Mat& p, TextConf t) {
        patches.push_back(p); text = t;
        times_seen = 0;
        sent_to_tts = false;
    }
};
        
struct STRLeftover {
    Mat patch;
    int times_seen;
    
    STRLeftover(const Mat& ptch) { ptch.copyTo(patch); times_seen = 0; }
};

class SequentialTextReader : public AbstractAlgorithm {
    
    vector<Point2f>                     candidatePoints;
    Characters                          candidateContours;
    int                                 frameNum;
    Mat                                 lastAdaptive;
    Mat                                 lastFrame;
    bool                                foundFirstWord;
    TesseractBridge                     TB;
    vector<pair<Point,STRWord> >        trackedWords;
    STRLine                             trackedLine;
    vector<pair<Point,STRLeftover> > 	trackedLeftovers;
    Vec2f                           	lastMotion;
    Rect                                focusArea, origFocusArea;
    
    static const float                  POINT_TO_LINE_THRESH = 15.0f;
    static const float                  MAX_CONTOUR_AREA = 1500.0f;
    static const float                  MIN_CONTOUR_AREA = 20.0f;
    static const int                    MIN_POINTS_FOR_LINE = 7;
    int                                 ADAPTIVE_THRESH;
    static const float                  LINE_ANGLE_THRESH = 0.2;
    static const int                    TRACKED_WORD_UNSEEN_THRESH = -7;
    static const int                    TRACKED_WORD_UNSEEN_UPPER_THRESH = 10;
    static const int                    TRACKED_LEFTOVERS_UNSEEN_THRESH = -10;
    float                               NEW_WORD_CUTOFF_POSITION_FACTOR;
    static const int                    NEW_WORD_CONF_THRESH = 76;
    
    vector<STRLine> findCandidateLines(Mat& img) {
        vector<STRLine> lines;
        vector<Point2f> ps = candidatePoints;
        for (int i=0; i<ps.size()-1; i++) {
            for (int j=i+1; j<ps.size(); j++) {
                Scalar color( 100+(rand()&155), 100+(rand()&155), 100+(rand()&155) );
                //                line(img, ps[i], ps[j], color);
                Point2f linep = getLine<float>(ps[i], ps[j]);
                if (fabsf(linep.x) > 0.2) {
                    continue;
                }
                lines.push_back(STRLine(linep,Characters(1,candidateContours[i])));
                drawLine(img, linep, color);
            }
        }
        return lines;
    }
    
    vector<STRLine> findCandidateLines2(Mat& img) {
        vector<Point2f> ps = candidatePoints;
        vector<pair<float,STRLine> > lines_and_err;
//        int low_b = focusArea.y, high_b = focusArea.y+focusArea.height;
        rectangle(img, focusArea, Scalar(255));
        rectangle(img, origFocusArea, Scalar(255,128,128));

        for (int i=0; i<ps.size(); i++) {
//            if(ps[i].y < low_b || ps[i].y > high_b) continue; //prune extremal points
            if(!focusArea.contains(ps[i])) continue;

            for (int j=i+1; j<ps.size(); j++) {
//                if(ps[j].y < low_b || ps[j].y > high_b) continue; //prune extremal points
                if(!focusArea.contains(ps[j])) continue;

                float nrm = norm(ps[i]-ps[j]);
                if(nrm < 5 || nrm > 80) continue;
                
                for (int k=j+1; k<ps.size(); k++) {
//                    if(ps[k].y < low_b || ps[k].y > high_b) continue; //prune extremal points
                    if(!focusArea.contains(ps[k])) continue;

                    float nrm = norm(ps[k]-ps[i]);
                    if(nrm < 5) continue;
                    
                    nrm = norm(ps[k]-ps[j]);
                    if(nrm < 5 || nrm > 80) continue;
                    
                    vector<Point2f> tofit;
                    tofit.push_back(ps[i]);
                    tofit.push_back(ps[j]);
                    tofit.push_back(ps[k]);
                    Vec4f candidate;
                    fitLine(tofit, candidate, CV_DIST_L2, 0, 0.01, 0.01);
                    
                    float dsum = pointToLineD(tofit[0],candidate) +
                                pointToLineD(tofit[1],candidate) +
                                pointToLineD(tofit[2],candidate);
                    if(dsum > POINT_TO_LINE_THRESH) continue; //early bail
                    
                    Characters cs;
                    cs.push_back(candidateContours[i]);
                    cs.push_back(candidateContours[j]);
                    cs.push_back(candidateContours[k]);
                    
                    STRLine l(CVToMBLine(candidate),cs);
                    if(fabsf(l.line.x) > LINE_ANGLE_THRESH) continue; //prune based on angle
                                        
                    lines_and_err.push_back(make_pair(dsum, l));
                }
            }
        }
        vector<STRLine> lines;
        if(lines_and_err.size() <= 0) return lines;
        
        std::sort(lines_and_err.begin(), lines_and_err.end(), sortbyfirst<float,STRLine>);
        
        for (int i=0; i<MAX(1,lines_and_err.size()/10); i++) { //take top %10 of best fitted lines, at least one
            lines.push_back(lines_and_err[i].second);
        }
        return lines;
    }
    
    void linesSupportAndRefine(vector<STRLine>& lines, Mat& img) {
        vector<Point2f> ps = candidatePoints;
        vector<pair<int,STRLine > > support(lines.size());
        for (int i=0; i<lines.size(); i++) {
            //look for support in all points
            Characters cs;
            support[i].first = 0;
            for (int p=0; p<ps.size(); p++) {
                if(pointToLineD(ps[p], lines[i].line) < POINT_TO_LINE_THRESH) {
                    cs.push_back(candidateContours[p]);
                    support[i].first++;
                }
            }
            if(support[i].first > 0)
                support[i].second = STRLine(lines[i].line,cs);
        }
        lines.clear();
        sort(support.begin(), support.end(), sortbyfirst<int,STRLine >);
        int cutoff = MAX(MIN_POINTS_FOR_LINE,support[support.size()/5].first);
        
        //fit lines for support points
        for (int i=0; i<support.size(); i++) {
            if (support[i].first == 0 || support[i].first < cutoff) continue;
            Vec4f l;
            fitLine(support[i].second.support, l, CV_DIST_L2, 0, 0.01, 0.01);
            //            lines.push_back(STRLine(CVToMBLine(l),support[i].second.characters));
            lines.push_back(support[i].second);
            lines.back().line = CVToMBLine(l);
        }
    }
    
    void linesBinning(vector<STRLine>& lines, Mat& img) {
        //calc 2D histogram to bin the lines
        int N_bins = 12;
        vector<vector<int> > bins(N_bins,vector<int>(N_bins));
        
        vector<Point2f> lines_mb;
        for (int i=0; i<lines.size(); i++) { lines_mb.push_back(lines[i].line); }
        
        float intercept_min = (*min_element(lines_mb.begin(), lines_mb.end(), sortpointsbyy<float>)).y;
        float intercept_max = (*max_element(lines_mb.begin(), lines_mb.end(), sortpointsbyy<float>)).y;
        float slope_min = (*min_element(lines_mb.begin(), lines_mb.end(), sortpointsbyx<float>)).x;
        float slope_max = (*max_element(lines_mb.begin(), lines_mb.end(), sortpointsbyx<float>)).x;
        float intercept_step = (intercept_max - intercept_min)/(float)N_bins;
        float slope_step = (slope_max - slope_min)/(float)N_bins;
        
        float slope_binning_factor = N_bins / (slope_max - slope_min);
        float intercept_binning_factor = N_bins / (intercept_max - intercept_min);
        for (int i=0; i<lines.size(); i++) {
            Point2f l = lines_mb[i];
            int bin_i = MIN(N_bins-1,floor((l.x - slope_min) * slope_binning_factor));
            int bin_j = MIN(N_bins-1,floor((l.y - intercept_min) * intercept_binning_factor));
            bins[bin_i][bin_j]++;
        }
        lines.clear();
        for (int i=0; i<N_bins; i++) {
            for (int j=0; j<N_bins; j++) {
                if(bins[i][j] > 0) {
                    float slope_f = ((float)i + 0.5f) * slope_step + slope_min;
                    float intercept_f = ((float)j + 0.5f) * intercept_step + intercept_min;
                    lines.push_back(STRLine(Point2f(slope_f,intercept_f),Characters()));
                }
            }
        }
    }
    
    void getCandidatePoints(Mat& img, Mat& adaptive) {
        vector<vector<Point> > contours;
        findContours(adaptive, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
        for (int c=0; c<contours.size(); c++) {
            float ca = (float)contourArea(contours[c]);
            //            cout << ca << endl;
            if(ca > MAX_CONTOUR_AREA || ca < MIN_CONTOUR_AREA) continue;
            Scalar color( 100+(rand()&155), 100+(rand()&155), 100+(rand()&155) );
            //            drawContours( img , contours, c, color, CV_FILLED, 1);
            
            vector<Point> contour = contours[c];
//            std::sort(contour.begin(), contour.end(), sortpointsbyy<int>);
            Point2f bottomp = *std::max_element(contour.begin(), contour.end(), sortpointsbyy<int>);
            
//            Scalar mn = mean(contours[c]);
//            Point2f midp((float)mn[0],(float)mn[1]);
//            Point2f bottomp = contour.back();
            //            circle(img, midp, 3, color, CV_FILLED);
            circle(img, bottomp, 3, color, CV_FILLED);
            candidatePoints.push_back(bottomp);
            candidateContours.push_back(contours[c]);
        }
    }
    
    /**
     * find and prune candidate text lines
     **/
    vector<STRLine> findGoodLines(Mat& img) {
        //find good lines
        vector<STRLine> lines;
        lines = findCandidateLines2(img);
        drawLines(img,lines,Scalar(0,0,255));
        
        if(lines.size() <= 0) return lines;
        
        linesSupportAndRefine(lines,img);
        drawLines(img,lines,Scalar(0,255));
        
        if(lines.size() <= 0) return lines;
        
        linesBinning(lines, img);
        
        if(lines.size() <= 0) return lines;
        
        linesSupportAndRefine(lines,img);
        
        if(lines.size() <= 0) return lines;
        
        linesBinning(lines, img);
        
        if(lines.size() <= 0) return lines;
        
        linesSupportAndRefine(lines,img);
        
//        for (int i=0; i<lines.size(); i++) drawLine(img, lines[i].line, Scalar(0,0,255));
        //        copy(lines.begin(),lines.end(),ostream_iterator<STRLine>(cout," S\n"));
        return lines;
    }
    
    void addCharsToWord(STRWord& w, const Rect& r, const vector<Point2f>& points, const Characters& characters, const Mat& orig, Mat& img) {
        rectangle(img, r, Scalar(255,0,255));
        w.chars.clear();
        for (int i=0; i<points.size(); i++) {
            if (points[i].inside(r)) {
                pair<Point,Mat> p;
                Rect bb = boundingRect(characters[i]);
                p.first = bb.tl();
                orig(bb).copyTo(p.second);
                w.chars.push_back(p);
            }
        }
    }
    
    pair<float,Point> findPatch(Point p, const Mat& patch, const Mat& orig, Rect& searchRect) {
        Mat_<float> res;
        int left_pad = patch.cols*0.66, top_pad = patch.rows/2;
        searchRect = Rect(p - Point(left_pad,top_pad), patch.size() + Size(left_pad*1.5,top_pad*2));
        searchRect &= Rect(Point(0,0),orig.size());
        
        if(searchRect.width < patch.cols || searchRect.height < patch.rows)
            return make_pair(1.0f,p);
        
        Mat roi = orig(searchRect);
        matchTemplate(roi, patch, res, CV_TM_SQDIFF_NORMED);
        Point minp; double minv,maxv;
        minMaxLoc(res,&minv,&maxv,&minp);
        
//        imshow("roi",roi);
//        imshow("template",patch);
//        imshow("res",(res - minv)/(maxv-minv));
        //        waitKey();
        
        return make_pair(res(minp),minp + searchRect.tl());
    }
    
    /**
     * track known words in the new image
     * @return the estimated motion in the image
     **/
    Vec2f trackWords(const Mat& orig, Mat& output_img) {
        //track words
        vector<pair<Point,STRWord> > newTrackedWords;
        Vec2f motion; float motionCount = 0.0f;
        for (int i=0; i<trackedWords.size(); i++) {
            Point lookupPoint = trackedWords[i].first+Point(lastMotion)*0.5;
            Size lookupSize = trackedWords[i].second.patches.back().size();
            
            if(fabsf((lookupPoint.y+lookupSize.height/2)-(focusArea.y+focusArea.height/2)) > 100) continue;
            if(pointToLineD(Point2f(lookupPoint.x + lookupSize.width/2,lookupPoint.y+lookupSize.height/2), trackedLine.line) > POINT_TO_LINE_THRESH*2) continue;

            rectangle(output_img, lookupPoint, lookupPoint+Point(lookupSize), Scalar(0,255));
            
//            int left_pad = lookupSize.width*0.5, top_pad = lookupSize.height/2;
//            Rect rr(lookupPoint - Point(left_pad,top_pad), lookupSize + Size(left_pad+5,top_pad*2));
            
            Rect rr;
            pair<float,Point> score_pt = findPatch(lookupPoint, trackedWords[i].second.patches.back(), orig, rr);
            rectangle(output_img, rr, Scalar(0,255));

            Point minp = score_pt.second;
            float score = score_pt.first;
            //            cout << "\"" << trackedWords[i].second.text.second << "\": " << minp << " " << score << endl;
            
            if (score > 0.0315) {
                trackedWords[i].second.times_seen--;
                
                Mat blurry; cvtColor(orig(Rect(minp,lookupSize)&Rect(0,0,orig.cols,orig.rows)), blurry, CV_BGR2GRAY);
                adaptiveThreshold(blurry, blurry, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 20);
                int white = countNonZero(blurry);
                float black_to_white_ratio = (float)(lookupSize.width*lookupSize.height - white) / (float)white;
//                Mat blurryRGB; cvtColor(blurry, blurryRGB, CV_GRAY2BGR);
//                putText(blurryRGB, SSTR(black_to_white_ratio), Point(10,10), CV_FONT_HERSHEY_PLAIN, 0.8, Scalar(0,0,255));
//                imshow("blurry"+SSTR(i),blurryRGB);
                
                //check if block is too blurry
                if(black_to_white_ratio < 0.25)
                    continue;
            } else
                trackedWords[i].second.times_seen = MIN(TRACKED_WORD_UNSEEN_UPPER_THRESH,trackedWords[i].second.times_seen+1);
            
            //either word was lost or moved too far back
            if(trackedWords[i].second.times_seen < TRACKED_WORD_UNSEEN_THRESH ||
               trackedWords[i].first.x < 20)
            {
                cout << "tracking for word \"" << trackedWords[i].second.text.second << "\" failed\n";
                for (int j=0; j<trackedWords[i].second.chars.size(); j++) {
                    trackedLeftovers.push_back(make_pair<Point,STRLeftover>(
                                               trackedWords[i].second.chars[j].first,
                                               STRLeftover(trackedWords[i].second.chars[j].second)
                                                                            ));
                }
                continue;
            }
            
            motionCount += 1.0f;
            motion += Vec2f(minp.x - trackedWords[i].first.x, minp.y - trackedWords[i].first.y);
            
            trackedWords[i].first = minp;
            Rect r = Rect(minp,trackedWords[i].second.patches.back().size());
            r &= Rect(Point(0,0),orig.size());
            Mat newpatch; orig(r).copyTo(newpatch);
            
            addWeighted(trackedWords[i].second.patches.back(), 0.5, newpatch, 0.5, 0.0, trackedWords[i].second.patches.back());
//            imshow("tracked word "+SSTR(i),trackedWords[i].second.patches.back());
//            trackedWords[i].second.patches.push_back(newpatch);
            
            addCharsToWord(trackedWords[i].second,r,candidatePoints,candidateContours,orig,output_img);
            
            newTrackedWords.push_back(trackedWords[i]);
            
            stringstream strm; strm << trackedWords[i].second.text.second << " ("<< trackedWords[i].second.text.first<<", "<<trackedWords[i].second.times_seen<<")";
            putText(output_img, strm.str(), trackedWords[i].first, CV_FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));
            Point br = trackedWords[i].first;
            br.x += trackedWords[i].second.patches.back().cols;
            br.y += trackedWords[i].second.patches.back().rows;
            rectangle(output_img, trackedWords[i].first, br, Scalar::all(255));
            for (int j=0; j<trackedWords[i].second.chars.size(); j++) {
                rectangle(output_img, Rect(trackedWords[i].second.chars[j].first, trackedWords[i].second.chars[j].second.size()), Scalar::all(155));
            }
        }
        motion[0] /= (float)motionCount; motion[1] /= (float)motionCount;
        trackedWords.clear();
        trackedWords = newTrackedWords;
        return motion;
    }
    
    
    /**
     * track the leftover patches
     **/
    void trackLeftovers(Vec2f motion, const Mat& orig, Mat& img) {
        //track the leftovers
        if(norm(motion) <= 0 || motion[0] != motion[0]) { // || isnan(motion[0])) {
            for (int i=0; i<trackedLeftovers.size(); i++) {
                if(trackedLeftovers[i].first.x+lastMotion[0] < 0) continue;
                if(trackedLeftovers[i].first.y+trackedLeftovers[i].second.patch.cols >= orig.cols-1) continue;
                
                Rect rr;
                pair<float,Point> p = findPatch(trackedLeftovers[i].first+Point(lastMotion[0],lastMotion[1]), trackedLeftovers[i].second.patch, orig, rr);
                if(p.first < 0.02) {
                    lastMotion = Vec2f(p.second.x - trackedLeftovers[i].first.x, p.second.y - trackedLeftovers[i].first.y);
                    break;
                }
            }
        }
        vector<pair<Point,STRLeftover> > newTrackedLeftovers;
        for (int i=0; i<trackedLeftovers.size(); i++) {
            trackedLeftovers[i].first += Point(lastMotion[0],lastMotion[1]);
            if(trackedLeftovers[i].first.x < 0) continue;
            if(trackedLeftovers[i].first.y+trackedLeftovers[i].second.patch.cols >= orig.cols-1) continue;
            
            Rect rr;
            pair<float,Point> p = findPatch(trackedLeftovers[i].first, trackedLeftovers[i].second.patch, orig, rr);
            if(p.first < 0.0315) {
                trackedLeftovers[i].first = p.second;
                trackedLeftovers[i].second.times_seen++;
                addWeighted(orig(Rect(p.second,trackedLeftovers[i].second.patch.size())), 0.5, trackedLeftovers[i].second.patch, 0.5, 0.0, trackedLeftovers[i].second.patch);
                //                    orig(Rect(p.second,trackedLeftovers[i].second.patch.size())).copyTo(trackedLeftovers[i].second.patch);
            } else {
                trackedLeftovers[i].second.times_seen--;
                
                Size lookupSize = trackedLeftovers[i].second.patch.size();
                Mat blurry; cvtColor(orig(Rect(p.second,lookupSize)), blurry, CV_BGR2GRAY);
                adaptiveThreshold(blurry, blurry, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 20);
                int white = countNonZero(blurry);
                float black_to_white_ratio = (float)(lookupSize.width*lookupSize.height - white) / (float)white;
                if(black_to_white_ratio < 0.1) continue;
            }
            
            //after so many frames, loose track of leftover
            if(trackedLeftovers[i].second.times_seen < TRACKED_LEFTOVERS_UNSEEN_THRESH) {
                rectangle(img, Rect(trackedLeftovers[i].first, trackedLeftovers[i].second.patch.size()), Scalar(255,255,0));
                continue;
            }
            
            putText(img, SSTR(trackedLeftovers[i].second.times_seen), trackedLeftovers[i].first, CV_FONT_HERSHEY_PLAIN, 0.5, Scalar(255));
            rectangle(img, Rect(trackedLeftovers[i].first, trackedLeftovers[i].second.patch.size()), Scalar(255,0,0));
            newTrackedLeftovers.push_back(trackedLeftovers[i]);
        }
        trackedLeftovers = newTrackedLeftovers;
    }
    
    TextConf getWordFromContoursAndImage(const Characters& characters, Mat& img, const Mat& orig, Rect& r) {
        vector<Point> allcontours;
        for (int c=0; c<characters.size(); c++)
            copy(characters[c].begin(), characters[c].end(), back_inserter(allcontours));
        Rect r_ = boundingRect(allcontours);
//        r = r_;
        r.y = r_.y - 10;
        if(r.width <= 0) {
            r.width = r_.width + 10;
            r.x = r_.x - 5;
        }
        r.height = r_.height + 20;
        r &= Rect(0,0,img.cols,img.rows);
        rectangle(img, r, Scalar(0,0,255));
        
        TextConf res = TB.process(orig, r);
        //        r = r_;
        return res;
    }
    
    int findNewWords(const Mat& orig, Mat& img) {
        //find rightmost tracked Word
        int leftCutoff = -1;
        for (int j=0; j<trackedWords.size(); j++) {
            int rightEndOfWord = trackedWords[j].first.x + trackedWords[j].second.patches.back().cols;
            if(rightEndOfWord > leftCutoff) {
                leftCutoff = rightEndOfWord;
            }
        }
        for (int j=0; j<trackedLeftovers.size(); j++) {
            int rightEndOfWord = trackedLeftovers[j].first.x + trackedLeftovers[j].second.patch.cols;
            if(rightEndOfWord > leftCutoff) {
                leftCutoff = rightEndOfWord;
            }
        }
        line(img, Point(leftCutoff,0), Point(leftCutoff,img.rows), Scalar(0,0,255));
        leftCutoff += 5;
        
        //only continue with OCR if cutoff point approaches middle of image
        if(leftCutoff > img.cols*NEW_WORD_CUTOFF_POSITION_FACTOR) return 2;
        
        //find candidate points that are not part of known words
        vector<Point2f> prunedPoints; Characters prunedCharacters;
        for (int i=0; i<candidatePoints.size(); i++) {
            bool outside = candidatePoints[i].x > leftCutoff;
            //            for (int j=0; j<trackedWords.size(); j++) {
            //                if(candidatePoints[i].inside(Rect(trackedWords[j].first-Point(5,5),trackedWords[j].second.patches.back().size()+Size(10,10)))) {
            //                    outside = false;
            //                }
            //            }
            if(outside) {
                prunedPoints.push_back(candidatePoints[i]);
                prunedCharacters.push_back(candidateContours[i]);
            }
        }
        for (int i=0; i<prunedPoints.size(); i++) {
            circle(img, prunedPoints[i], 5, Scalar::all(255));
        }
        
        if(prunedPoints.size() >= 2) {
            Rect r; r.x = leftCutoff; r.width = img.cols - leftCutoff;
            TextConf txt = getWordFromContoursAndImage(prunedCharacters, img, orig, r);
            stringstream strm; strm << txt.second << " ("<<txt.first<<")";
            putText(img, strm.str(), r.tl(), CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
            
            if(txt.first > NEW_WORD_CONF_THRESH && txt.second.size() > 0) {
                rectangle(img, r, Scalar(0,255,0));
                trackedWords.push_back(make_pair(r.tl(),STRWord(orig(r),txt)));
                cout << txt.second << endl;
                addCharsToWord(trackedWords.back().second, r, prunedPoints, prunedCharacters, orig,img);
                
                newWordFound(txt.second);
            }
        }
        return prunedPoints.size();
    }
    
    void prunePointsBasedOnTrackedLine() {
        //prune points based on known line
        vector<Point2f> prunedPoints; Characters prunedCharacters;
        for (int i=0; i<candidatePoints.size(); i++) {
            if(pointToLineD(candidatePoints[i], trackedLine.line) < POINT_TO_LINE_THRESH * 2) {
                prunedPoints.push_back(candidatePoints[i]);
                prunedCharacters.push_back(candidateContours[i]);
            }
        }
        candidatePoints = prunedPoints;
        candidateContours = prunedCharacters;
    }
    
public:
    SequentialTextReader():
    frameNum(0),
    ADAPTIVE_THRESH(101),
    NEW_WORD_CUTOFF_POSITION_FACTOR(0.6),
    myHandler(NULL),
    focusArea(Rect(0,0,0,0))
    {
        TB.init();
    }
    
    class Handler {
        SequentialTextReader* mySTR;
    public:
        void setSTR(SequentialTextReader* str) { mySTR = str;}
        virtual void newWordFound(std::string str) = 0;
        virtual void endOfLine() = 0;
        virtual void textFound() = 0;
        virtual void escapeUp() = 0;
        virtual void escapeDown() = 0;
        virtual void escapeDistance(int) = 0;
        void setThresh(int t) {mySTR->setThresh(t);}
        void processImage(Mat& img) {mySTR->processImage(img);}
    };
private:
    Handler* myHandler;
public:
    
    void setHandler(Handler& h) {myHandler = &h; myHandler->setSTR(this);}
    void setHandler(Handler* h) {myHandler = h; myHandler->setSTR(this);}
    void newWordFound(const std::string& str) {if(myHandler) myHandler->newWordFound(str);};
    void endOfLine() {if(myHandler) myHandler->endOfLine();};
    void textFound() {if(myHandler) myHandler->textFound();};
    void escapeUp() {if(myHandler) myHandler->escapeUp();};
    void escapeDown() {if(myHandler) myHandler->escapeDown();};
    void escapeDistance(int d) {if(myHandler) myHandler->escapeDistance(d);};
    
    void setThresh(int t) {
        ADAPTIVE_THRESH = t - (t%2) + 1;
    }
    
    void setFocusLocation(int y) { focusArea.y = y; origFocusArea.y = y; }
    void setFocusSize(int s) { focusArea.height = s; origFocusArea.height = s; }
    const Rect& getFocusArea() { return origFocusArea; }
    
    void reset() {
        foundFirstWord = false;
        candidatePoints.clear();
        candidateContours.clear();
        trackedWords.clear();
        trackedLeftovers.clear();
        lastMotion = Vec2f(0,0);
        focusArea = origFocusArea;
        trackedLine.line = Point2f(0,0);
    }
    
    Mat adaptiveForGUI;
    
    void processImage(Mat& img) {
        candidatePoints.clear();
        candidateContours.clear();
        
        Mat orig; img.copyTo(orig);
        Mat grey; cvtColor(img, grey, CV_BGR2GRAY);
        Mat adaptive_;
        adaptiveThreshold(grey, adaptive_, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, ADAPTIVE_THRESH, 30);
        adaptive_.copyTo(adaptiveForGUI);
        
//        cvtColor(adaptive_, img, CV_GRAY2BGR);
//        threshold(grey, adaptive_, 128, 255, THRESH_BINARY + THRESH_OTSU);
//        imshow("adaptive",adaptive_);
        
        Mat adaptive = ~adaptive_;
        
        if(focusArea.width <= 0) {
            focusArea.x = 0;
            focusArea.height = img.rows / 6.0;
            focusArea.y = img.rows/2 - focusArea.height/2;
            focusArea.width = img.cols;
            
            origFocusArea = focusArea;
        }
        
        if (!foundFirstWord) {
            getCandidatePoints(img, adaptive);
            
            vector<STRLine> lines;
            lines = findGoodLines(img);
            
            vector<pair<float,STRLine> > lines_and_distanceToFocus;
            int midy = origFocusArea.y+origFocusArea.height/2;
            for(int i=0;i<lines.size();i++) {
                lines_and_distanceToFocus.push_back(make_pair(fabsf((2*lines[i].line.y + lines[i].line.x*img.cols)/2 - midy),lines[i]));
            }
            sort(lines_and_distanceToFocus.begin(), lines_and_distanceToFocus.end(), sortbyfirst<float,STRLine>);
            
            for (int i=0; i<lines_and_distanceToFocus.size(); i++) {
                STRLine ln = lines_and_distanceToFocus[i].second;
                putText(img, SSTR(lines_and_distanceToFocus[i].first), Point(5,ln.line.y), CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));
                
                Rect r;
                TextConf txt = getWordFromContoursAndImage(ln.characters, img, orig, r);
                
                if(txt.first > NEW_WORD_CONF_THRESH && txt.second.length() > 0) {
                    stringstream strm; strm << txt.second << " ("<<txt.first<<")";
                    putText(img, strm.str(), r.tl(), CV_FONT_HERSHEY_PLAIN, 1.0, Scalar(0,0,255));

                    rectangle(img, r, Scalar(0,255,0));
                    trackedWords.push_back(make_pair(r.tl(),STRWord(orig(r),txt)));
                    cout << txt.second << endl;
                    addCharsToWord(trackedWords.back().second, r, candidatePoints, candidateContours, orig, img);
                    newWordFound(txt.second);
                    
                    trackedLine = ln;
                    
                    foundFirstWord = true;
                    
                    textFound();
                    break;
                }
                 
            }
        } else {
            CV_PROFILE(getCandidatePoints(img, adaptive);)
            
//            putText(img, "candidate points: " + SSTR(candidatePoints.size()), Point(10,10), CV_FONT_HERSHEY_PLAIN, 1.0, Scalar::all(255));
            
            drawLine(img, trackedLine.line, Scalar(0,255),2);
            
            CV_PROFILE(prunePointsBasedOnTrackedLine();)
            
            vector<STRLine> lines;
            CV_PROFILE(lines = findGoodLines(img);)
            if(lines.size() > 0) {
                trackedLine = lines[0];
            }
            focusArea.height = MIN(focusArea.height+1,img.rows/3);
            focusArea.y = (trackedLine.line.y + trackedLine.line.x*img.cols + trackedLine.line.y)/2 - focusArea.height/2;
            
//            origFocusArea.y = 0.95*(origFocusArea.y + origFocusArea.height/2) + 0.05*(focusArea.y+focusArea.height/2) - origFocusArea.height/2;
            
            //TODO: see if line is escaping up or down, or by distance
                        
            Vec2f motion;
            CV_PROFILE(motion = trackWords(orig,img);)
            
            /*
            for(int i=0;i<trackedWords.size();i++)
            {
                if(!trackedWords[i].second.sent_to_tts && trackedWords[i].second.times_seen >= TRACKED_WORD_UNSEEN_UPPER_THRESH)
                {
                    newWordFound(trackedWords[i].second.text.second);
                    trackedWords[i].second.sent_to_tts = true;
                }
            }
             */
            
            if(norm(motion) > 0)
                lastMotion = motion;
            
            CV_PROFILE(trackLeftovers(motion,orig,img);)
            
            int num_chars_end_of_line = 0;
            CV_PROFILE(num_chars_end_of_line = findNewWords(orig, img);)
            
            int trackedLineMidpY = (trackedLine.line.y + trackedLine.line.x*img.cols + trackedLine.line.y)/2;
            int origFocusAreaMidpY = origFocusArea.y + origFocusArea.height/2;
            
            //go back to seek mode...
            if((trackedWords.size() == 0 && trackedLeftovers.size() == 0) || fabsf(trackedLineMidpY-origFocusAreaMidpY) > 80) {
                endOfLine();
                foundFirstWord = false;
                focusArea = origFocusArea;
            }
        }
        
        
//        imshow("img",img);
        
        frameNum++;
    }
    
};

#endif /* defined(__testopencv__SeqentialTextReader__) */
