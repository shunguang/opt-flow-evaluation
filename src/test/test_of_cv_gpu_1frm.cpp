//------------------------------------------------------------------------------
// test_of_cv_gpu_1frm.cpp
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



int test_of_cv_gpu_1frm(const CfgApp &cfg)
{
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

    // Sparse
    Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
            Size(prm->winSize, prm->winSize), prm->maxPyrLevel, prm->iters, false);

    for( int fn=0; fn< (prm->numFramesToProcess-1); fn++ ){
        loadImgAndFeaturePts(prm->inputImgDir, fn, isVincent, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevPts_h, gtPts_h);

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

        if(isVincent){
            dumpFeaturePtsVincent( prm->outputImgDir, "classRoom-result-cv-gpu", prevPts_h, gtPts_h, nextPts_h, status_h);
        }
        else{
            dumpFeaturePts( prm->outputImgDir,"fpt-cv-gpu", fn+1, prevPts_h, nextPts_h, status_h);
        }

        if(isVincent){
           break;
        } 

    }
    return 0;
}

