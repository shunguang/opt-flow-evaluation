//------------------------------------------------------------------------------
// test_of_cv_gpu_multi_frm.cpp
// compute multi-frm optical flow by cuda based alg: cv::cuda::SparsePyrLKOpticalFlow()
// accuracy oriented, do not care about efficency
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------

#include "libUtil/util.h"

using namespace app;
using namespace std;
using namespace cv;
using namespace cv::cuda;

#define DUMP_FEATURE_PT 1



int test_of_cv_gpu_multi_frm(const CfgApp &cfg)
{
    const uint8_t goodStatus=1;
    const CfgOfPtr prm = cfg.of;
    bool isVincent=prm->isForVincentDataSet;

    if (prm->flow_type != "sparse" ){
        dumpLog("we onl;y support sparse flow!");
        return -1;
    }


    bool useGray = prm->useGray;
    int nLines = prm->initFeaturePts;
    cv::Size frameSize( prm->inputImgW, prm->inputImgW);

    Mat h_frame0(frameSize, CV_8UC3);
    Mat h_frame1(frameSize, CV_8UC3);
    Mat h_frame0Gray(frameSize, CV_8UC1);
    Mat h_frame1Gray(frameSize, CV_8UC1);

    GpuMat d_frame0(frameSize, CV_8UC3);
    GpuMat d_frame1(frameSize, CV_8UC3);
    GpuMat d_frame0Gray(frameSize, CV_8UC1);
    GpuMat d_frame1Gray(frameSize, CV_8UC1);

    GpuMat d_prevPts;
    GpuMat d_nextPts;
    GpuMat d_status;

    vector<Point2f> prevPts_h;
    vector<Point2f> gtPts_h;
    vector<Point2f> nextPts_h;
    vector<uint8_t> status_h;
    vector<Point2f> good_new;

    // Sparse
    Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
            Size(prm->winSize, prm->winSize), prm->maxPyrLevel, prm->iters, false);

    loadImgAndFeaturePts(prm->inputImgDir, 0, false, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevPts_h, gtPts_h);
    dumpFeaturePts( prm->outputImgDir, "multi-cv-gpu", 0, prevPts_h, goodStatus);

    for( int fn=1; fn< prm->numFramesToProcess; fn++ ){
        loadImg(prm->inputImgDir, fn, h_frame1, h_frame1Gray);

        d_frame0.upload(h_frame0);
        d_frame1.upload(h_frame1);
        d_frame0Gray.upload(h_frame0Gray);
        d_frame1Gray.upload(h_frame1Gray);
        uploadFeaturePts( prevPts_h, d_prevPts );

        if( useGray ){
          d_pyrLK_sparse->calc( d_frame0Gray, d_frame1Gray, d_prevPts, d_nextPts, d_status);
        }
        else{
          d_pyrLK_sparse->calc( d_frame0, d_frame1, d_prevPts, d_nextPts, d_status);
        }

        printf("d_nextPts(%d x %d), d_status(%d x %d)\n", d_nextPts.rows, d_nextPts.cols, d_status.rows, d_status.cols );

        nextPts_h.clear();
        status_h.clear();
        downloadFeaturePts(d_nextPts, nextPts_h);
        downloadStatus(d_status, status_h);

        dumpFeaturePts( prm->outputImgDir, "multi-cv-gpu", fn, nextPts_h, status_h);


        //-----------------------------------------
        // visualize  results
        //-----------------------------------------
        cv::Scalar line_color = cv::Scalar(0, 0, 255);
        OptFlowVis::drawArrows(h_frame0, prevPts_h, nextPts_h, status_h, line_color, goodStatus);
        if (prm->isDumpFlowImg)
        {
            OptFlowVis::dumpImg(h_frame0, "cv-gpu-OF.jpg", fn);
        }
        if (prm->isShowFlowImg)
        {
            imshow("flow", h_frame0);
            int keyboard = waitKey(20);
            if (keyboard == 'q' || keyboard == 27)
                break;
        }

        //-----------------------------------------
        // prepare for next frame
        //-----------------------------------------
        selectTrackedPts( good_new, nextPts_h, status_h, goodStatus);     

        h_frame1.copyTo(h_frame0);
        h_frame1Gray.copyTo(h_frame0Gray);
        prevPts_h = good_new;

    }
    return 0;
}

