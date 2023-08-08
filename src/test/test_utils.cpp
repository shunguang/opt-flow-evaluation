/*
 * main.cpp
 *
 *  Created on: Jun 8, 2023
 *      Author: wus1
 */
#include "libUtil/util.h"

using namespace std;
using namespace app;
using namespace cv;

int test_combine_imgs();
int test_template2();
int testVpiUpload();

int test_utils(int argc, char *argv[])
{
    int x;
    x = test_combine_imgs();
    return x;
}

int testVpiUpload()
{
    VPIArray curFeatures = NULL;
    // CHECK_STATUS(vpiArrayCreate(MAX_KEYPOINTS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &curFeatures));

    int32_t cap = 200;
    int32_t nSize = 100;

    std::vector<cv::Point2f> v;
    for (int i = 0; i < nSize; i++)
        v.push_back(cv::Point2f(i, i * i));
#if 1
    VPIArrayData arraydata;
    arraydata.bufferType = VPI_ARRAY_BUFFER_HOST_AOS;
    arraydata.buffer.aos.type = VPI_ARRAY_TYPE_KEYPOINT_F32;
    arraydata.buffer.aos.capacity = cap;
    arraydata.buffer.aos.sizePointer = &nSize;
    arraydata.buffer.aos.data = (VPIKeypointF32 *)v.data(); // Work
    CHECK_STATUS(vpiArrayCreateWrapper(&arraydata, 0, &curFeatures));
#else
    // todo: why does not work
    uploadFeaturePts(v, &curFeatures);
#endif

    // download it
    std::vector<cv::Point2f> v_h;
    downloadFeaturePts(curFeatures, nSize, v_h);
    for (int i = 0; i < nSize; i++)
        std::cout << "v_h[i].x, v_h[i].y : " << v_h[i].x << ",  " << v_h[i].y << std::endl;

    vpiArrayDestroy(curFeatures);
    return 0;
}


int test_combine_imgs()
{   
    const std::string dir = ".";
    const int nTot = 97;
    const int W=1920;
    const int H=1080;

    const int W1=1920/2;
    const int H1=1080/2;

    cv::Mat y1,y2,y3;   
    cv::Mat out(3*H1, W1, CV_8UC3 );
    for( int i=1; i<nTot; i++){
        char buf[32] = {};
        snprintf(buf, sizeof(buf) - 1, "_%04d", i);
        std::string  f1 = dir + "/cv-cpu-OF"+ string(buf) + ".jpg";
        std::string  f2 = dir + "/cv-gpu-OF"+ string(buf) + ".jpg";
        std::string  f3 = dir + "/nv-vpi-OF"+ string(buf) + ".jpg";

        cv::Mat x1 = cv::imread( f1, cv::IMREAD_COLOR );
        cv::Mat x2 = cv::imread( f2, cv::IMREAD_COLOR );
        cv::Mat x3 = cv::imread( f3, cv::IMREAD_COLOR );

        cv::putText(x1, "CV-CPU",cv::Point(50,H-50),cv::FONT_HERSHEY_DUPLEX,2,cv::Scalar(255,255,255), 2,false);
        cv::putText(x2, "CV-GPU",cv::Point(50,H-50),cv::FONT_HERSHEY_DUPLEX,2,cv::Scalar(255,255,255), 2,false);
        cv::putText(x3, "NV-VPI",cv::Point(50,H-50),cv::FONT_HERSHEY_DUPLEX,2,cv::Scalar(255,255,255), 2,false);

        assert(x1.rows == H && x1.cols == W  );
        assert(x2.rows == H && x2.cols == W  );
        assert(x3.rows == H && x3.cols == W  );

        if( W !=W1 || H !=H1 ){
            cv::resize(x1, y1, cv::Size(W1,H1), cv::INTER_LINEAR );
            cv::resize(x2, y2, cv::Size(W1,H1), cv::INTER_LINEAR );
            cv::resize(x3, y3, cv::Size(W1,H1), cv::INTER_LINEAR );
        }
        else{
            y1=x1;
            y2=x2;
            y3=x3;
        }

        out=cv::Scalar(0,0,0);
        int y0 = 0;
        y1.copyTo( out( cv::Rect(0, y0, W1, H1) ) );
        y0 += H1;
        y2.copyTo( out( cv::Rect(0, y0, W1, H1) ) );
        y0 += H1;
        y3.copyTo( out( cv::Rect(0, y0, W1, H1) ) );
        
        OptFlowVis::dumpImg( out, "./combined-smaller.jpg", i);      
    }

    return 0;
}

int test_template2()
{
    return 0;
}

