//------------------------------------------------------------------------------
// test_of_cv_cpu_multi_frm.cpp
// compute multi-frm optical flow by cpu based alg: cv::calcOpticalFlowPyrLK()
// accuracy oriented, do not care about efficency
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------
#include "libUtil/util.h"

using namespace cv;
using namespace std;
using namespace app;


int test_of_cv_cpu_multi_frm(const CfgApp &cfg0)
{
    const uint8_t goodStatus = 1;
    const CfgOfPtr prm = cfg0.of;
    int nLines = prm->initFeaturePts;
    cv::Size frameSize( prm->inputImgW, prm->inputImgW);

    Mat h_frame0(frameSize, CV_8UC3);
    Mat h_frame1(frameSize, CV_8UC3);
    Mat h_frame0Gray(frameSize, CV_8UC1);
    Mat h_frame1Gray(frameSize, CV_8UC1);

    vector<Point2f> prevPts_h,nextPts_h, good_new, gtPts_h;
    vector<uint8_t> status_h;
    vector<float> err;

    TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), prm->iters, prm->epsilon );
    std::string fname0,fname1, fnameFpts;

    loadImgAndFeaturePts(prm->inputImgDir, 0, false, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevPts_h, gtPts_h);
    dumpFeaturePts( prm->outputImgDir, "multi-cv-cpu", 0, prevPts_h, goodStatus);

    for( int fn=1; fn<prm->numFramesToProcess; fn++ ){
        loadImg(prm->inputImgDir, fn, h_frame1, h_frame1Gray);

        calcOpticalFlowPyrLK(h_frame0Gray, h_frame1Gray, prevPts_h, nextPts_h, status_h, err, Size(prm->winSize, prm->winSize), prm->maxPyrLevel, criteria);
        
        dumpFeaturePts( prm->outputImgDir, "multi-cv-cpu", fn, nextPts_h, status_h);


        // visualize  results
        cv::Scalar line_color = cv::Scalar(0, 0, 255);
        OptFlowVis::drawArrows(h_frame0, prevPts_h, nextPts_h, status_h, line_color, goodStatus);
        if (cfg0.of->isDumpFlowImg)
        {
            OptFlowVis::dumpImg(h_frame0, "cv-cpu-OF.jpg", fn);
        }
        if (cfg0.of->isShowFlowImg)
        {
            imshow("flow", h_frame0);
            int keyboard = waitKey(20);
            if (keyboard == 'q' || keyboard == 27)
                break;
        }

        // prepare for next frame
        selectTrackedPts( good_new, nextPts_h, status_h, goodStatus);     

        h_frame1.copyTo(h_frame0);
        h_frame1Gray.copyTo(h_frame0Gray);
        prevPts_h = good_new;
    }
}

