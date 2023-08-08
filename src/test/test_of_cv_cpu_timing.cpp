//------------------------------------------------------------------------------
// test_of_cv_cpu_timing.cpp
// compute multi-frm optical flow by cpu based alg: cv::calcOpticalFlowPyrLK()
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------
#include "libUtil/util.h"

using namespace cv;
using namespace std;
using namespace app;

int test_of_cv_cpu_timing(const CfgApp &cfg0)
{
    const CfgOfPtr prm = cfg0.of;
    // Create some random colors
    vector<Scalar> colors;
    RNG rng;
    for (int i = 0; i < 100; i++)
    {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r, g, b));
    }

    Mat h_frame0, h_frame1, h_frame0Gray, h_frame1Gray;
    vector<Point2f> prevPts_h, nextPts_h, gtPts_h;
    vector<Point2f> good_new;

    AppTicToc tt;
    AppMeanStd measStd("us");

    // read first frame1 and find corners in it
    uint8_t goodStatus=1;
    int nLines = prm->initFeaturePts;
    loadImgAndFeaturePts(prm->inputImgDir, 0, false, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevPts_h, gtPts_h);
    //dumpFeaturePts( prm->outputImgDir, "timing-cv-cpu", 0, prevPts_h, goodStatus);

    //OptFlowVis::printFeatures(prevPts_h);

    // Create a mask image for drawing purposes
    // Mat mask = Mat::zeros(frame0.size(), frame0.type());
    Mat frame1, gray1;
    TermCriteria criteria = TermCriteria((TermCriteria::COUNT) + (TermCriteria::EPS), cfg0.of->iters, 0.03);
    for (uint32_t fn = 1; fn<prm->numFramesToProcess; fn++)
    {
        loadImg(prm->inputImgDir, fn, h_frame1, h_frame1Gray);
#if 0
        char buffer[512] = {};
        snprintf(buffer, sizeof(buffer) - 1, "./img%04d.png", frmCnt);
        imwrite(buffer, h_frame1);
#endif
        // calculate optical flow
        vector<uchar> status;
        vector<float> err;

        tt.tic();
        calcOpticalFlowPyrLK(h_frame0Gray, h_frame1Gray, prevPts_h, nextPts_h, status, err, Size(cfg0.of->winSize, cfg0.of->winSize), cfg0.of->maxPyrLevel, criteria);
        measStd.addSample(tt.toc());

        // Select good points
        selectTrackedPts( good_new, nextPts_h, status, goodStatus);     

        // visualize  and dump results
        cv::Scalar line_color = cv::Scalar(0, 0, 255);
        OptFlowVis::drawArrows(h_frame0, prevPts_h, nextPts_h, status, line_color, 1);

       // dump h_frame0
        if (cfg0.of->isDumpFlowImg)
        {
            OptFlowVis::dumpImg(h_frame0, "cv-cpu-OF.jpg", fn);
        }

       // show h_frame0
        if (cfg0.of->isShowFlowImg)
        {
            imshow("flow", h_frame0);
            int keyboard = waitKey(20);
            if (keyboard == 'q' || keyboard == 27)
                break;
        }

        // Now update the previous frame1 and previous points
        h_frame1.copyTo(h_frame0);
        h_frame1Gray.copyTo(h_frame0Gray);
        prevPts_h = good_new;
    }
    measStd.calMeanStd();
    printf("OF alg:  CPU: totFrame=%u, OF used time: mu=%.1f, std=%.1f(us)", prm->numFramesToProcess, measStd.mu, measStd.std);

    if (cfg0.of->isShowFlowImg)
        waitKey(100);
}

