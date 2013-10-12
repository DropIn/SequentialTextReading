//
//  FingertipDetector.h
//  SequentialTextReader
//
//  Created by roy_shilkrot on 10/10/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#ifndef _FINGERTIPDETECTOR_H
#define _FINGERTIPDETECTOR_H

#include "std.h"

#include "AbstractAlgorithm.h"
#include "MathGLTools.h"

#define TwoPi 6.28318530718

#pragma mark Curves Utilities
template<typename T, typename V>
void PolyLineSplit(const vector<Point_<T> >& pl,vector<V>& contourx, vector<V>& contoury) {
	contourx.resize(pl.size());
	contoury.resize(pl.size());
	
	for (int j=0; j<pl.size(); j++)
	{
		contourx[j] = (V)(pl[j].x);
		contoury[j] = (V)(pl[j].y);
	}
}

template<typename T, typename V>
void PolyLineMerge(vector<Point_<T> >& pl, const vector<V>& contourx, const vector<V>& contoury) {
	assert(contourx.size()==contoury.size());
	pl.resize(contourx.size());
	for (int j=0; j<contourx.size(); j++) {
		pl[j].x = (T)(contourx[j]);
		pl[j].y = (T)(contoury[j]);
	}
}

template<typename T, typename V>
void ConvertCurve(const vector<Point_<T> >& curve, vector<Point_<V> >& output) {
	output.clear();
	for (int j=0; j<curve.size(); j++) {
		output.push_back(Point_<V>(curve[j].x,curve[j].y));
	}
}

template<typename T>
void drawOpenCurve(Mat& img, const vector<Point_<T> >& curve, Scalar color, int thickness) {
	if (curve.size() <= 0) {
		return;
	}
	vector<cv::Point> curve2i;
	ConvertCurve(curve, curve2i);
	for (int i=0; i<curve2i.size()-1; i++) {
		line(img, curve2i[i], curve2i[i+1], color, thickness);
	}
}

#pragma mark Gaussian Smoothing and Curvature


void ComputeCurveCSS(const vector<double>& curvex,
					 const vector<double>& curvey,
					 vector<double>& kappa,
					 vector<double>& smoothX, vector<double>& smoothY,
					 double sigma,
					 bool isOpen
					 );

template<typename T>
void ComputeCurveCSS(const vector<Point_<T> >& curve,
					 vector<double>& kappa,
					 vector<Point_<T> >& smooth,
					 double sigma,
					 bool isOpen = false
					 )
{
	vector<double> contourx(curve.size()),contoury(curve.size());
	PolyLineSplit(curve, contourx, contoury);
	
	vector<double> smoothx, smoothy;
	ComputeCurveCSS(contourx, contoury, kappa, smoothx, smoothy, sigma, isOpen);
	
	PolyLineMerge(smooth, smoothx, smoothy);
}

vector<int> FindCSSInterestPoints(const vector<double>& kappa);

template<typename T, typename V>
void GetCurveSegments(const vector<Point_<T> >& curve, const vector<int>& interestPoints, vector<vector<Point_<V> > >& segments, bool closedCurve = true) {
	if (closedCurve) {
		segments.resize(interestPoints.size());
	} else {
		segments.resize(interestPoints.size()+1);
	}
    
	for (int i = (closedCurve)?0:1; i<segments.size()-1; i++) {
		int intpt_idx = (closedCurve)?i:i-1;
		segments[i].clear();
		for (int j=interestPoints[intpt_idx]; j<interestPoints[intpt_idx+1]; j++) {
			segments[i].push_back(Point_<V>(curve[j].x,curve[j].y));
		}
	}
	if (closedCurve) {
		//put in the segment that passes the 0th point
		segments.back().clear();
		for (int j=interestPoints.back(); j<curve.size(); j++) {
			segments.back().push_back(Point_<V>(curve[j].x,curve[j].y));
		}
		for (int j=0; j<interestPoints[0]; j++) {
			segments.back().push_back(Point_<V>(curve[j].x,curve[j].y));
		}
	} else {
		//put in the segment after the last point
		segments.back().clear();
		for (int j=interestPoints.back(); j<curve.size(); j++) {
			segments.back().push_back(Point_<V>(curve[j].x,curve[j].y));
		}
		//put in the segment before the 1st point
		segments.front().clear();
		for (int j=0; j<interestPoints[0]; j++) {
			segments.front().push_back(Point_<V>(curve[j].x,curve[j].y));
		}
	}
}


#pragma mark Finger Detector

struct FingertipResult {
	Point2f p;
	float probability;
    RotatedRect rr;
};

class FingertipDetector : public AbstractAlgorithm {
public:
    FingertipDetector() {
        last.probability = 0;
    }
    
	void train() {}
    
	void detect() {
        //search around last, using latest appearence
    }
    
    /**
     * Find fingertip candidate without prior knowledge of location
     * @param image to work on
     **/
	void bootstrap(Mat& img) {
        Mat edges;
        Canny(img, edges, 100, 100);
        vector<vector<Point> > contours;
        findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
        vector<vector<Point> > contours_long;
        for (int i=0; i<contours.size(); i++) {
            if (contours[i].size()>150) {
                contours_long.push_back(contours[i]);
            }
        }
        contours = contours_long;
        
        vector<RotatedRect> candidates;

        for (int i=0; i<contours.size(); i++) {
            vector<double> kappa; vector<Point> smooth;
            ComputeCurveCSS(contours[i], kappa, smooth, 11.0);
            //            ShowMathGLData(kappa);
            
            //            Mat tmp; img.copyTo(tmp);
            //            drawOpenCurve(tmp, contours[i], Scalar(255), 1);
            //            drawOpenCurve(tmp, smooth, Scalar(0,255), 1);
            
            vector<int> ips = FindCSSInterestPoints(kappa);
//            for (int k=0; k<ips.size(); k++) {
//                circle(img, contours[i][ips[k]], 3, Scalar(0,0,255),CV_FILLED);
//            }
            vector<vector<Point> > segs;
            if(ips.size()>0)
                GetCurveSegments(contours[i], ips, segs);
            else
                segs.push_back(contours[i]); //just put whole curve
            
            for (int k=0; k<segs.size(); k++) {
                if(segs[k].size() < 30) continue;
                
//                drawOpenCurve(img, segs[k], Scalar::all(255), 1);
                
                RotatedRect box = fitEllipse(segs[k]);
                
                float r = box.size.width/box.size.height;
                if(fabsf(r-1.0) > 0.23) continue;
                int a = box.boundingRect().area();
                if(a < 25000 || a > 40000) continue;
                
                Point2f vtx[4]; box.points(vtx);
                putText(img, SSTR(r)/* + "," + SSTR(a)*/, vtx[0], CV_FONT_NORMAL, 0.5, Scalar::all(255));
                ellipse(img, box, Scalar(255), 1);
//                for( int j = 0; j < 4; j++ )
//                    line(img, vtx[j], vtx[(j+1)%4], Scalar(0,255,0), 1);
                
                candidates.push_back(box);
                last.p = *min_element(segs[k].begin(), segs[k].end(), sortpointsbyy<int>);
                last.probability = 1;
                last.rr = box;
            }
        }
        
        //TODO bin candidates, select best one
	}
    
    
    FingertipResult processImage(Mat& img) {
        if(last.probability < 0.5)
            bootstrap(img);
        else {
            //TODO add another modality, like template matching
            
            last.probability *= 0.9;
            Rect r = last.rr.boundingRect();
            r.x -= 50; r.y -= 20; r.width += 100; r.height -= 50;
            r = r & Rect(0,0,img.cols,img.rows);
            
            if(r.width < 30 || r.height < 30) { last.probability = 0; return last; }
            
            Mat tmp = img(r);
            bootstrap(tmp);
            
            if(last.probability == 1.0) {
                last.p += Point2f(r.x,r.y);
                last.rr.center += Point2f(r.x,r.y);
            }
            rectangle(img, r, Scalar(0,0,255));
        }
        
        return last;
    }

private:
    FingertipResult last;
};


#endif
