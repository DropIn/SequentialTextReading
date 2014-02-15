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

#include <deque>

#include "AbstractAlgorithm.h"
#include "MathGLTools.h"

#ifndef CV_PROFILE
#define CV_PROFILE(code)	\
{\
std::cout << #code << " ";\
double __time_in_ticks = (double)cv::getTickCount();\
{ code }\
std::cout << "DONE " << ((double)cv::getTickCount() - __time_in_ticks)/cv::getTickFrequency() << "s" << std::endl;\
}
#endif

#ifdef CV_PROFILE
#define CV_PROFILE(code) code
#endif


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

template<typename T>
float CurveLength(const vector<Point_<T> >& curve) {
    float sum = 0.0f;
    for (int i=0; i<curve.size()-1; i++) {
        sum += norm(curve[i]-curve[i+1]);
    }
    return sum;
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

        gaussianKernel = cv::getGaussianKernel(5, 2.0, CV_32F);
		std::cout << gaussianKernel << std::endl;

		bTraining = false;
    }
    
	void train() {bTraining = true; trainingSamples.clear();}
    
	void detect() {
        //search around last, using latest appearence
    }

    /**
     * analyze the bottom of the image to find a possible area where the finger is
     * @param img Image to work on
     * @return the cutoff area
     **/
    Rect bottomLineAnalysis(Mat& _img) {
        // Look for a clear horizontal cutoff point where the finger enters the image
    	Mat img;
//    	_img.copyTo(img);
    	GaussianBlur(_img,img,Size(25,25),11.0,11.0);
        
        int yoffset = 100;
    	int xoffset = 50;
        int bandsize = img.rows / 5;
        Mat btmLine; reduce(img(Rect(xoffset,img.rows-bandsize-yoffset,img.cols-xoffset*2,bandsize)), btmLine, 0, CV_REDUCE_AVG);
//        repeat(btmLine, bandsize, 1, img(Rect(0,img.rows-bandsize,img.cols,bandsize)));
        
//        Mat btmLine; img.row(img.rows-1).copyTo(btmLine);
        Mat tmp; btmLine.convertTo(tmp, CV_32FC3, 1.0/255.0);
        Mat samples = Mat(tmp.t()).reshape(1);
        Mat lables_;
        kmeans(samples, 2, lables_, TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS, 50, 0.1), 1, KMEANS_RANDOM_CENTERS);
        Mat_<uchar> lables; lables_.convertTo(lables, CV_8UC1);
//        imshow("first line",lables*255.0/3.0);
//        cout << lables;
        
        int window = img.cols/4; window += (window % 2) == 0 ? 1 : 0;
        int twowindow = window * 2;
        medianBlur(lables, lables, window);
//        imshow("first line median",lables*255.0/3.0);
//        cout << lables;
        
        Rect possibleArea;
        
        for (int i=0; i<lables.rows; i++) {
            _img.at<Vec3b>(img.rows-1,i+xoffset) =   (lables(i) == 0) ? Vec3b(255,0,0) :
                                            (lables(i) == 1) ? Vec3b(0,255,0) :
                                            Vec3b(0,0,255);
            
            if(i>50 && i<img.cols-50) {
                if(lables(i) != lables(i-1)) {
//                    line(img, Point(i-1,0), Point(i-1,img.rows), Scalar(255));
//                    line(img, Point(i+twowindow+1,0), Point(i+twowindow+1,img.rows), Scalar(0,255));
                    possibleArea = Rect(i+xoffset, 0, twowindow,img.rows) & Rect(0,0,img.cols,img.rows);
                    break;
                }
            }
        }
        if(possibleArea.x <= 0 || possibleArea.width <=0 || possibleArea.height <= 0)
            return Rect(0,0,0,0);
        
        
        // Now look for a vertical cutoff point
        
        bandsize = img.cols/6;
        int midx = (possibleArea.x+possibleArea.x+possibleArea.width)/2;
//        line(img,Point(midx,0),Point(midx,img.rows),Scalar(0,255));
        reduce(img(Rect(midx-bandsize/2, yoffset, bandsize, img.rows-2*yoffset) & Rect(0,0,img.cols,img.rows)), btmLine, 1, CV_REDUCE_AVG);
        btmLine.convertTo(tmp, CV_32FC3, 1.0/255.0);
        samples = tmp.reshape(1);
        kmeans(samples, 2, lables_, TermCriteria(TermCriteria::MAX_ITER+TermCriteria::EPS, 50, 0.1), 1, KMEANS_RANDOM_CENTERS);
        lables_.convertTo(lables, CV_8UC1);
        medianBlur(lables, lables, window);
        
        for (int i=0; i<lables.rows; i++) {
            if(i>50 && i<img.rows-50) {
                if(lables(i) != lables(i-1)) {
                	_img.at<Vec3b>(0,i+yoffset) =   (lables(i) == 0) ? Vec3b(255,0,0) :
													(lables(i) == 1) ? Vec3b(0,255,0) :
													Vec3b(0,0,255);
//                    line(img, Point(0,i-window/2-1), Point(img.cols,i-window/2-1), Scalar(255));
//                    line(img, Point(0,i+window/2+1), Point(img.cols,i+window/2+1), Scalar(0,255));
                    possibleArea &= Rect(0, i-window/2 + yoffset, img.cols, window);
                    break;
                }
            }
        }
        
//        waitKey();
        return possibleArea & Rect(0,0,img.cols,img.rows);
    }
    
    vector<float> ratios;
    vector<float> areas;
    
    /**
     * Find fingertip candidate without prior knowledge of location
     * @param image to work on
     **/
	void bootstrap(Mat& img) {
        
        Mat edges;
        CV_PROFILE(Canny(img, edges, 100, 100);)
        vector<vector<Point> > contours;
        CV_PROFILE(findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);)

        vector<RotatedRect> candidates;

        for (int i=0; i<contours.size(); i++) {
//            drawOpenCurve(img, contours[i], Scalar::all(255), 1);
            if (contours[i].size() < 15 || CurveLength(contours[i]) < 100) continue;
            
            vector<double> kappa; vector<Point> smooth;

//            CV_PROFILE(approxPolyDP(contours[i], contours[i], 0.5, false);)
            CV_PROFILE(ComputeCurveCSS(contours[i], kappa, smooth, 3.0);)
            //            ShowMathGLData(kappa);
            contours[i] = smooth;
            
//            Mat tmp; img.copyTo(tmp);
//            drawOpenCurve(tmp, contours[i], Scalar(255), 1);
//            drawOpenCurve(img, smooth, Scalar(0,255), 1);
            
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
                if(segs[k].size() < 5) continue;
                if(CurveLength(segs[k]) < 100) continue;
                                
//                drawOpenCurve(img, segs[k], Scalar::all(255), 1);
                
                RotatedRect box = fitEllipse(segs[k]);
//                ellipse(img, box, Scalar(0,255), 1);

                float r = box.size.width/box.size.height;
                int a = box.boundingRect().area();

                Point2f vtx[4]; box.points(vtx);
//                putText(img, SSTR(r), vtx[0], CV_FONT_NORMAL, 0.5, Scalar::all(255));
//                putText(img, SSTR(a), vtx[0]+Point2f(0,10), CV_FONT_NORMAL, 0.5, Scalar::all(255));
                
                if(fabsf(r-0.81) > 0.15) continue;
                if(fabsf(a - 26000) > 5000) continue;
                
//                ratios.push_back(r);
//                areas.push_back(a);
//                ShowMathGLData(ratios,NULL,"ratios",true);
//                ShowMathGLData(areas,NULL,"areas",true);
                
//                ellipse(img, box, Scalar(255), 1);
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
    	FingertipResult fr;
        if(bTraining) {

        static const int median_filter_size = 20;
//        Mat img; resize(img_, img, Size(),0.5,0.5); //work on a half-res
//        if(last.probability < 0.5) {
            Rect cutoff = bottomLineAnalysis(img);

            centers.push_back((cutoff.br()+cutoff.tl())*0.5);
            if(centers.size()>median_filter_size) centers.pop_front();

//            for (int i = 0; i < centers.size(); ++i) {
//            	circle(img,centers[i],3,Scalar(0,255,255),1);
//			}
            Point medianC = centers.back();
            if(centers.size() > median_filter_size) {
            	set<int> cx,cy;
				for (int i = 0; i < centers.size(); ++i) {
					cx.insert(centers[i].x); cy.insert(centers[i].y);
				}
				vector<int> cxv,cyv;
				std::copy(cx.begin(),cx.end(),std::back_inserter(cxv));
				std::copy(cy.begin(),cy.end(),std::back_inserter(cyv));
				medianC = Point(*(cxv.begin()+(cxv.size()/2)),*(cyv.begin()+(cyv.size()/2)));
				medians.push_back(medianC);
				if(medians.size()>median_filter_size/2) {
					while(medians.size()>median_filter_size/2) medians.pop_front();

					float rezx = 0,rezy = 0;
					for (int i = 0; i < medians.size(); ++i) {
						rezx += (float)(medians[i].x) * gaussianKernel(i);
						rezy += (float)(medians[i].y) * gaussianKernel(i);
					}
					medianC.x = rezx;
					medianC.y = rezy;
				}

//				circle(img,medianC,3,Scalar(0,255,255),1);

				cutoff.x = medianC.x - 50;
				cutoff.y = medianC.y;
				cutoff.width = cutoff.height = 100;
            }

            if(cutoff.x > 0 && cutoff.width > 0 && cutoff.height > 0) {
				last.rr = RotatedRect(medianC,cutoff.size(),0.0f);
				last.p = medianC;
				last.probability = 1.0;

//				rectangle(img, cutoff, Scalar(0,0,255));
//	            putText(img, SSTR(last.probability), cutoff.tl()-Point(0,15), CV_FONT_NORMAL, 1.0, Scalar::all(255));
            } else {
            	last.probability = 0;
            }

            fr = last;


            /*
            Mat tmp;
            if(cutoff.x > 0 && cutoff.width > 0 && cutoff.height > 0)
                tmp = img(cutoff);
            else
                tmp = img;
            
            bootstrap(tmp);
            
            if(cutoff.x > 0 && cutoff.width > 0 && cutoff.height > 0) {
                if(last.probability == 1.0) {
                    last.p += Point2f(cutoff.x,cutoff.y);
                    last.rr.center += Point2f(cutoff.x,cutoff.y);
                } else {
                    last.probability = 0.75;
                    last.p = (cutoff.tl()+cutoff.br())*0.5;
                }
            }

        } else {
            //TODO add another modality, like template matching, curve tracking, histogram based
            
            last.probability *= 0.9;
            Rect r = last.rr.boundingRect();
            r.x -= 25; r.y -= 15; r.width += 50; r.height -= 40;
            r = r & Rect(0,0,img.cols,img.rows);
            
            if(r.width < 30 || r.height < 30) { last.probability = 0; return last; }
            
            Mat tmp = img(r);
            bootstrap(tmp);
            
            if(last.probability == 1.0) {
                last.p += Point2f(r.x,r.y);
                last.rr.center += Point2f(r.x,r.y);
            }
//            rectangle(img, r, Scalar(0,0,255));
//            putText(img, SSTR(last.probability), r.tl()-Point(0,15), CV_FONT_NORMAL, 1.0, Scalar::all(255));
        }
        */

         /*
        resize(img, img_, Size(), 2.0, 2.0);
        
        RotatedRect rr_ = last.rr;
        rr_.center.x *= 2.0;
        rr_.center.y *= 2.0;
        rr_.size.width *= 2.0;
        rr_.size.height *= 2.0;
        
        fr.p = Point2f(last.p.x*2.0,last.p.y*2.0);
        fr.rr = rr_;
        fr.probability = last.probability;
          */
        	trainingSamples.push_back(fr.rr.boundingRect());
        	if(trainingSamples.size() > 100) {
        		bTraining = false;

                Rect cutoff_;
                std::vector<Point> trainingcenters;

                for (int i = 0; i < trainingSamples.size(); ++i) {
                    trainingcenters.push_back((trainingSamples[i].br()+trainingSamples[i].tl())*0.5);
    			}

                Point medianC_ = trainingcenters.back();
    			set<int> cx_,cy_;
    			for (int i = 0; i < trainingcenters.size(); ++i) {
    				cx_.insert(trainingcenters[i].x); cy_.insert(trainingcenters[i].y);
    			}
    			vector<int> cxv_,cyv_;
    			std::copy(cx_.begin(),cx_.end(),std::back_inserter(cxv_));
    			std::copy(cy_.begin(),cy_.end(),std::back_inserter(cyv_));
    			medianC_ = Point(*(cxv_.begin()+(cxv_.size()/2)),*(cyv_.begin()+(cyv_.size()/2)));
    //			medians.push_back(medianC);
    //			if(medians.size()>median_filter_size/2) {
    //				while(medians.size()>median_filter_size/2) medians.pop_front();
    //
    //				float rezx = 0,rezy = 0;
    //				for (int i = 0; i < medians.size(); ++i) {
    //					rezx += (float)(medians[i].x) * gaussianKernel(i);
    //					rezy += (float)(medians[i].y) * gaussianKernel(i);
    //				}
    //				medianC.x = rezx;
    //				medianC.y = rezy;
    //			}

    //				circle(img,medianC,3,Scalar(0,255,255),1);

    			cutoff_.x = medianC_.x - 50;
    			cutoff_.y = medianC_.y;
    			cutoff_.width = cutoff_.height = 100;

    			trainedRR = RotatedRect(medianC_,cutoff_.size(),0.0f);
        	}
        } else {
			last.rr = trainedRR;
			last.p = trainedRR.center;
			last.probability = 1.0;
        }

        ellipse(img, fr.rr, Scalar(255), 1);
        return fr;
    }

private:
    FingertipResult 			last;
    std::deque<Point> 			centers,medians;
    cv::Mat_<float>				gaussianKernel;
    bool						bTraining;

    std::vector<cv::Rect> 		trainingSamples;
    cv::RotatedRect				trainedRR;
};


#endif
