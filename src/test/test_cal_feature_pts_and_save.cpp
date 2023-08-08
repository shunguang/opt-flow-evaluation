
// (1) test_gst_mux_enc_v0.cpp is the initial study version
// (2) test_gst_mux_enc_v1.cpp is the final version of test_gst_mux_enc_v0.cpp
// (3) both versions functionally are same with each other
//
// modified from https://github.com/impleotv/gstreamer-test-klv

/*
testvideoSrc->srcCapsFilter->timeoverla->encoderH264->vidCapsFilter->video_queue->mpegtsmux->filesink
                                                                                ^
                                                                                |
                                                                              metadataSrc
*/
//-------------------------------------------------------------------
// test_gst_klv_enc_min_v1.cpp
// an example to enc metadata wand image by two callback functions
//-------------------------------------------------------------------
#include <iostream>
#include <cassert>
#include <string>
#include <thread>
#include <stdio.h>
#include <unistd.h>

#include <dlfcn.h>
#include <unistd.h>

#include "libUtil/util.h"
#include "libUtil/Cfg.h"

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include "libUtil/util.h"

using namespace cv;
using namespace std;
using namespace app;
int test_cal_feature_pts_and_save(const CfgApp &cfg0)
{
    const string dirOut = "../../dataset/dataset/opticalFlow/sparse/cars/img";
    const string filename("../../dataset/opticalFlow/sparse/cars.mp4");
    CapImg cap(filename);

    if (!cap.isProbeSuccess())
    {
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    Mat frame0, gray0;
    vector<Point2f> p0;
    for (uint32_t frmCnt = 0; frmCnt<cfg0.of->numFramesToProcess; frmCnt++)
    {
        printf("%d/%d, ", frmCnt, cap.m_nTotFrames);
        if (0 == frmCnt % 50) printf("/n");

        // get an new frame1
        bool hasNewFrm = cap.getNextFrame(frame0);
        if (!hasNewFrm) break;



        Mat frame1;
        if( cfg0.of->resizedImgH > 0 && cfg0.of->resizedImgW>0 ){
            cv::resize(frame0, frame1,cv::Size(cfg0.of->resizedImgW, cfg0.of->resizedImgH), 0, 0, CV_INTER_LINEAR);
        }
        else{
            frame1 = frame0;
        }

        cvtColor(frame1, gray0, COLOR_BGR2GRAY);
        goodFeaturesToTrack(gray0, p0, cfg0.of->maxFeaturePoints, 0.3, 7, Mat(), 7, false, 0.04);

        std::vector<uchar> status_h( p0.size(), 1);       
        OptFlowVis::dumpImg(frame1, dirOut + "/img/ori-frm.png", frmCnt);
        dumpFeaturePts( dirOut+"/fpt", "featurePts", frmCnt, p0, status_h);
    }
}

