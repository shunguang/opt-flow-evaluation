//------------------------------------------------------------------------------
// test_of_cv_cpu_1frm.cpp
// compute multi-frm optical flow by cpu based alg: cv::calcOpticalFlowPyrLK()
// accuracy oriented, do not care about efficency
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------
#include "libUtil/util.h"

using namespace cv;
using namespace std;
using namespace app;


int test_of_cv_cpu_1frm(const CfgApp &cfg0)
{
    const CfgOfPtr prm = cfg0.of;
    bool isVincent=prm->isForVincentDataSet;

    int nLines = prm->initFeaturePts;
    cv::Size frameSize( prm->inputImgW, prm->inputImgW);

    Mat h_frame0(frameSize, CV_8UC3);
    Mat h_frame1(frameSize, CV_8UC3);
    Mat h_frame0Gray(frameSize, CV_8UC1);
    Mat h_frame1Gray(frameSize, CV_8UC1);

    vector<Point2f> prevPts_h;
    vector<Point2f> gtPts_h;
    vector<Point2f> nextPts_h;
    vector<uint8_t> status_h;
    vector<float> err;

    TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), prm->iters, prm->epsilon );
    std::string fname0,fname1, fnameFpts;

    for( int fn=0; fn<(prm->numFramesToProcess-1); fn++ ){
        loadImgAndFeaturePts(prm->inputImgDir, fn, isVincent, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevPts_h, gtPts_h);

        calcOpticalFlowPyrLK(h_frame0Gray, h_frame1Gray, prevPts_h, nextPts_h, status_h, err, Size(prm->winSize, prm->winSize), prm->maxPyrLevel, criteria);

        if(isVincent){
            dumpFeaturePtsVincent( prm->outputImgDir, "classRoom-result-cv-cpu", prevPts_h, gtPts_h, nextPts_h, status_h);
        }
        else{
            dumpFeaturePts( prm->outputImgDir, "fpt-cv-cpu", fn+1, prevPts_h, nextPts_h, status_h);
        }

        if(isVincent){
           break;
        } 
    }
}

