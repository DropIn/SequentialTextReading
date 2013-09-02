////
////  main.cpp
////  testopencv
////
////  Created by roy_shilkrot on 5/6/13.
////  Copyright (c) 2013 roy_shilkrot. All rights reserved.
////
//
//  main.cpp
//  testopencv
//
//  Created by roy_shilkrot on 5/6/13.
//  Copyright (c) 2013 roy_shilkrot. All rights reserved.
//

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

using namespace cv;
using namespace std;

typedef Mat_<float> Matf;
typedef Mat_<double> Matd;


void Kwon2DDLT(const Matf& pc, const Matf& Q) {
    Matf A,B(8,1);
    float R = 1;
    for (int i=0; i<pc.rows; i++) {
        Matf ap = (Matf(2,8) <<
                   Q(0,0)/R, Q(0,1)/R, 1.0f/R, 0, 0, 0, -pc(0,0)*Q(0,0)/R, -pc(0,0)*Q(0,1)/R,
                   0, 0, 0, Q(0,0)/R, Q(0,1)/R, 1.0f/R, -pc(0,1)*Q(0,0)/R, -pc(0,1)*Q(0,1)/R
                   );
        A.push_back(ap);
        B(i*2) = pc(0,0)/R;
        B(i*2+1) = pc(0,1)/R;
    }
    Matf X;
    solve(A, B, X, DECOMP_SVD);
}

void YangCaoLoZhang(const Matf& _pc, const Matf& Q) {
    Matf Qbar = Q;
    Matf pc = _pc;
//    Q.colRange(0, 2).copyTo(Qbar.colRange(0, 2));
//    Qbar.col(2).setTo(1);
    cout << "Qbar " << Qbar << endl;
    cout << "pc " << pc << endl;
    Matf A;
    for (int i=0; i<pc.rows; i++) {
//        float lambda = pc(i,2), u = pc(i,0), v = pc(i,1);
        float lambda = 1.0, u = pc(i,0)/pc(i,2), v = pc(i,1)/pc(i,2);
        Matf Qi = Qbar.row(i);
        Matf ap = (Matf(2,9) << 0,0,0, -lambda * Qi(0), -lambda * Qi(1), -lambda * Qi(3), v * Qi(0), v * Qi(1), v * Qi(3) ,
                   lambda * Qi(0), lambda * Qi(1), lambda * Qi(3), 0,0,0, -u * Qi(0), -u * Qi(1), -u * Qi(3)
                   );
        A.push_back(ap);
    }
    cout << "A("<<A.rows<<"x"<<A.cols<<") " << A << endl;
//    Matf h;
//    SVD svd(A);
//    cout << "vt " << svd.vt << endl;
//    Matf nul = svd.vt.row(svd.vt.rows-1);
//    cout << "vt last row: " << nul << endl;
    Matf nul;
    SVD::solveZ(A, nul);
    cout << "nul: " << nul << endl;
    nul = nul.t();
    cout << "||A*nul|| " << norm(A*nul.t()) << endl;
    
    Matf R(3,3);
    nul /= norm(nul.colRange(0, 3));
    Matf h1 = nul.colRange(0, 3), h2 = nul.colRange(3,6);
    Matf(h1.t()).copyTo(R.col(0));
    Matf(h2.t()).copyTo(R.col(1));
    Matf R2 = R.col(0).cross(R.col(1)); //using cross product
    R2.copyTo(R.col(2));

    cout << "R " << R << endl;
    cout << "|R| " << determinant(R) << endl;
//    spm.train(cam_rgb, mask_);
//    imshow("mask",depthGRAY & mask_);
    cout << "||R0|| " << norm(R.col(0)) << endl;
    cout << "||R1|| " << norm(R.col(1)) << endl;
}

/**
 * Compute rotation and translation of plane from 3 camera-plane point pairs.
 *
 * The simple linear solution (Appendix 1)
 * from "Review and Analysis of Solutions of the Three Point Perspective Pose Estimation Problem"
 * R Harralick,C-N Lee, K Ottenberg, M Nolle, IJCV 1994
 *
 * @param pc points in camera coordinates (calibrated, Kinv * p)
 * @param pw points in world coordinates
 * @param R output rotation matrix 3x3
 * @param t output translation vector 1x3
 */
void HaralickLeeOttenbergNolle(const Matf& pc, const Matf& pw, Matf& Rcf, Matf& tc) {
    assert(pc.rows == pw.rows);
    
    cout << "pc " << pc << endl;
    cout << "pw " << pw << endl;
    
    Matf A,B(3*pc.rows,1);
    for (int i=0; i<pw.rows; i++) {
        Matf A_part = (Matf(3,9) <<
                       pw.row(i)(0),pw.row(i)(1),0,0,0,0,1,0,0,
                       0,0,pw.row(i)(0),pw.row(i)(1),0,0,0,1,0,
                       0,0,0,0,pw.row(i)(0),pw.row(i)(1),0,0,1
                       );
        A.push_back(A_part);
        
        for (int j=0; j<3; j++) {
            B(i*3+j,0) = pc(i,j);
        }
    }
    Matf X;
    solve(A, B, X, DECOMP_SVD);
    cout << "A " << A << endl;
    cout << "B " << B << endl;
    cout << "X " << X << endl;
    
    Matf R(3,3);
    R(0,0) = X(0); R(0,1) = X(1);
    R(1,0) = X(2); R(1,1) = X(3);
    R(2,0) = X(4); R(2,1) = X(5);
    /*
     //using direct calculation
    R(2,2) = R(0,0)*R(1,1) - R(0,1)*R(1,0); //r33 = rllr22 -- r12r21
    R(1,2) = R(0,1)*R(2,0) - R(0,0)*R(2,1); //r23 = r12r31 -- r11r32
    R(0,2) = R(1,0)*R(2,1) - R(1,1)*R(2,2); //r13 = r21r32 -- r22r33
     */
    Matf R2 = R.col(0).cross(R.col(1)); //using cross product
    R2.copyTo(R.col(2));
    Matf t = (Matf(1,3) << X(6),X(7),X(8));
    
    cout << "R " << R << endl;
    cout << "t " << t << endl;
    Rcf = R; tc = t;
}

int main1(int argc, const char* argv[]) {
//    Matf K = (Mat_<float>(3,3) << mwh,0,img_w/2.0, 0,mwh,img_h/2.0, 0,0,1);
//    cout << "K " << K << endl;

    Matf pw(3,4); pw.colRange(2, 3).setTo(0); pw.colRange(3, 4).setTo(1);
    pw(0,0) = -50; pw(0,1) = -50;
    pw(1,0) = -50; pw(1,1) = 50;
    pw(2,0) = 50; pw(2,1) = 50;
    
    Matf pc(3,3);
    pc(0,0) = 0.1530434787273407; pc(0,1) =  0.2660144865512848; pc(0,2) = 0.009999999776482582;
    pc(1,0) = 0.06608695536851883; pc(1,1) = 0.7732608914375305; pc(1,2) = 0.009999999776482582;
    pc(2,0) = 0.6226087212562561; pc(2,1) = 0.7732608914375305; pc(2,2) = 0.009999999776482582;

    Matf R,t;
    HaralickLeeOttenbergNolle(pc, pw, R, t);
}

int main2(int argc, const char * argv[])
{
    float img_w = 480, img_h = 800;
    float mwh = max(img_w,img_h);
     
    Matf K = (Mat_<float>(3,3) << mwh,0,img_w/2.0, 0,mwh,img_h/2.0, 0,0,1);
    cout << "K " << K << endl;
    Matf Kinv = K.inv();
    cout << "Kinv " << Kinv << endl;
    
    RNG rng;
//    Matf Q(4,4);
//    for (int i=0; i<Q.rows; i++) {
//        Q(i,0) = rng.uniform(-3.0f, 3.0f);
//        Q(i,1) = rng.uniform(-3.0f, 3.0f);
//        Q(i,2) = 0;
//        Q(i,3) = 1.0f;
//    }
    Matf Q(4,4); Q.colRange(2, 3).setTo(0); Q.colRange(3, 4).setTo(1);
    Q(0,0) = -50; Q(0,1) = -50;
    Q(1,0) = -50; Q(1,1) = 50;
    Q(2,0) = 50; Q(2,1) = 50;
    Q(3,0) = 50; Q(3,1) = -50;

    cout << "Q " << Q << endl;
    
//    Rx = matrix([[1,0,0],[0,cos(rotX),-sin(rotX)],[0,sin(rotX),cos(rotX)]])
//    Ry = matrix([[cos(rotY),0,sin(rotY)],[0,1,0],[-sin(rotY),0,cos(rotY)]])
//    Rz = matrix([[cos(rotZ),-sin(rotZ),0],[sin(rotZ),cos(rotZ),0],[0,0,1]])

    float rotX = rng.uniform(-CV_PI/16, CV_PI/16), rotY = rng.uniform(-CV_PI/16, CV_PI/16), rotZ = rng.uniform(-CV_PI/16, CV_PI/16);
    Matf Rx = (Matf(3,3) << 1,0,0, 0,cos(rotX),-sin(rotX), 0,sin(rotX),cos(rotX));
    Matf Ry = (Matf(3,3) << cos(rotY),0,sin(rotY), 0,1,0, -sin(rotY),0,cos(rotY));
    Matf Rz = (Matf(3,3) << cos(rotZ),-sin(rotZ),0, sin(rotZ),cos(rotZ),0, 0,0,1);
    Matf R = Rx*Ry*Rz;
    cout << "R " << R << endl;
    Matf C = (Matf(3,1) << rng.uniform(-1.0f, 1.0f), rng.uniform(-1.0f, 1.0f), rng.uniform(-1.0f, 1.0f));
//    Matf C = (Matf(3,1) << 0,0,0);
    cout << "C " << C << endl;
    
    Matf P(3,4); R.copyTo(P.colRange(0,3)); P.col(3) = -R*C;
    cout << "P " << P << endl;
    
    Matf KP = K*P;
    cout << "KP " << KP << endl;
    
    Matf KPQ = KP * Q.t();
    for (int i=0; i<KPQ.cols; i++) {
        KPQ.col(i) /= KPQ(2,i);
    }
    cout << "KPQ " << KPQ << endl;

    Matf pc =  P*Q.t(); //KPQ;//
    cout << "pc " << pc << endl;
    
    Matf Rcf,tc;
    HaralickLeeOttenbergNolle(pc.t(), Q, Rcf, tc);
    YangCaoLoZhang(pc.t(), Q);
    /*
    Matd rvec,tvec,Rc(3,3);
    cout << "Q.colRange(0, 3) " << Q.colRange(0, 3) << endl;
    cout << "pc.rowRange(0, 2).t() " << pc.rowRange(0, 2).t() << endl;
    vector<Point3f> p3f(Q.rows); Q.colRange(0, 3).reshape(3,Q.rows).copyTo(Mat(p3f));
    vector<Point2f> p2f(pc.cols); Mat(pc.rowRange(0, 2).t()).reshape(2,pc.cols).copyTo(Mat(p2f));
    cout << "p3f " << p3f << endl;
    cout << "p2f " << p2f << endl;
    solvePnP(p3f, p2f, K, Mat(), rvec, tvec);
    Rodrigues(rvec, Rc);
    cout << "Rc " << Rc << endl;
    cout << "tc " << tvec << endl;
    Matf Rcf = Rc; Matf tc = tvec;
     */
    
//    cout << "|Rc.t() * R - I| " << determinant(Rcf.t() * R - Matf::eye(3,3)) << endl;
//    cout << "||tc - t|| " << norm(-Rcf.t() * tc.t() - C) << endl;

    return 0;
}

