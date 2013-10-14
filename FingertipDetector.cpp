//
//  FingertipDetector.cpp
//  TextReading
//
//  Created by roy_shilkrot on 10/10/13.
//
//

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;

#include "FingertipDetector.h"

/* 1st and 2nd derivative of 1D gaussian
 */
void getGaussianDerivs(double sigma, int M, vector<double>& gaussian, vector<double>& dg, vector<double>& d2g) {
    //	static double sqrt_two_pi = sqrt(TwoPi);
	int L;
	if (sigma < 0) {
		M = 1;
		L = 0;
		dg.resize(M); d2g.resize(M); gaussian.resize(M);
		gaussian[0] = dg[0] = d2g[0] = 1.0;
		return;
	}
    
	L = (M - 1) / 2;
	dg.resize(M); d2g.resize(M); gaussian.resize(M);
	getGaussianKernel(M, sigma, CV_64F).copyTo(Mat(gaussian));
    
	double sigma_sq = sigma * sigma;
	double sigma_quad = sigma_sq*sigma_sq;
	for (double i = -L; i < L+1.0; i += 1.0) {
		int idx = (int)(i+L);
		// from http://www.cedar.buffalo.edu/~srihari/CSE555/Normal2.pdf
		dg[idx] = (-i/sigma_sq) * gaussian[idx];
		d2g[idx] = (-sigma_sq + i*i)/sigma_quad * gaussian[idx];
	}
}

/* 1st and 2nd derivative of smoothed curve point */
void getdX(vector<double> x,
		   int n,
		   double sigma,
		   double& gx,
		   double& dgx,
		   double& d2gx,
		   const vector<double>& g,
		   const vector<double>& dg,
		   const vector<double>& d2g,
		   bool isOpen = false)
{
	int L = (g.size() - 1) / 2;
    
	gx = dgx = d2gx = 0.0;
    //	cout << "Point " << n << ": ";
	for (int k = -L; k < L+1; k++) {
		double x_n_k;
		if (n-k < 0) {
			if (isOpen) {
				//open curve -
				//mirror values on border
                //				x_n_k = x[-(n-k)];
				//stretch values on border
				x_n_k = x.front();
			} else {
				//closed curve - take values from end of curve
				x_n_k = x[x.size()+(n-k)];
			}
		} else if(n-k > x.size()-1) {
			if (isOpen) {
				//mirror value on border
                //				x_n_k = x[n+k];
				//stretch value on border
				x_n_k = x.back();
			} else {
				x_n_k = x[(n-k)-(x.size())];
			}
		} else {
            //			cout << n-k;
			x_n_k = x[n-k];
		}
        //		cout << "* g[" << g[k + L] << "], ";
        
		gx += x_n_k * g[k + L]; //gaussians go [0 -> M-1]
		dgx += x_n_k * dg[k + L];
		d2gx += x_n_k * d2g[k + L];
	}
    //	cout << endl;
}


/* 0th, 1st and 2nd derivatives of whole smoothed curve */
void getdXcurve(vector<double> x,
				double sigma,
				vector<double>& gx,
				vector<double>& dx,
				vector<double>& d2x,
				const vector<double>& g,
				const vector<double>& dg,
				const vector<double>& d2g,
				bool isOpen = false)
{
	gx.resize(x.size());
	dx.resize(x.size());
	d2x.resize(x.size());
	for (int i=0; i<x.size(); i++) {
		double gausx,dgx,d2gx; getdX(x,i,sigma,gausx,dgx,d2gx,g,dg,d2g,isOpen);
        if(gausx!=gausx) gausx = 0;
        if(dgx!=dgx) dgx = 0;
        if(d2gx!=d2gx) d2gx = 0;
        
		gx[i] = gausx;
		dx[i] = dgx;
		d2x[i] = d2gx;
	}
}

#pragma mark CSS Image

void SmoothCurve(const vector<double>& curvex,
				 const vector<double>& curvey,
				 vector<double>& smoothX,
				 vector<double>& smoothY,
				 vector<double>& X,
				 vector<double>& XX,
				 vector<double>& Y,
				 vector<double>& YY,
				 double sigma,
				 bool isOpen)
{
	int M = round((10.0*sigma+1.0) / 2.0) * 2 - 1;
    //	assert(M % 2 == 1); //M is an odd number
	
	vector<double> g,dg,d2g;
	getGaussianDerivs(sigma,M,g,dg,d2g);
	
	
	getdXcurve(curvex,sigma,smoothX,X,XX,g,dg,d2g,isOpen);
	getdXcurve(curvey,sigma,smoothY,Y,YY,g,dg,d2g,isOpen);
}

/* compute curvature of curve after gaussian smoothing
 from "Shape similarity retrieval under affine transforms", Mokhtarian & Abbasi 2002
 curvex - x position of points
 curvey - y position of points
 kappa - curvature coeff for each point
 sigma - gaussian sigma
 */
void ComputeCurveCSS(const vector<double>& curvex,
					 const vector<double>& curvey,
					 vector<double>& kappa,
					 vector<double>& smoothX, vector<double>& smoothY,
					 double sigma,
					 bool isOpen
					 )
{
	vector<double> X,XX,Y,YY;
	SmoothCurve(curvex, curvey, smoothX, smoothY, X,XX,Y,YY, sigma, isOpen);
	
	kappa.resize(curvex.size());
	for (int i=0; i<curvex.size(); i++) {
		// Mokhtarian 02' eqn (4)
		kappa[i] = (X[i]*YY[i] - XX[i]*Y[i]) / pow(X[i]*X[i] + Y[i]*Y[i], 1.5);
        if (kappa[i] != kappa[i]) { kappa[i] = 0; }
	}
}

/* find zero crossings on curvature */
vector<int> FindCSSInterestPoints(const vector<double>& kappa) {
	vector<int> crossings;
	for (int i=0; i<kappa.size()-1; i++) {
		if ((kappa[i] < 0 && kappa[i+1] > 0) || (kappa[i] > 0 && kappa[i+1] < 0)) {
			crossings.push_back(i);
		}
	}
	return crossings;
}

void testFingerDetector() {
    FingertipDetector fd;
    
    VideoCapture vc;
    vc.open("/Users/roy_shilkrot/Documents/src/TextReading/build/Debug/woz2.mp4");
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
        
        CV_PROFILE(fd.processImage(img);)
        
        imshow("img",img);
        
        //        if (!writer.isInitialized()) {
        //            writer.setup("/Users/roy_shilkrot/Desktop/output.gif", img.cols, img.rows);
        //        }
        
        //        cvtColor(img, img, CV_BGR2RGB);
        //        writer.addFrame(img.data);
        //        stringstream ss; ss <<"/Users/roy_shilkrot/Desktop/output/img"<<i<<".jpg";
        //        imwrite(ss.str(), img);
        
        int c = waitKey(10);
        if(c==27) break;
    }
    //    writer.close();
}
