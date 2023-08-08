//------------------------------------------------------------------------------
// test_of_cv_gpu_timing.cpp
// compute multi-frm optical flow by cuda based alg: cv::cuda::SparsePyrLKOpticalFlow()
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------

#include "libUtil/util.h"

using namespace app;
using namespace std;
using namespace cv;
using namespace cv::cuda;

int test_of_cv_gpu_timing(const CfgApp &cfg)
{
    const CfgOfPtr prm = cfg.of;

    if (prm->flow_type != "sparse" ){
        dumpLog("we onl;y support sparse flow!");
        return -1;
    }
    
    cv::Size frameSize(prm->inputImgW, prm->inputImgH);

    Mat h_frame0(frameSize, CV_8UC3);
    Mat h_frame1(frameSize, CV_8UC3);
    Mat h_frame0Gray(frameSize, CV_8UC1);
    Mat h_frame1Gray(frameSize, CV_8UC1);
    vector<Point2f> h_prevPts, h_nextPts, h_gtPts;
    vector<uchar> h_status;

    GpuMat d_frame0(frameSize, CV_8UC3);
    GpuMat d_frame1(frameSize, CV_8UC3);
    GpuMat d_frame0Gray(frameSize, CV_8UC1);
    GpuMat d_frame1Gray(frameSize, CV_8UC1);
    GpuMat d_prevPts;
    GpuMat d_nextPts;
    GpuMat d_status;
    GpuMat d_flow(frameSize, CV_32FC2);

    //read the 0-th  frame
    int nLines = prm->initFeaturePts;
    loadImgAndFeaturePts(prm->inputImgDir, 0, false, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, h_prevPts, h_gtPts);
    if (prm->useGray)
    {
        d_frame0Gray.upload(h_frame0Gray);
    }
    else
    {
        d_frame0.upload(h_frame0);
    }

   // goodFeaturesToTrack
    dumpLog("oodFeaturesToTrack(): Points count: %d, minDist:%f", prm->maxFeaturePoints, prm->minFeaturePointDist);

    uploadFeaturePts( h_prevPts, d_prevPts);
    cout << "d_prevPts: " << d_prevPts.rows << ", " << d_prevPts.cols <<endl;


    // Sparse
    Ptr<cuda::SparsePyrLKOpticalFlow> d_pyrLK_sparse = cuda::SparsePyrLKOpticalFlow::create(
            Size(prm->winSize, prm->winSize), prm->maxPyrLevel, prm->iters);

    std::string windowName = "PyrLK [Sparse]";
    namedWindow( windowName, /*WINDOW_AUTOSIZE*/ WINDOW_NORMAL);
    AppTicToc   tt;
    AppMeanStd  dt1("us");
    AppMeanStd  dt2("us");
    AppMeanStd  dt3("us");
    for (uint32_t fn = 1; fn<prm->numFramesToProcess; fn++)
    {
        loadImg(prm->inputImgDir, fn, h_frame1, h_frame1Gray);

        tt.tic();
        if( prm->useGray ){
          d_frame1Gray.upload(h_frame1Gray);
        } 
        else{
          d_frame1.upload(h_frame1);          
        }
        dt1.addSample( tt.toc() );
        tt.tic();

        if( prm->useGray ){
          d_pyrLK_sparse->calc( d_frame0Gray, d_frame1Gray, d_prevPts, d_nextPts, d_status);
        }
        else{
          d_pyrLK_sparse->calc( d_frame0, d_frame1, d_prevPts, d_nextPts, d_status);
        }
        dt2.addSample( tt.toc() );
        tt.tic();

        downloadFeaturePts(d_nextPts, h_nextPts);
        downloadStatus(d_status, h_status);
        dt3.addSample( tt.toc() );



        OptFlowVis::drawArrows(h_frame0, h_prevPts, h_nextPts, h_status, Scalar(0, 0, 255), 1);
        if (prm->isDumpFlowImg)
        {
            OptFlowVis::dumpImg( h_frame0, "cv-gpu-OF.jpg",  fn-1);
        }

        if (prm->isShowFlowImg)
        {
            cv::imshow( windowName, h_frame0);

            int keyboard = waitKey(20);
            if (keyboard == 'q' || keyboard == 27)
                break;
        }

        //update previous Pts and d_frame0 and d_frame0Gray
        //todo: using ping-pong pointers to avoid this expensive copy      
        h_frame1.copyTo( h_frame0 );
        h_frame1Gray.copyTo( h_frame0Gray );  //one use frame0Gray
        d_frame1.copyTo( d_frame0);
        d_frame1Gray.copyTo( d_frame0Gray);
        d_nextPts.copyTo( d_prevPts );
        h_prevPts = h_nextPts;
    }
    dt1.calMeanStd();
    dt2.calMeanStd();
    dt3.calMeanStd();
    
    printf("OF alg: cv-cuda: totFrame=%u, dt1(mu=%.1f, std=%.1f), dt2(mu=%.1f, std=%.1f), dt3(mu=%.1f, std=%.1f) (us)\n",
             prm->numFramesToProcess, dt1.mu,  dt1.std, dt2.mu,  dt2.std, dt3.mu,  dt3.std);

    printf("press any key on image window to quit ... \n");

    if (prm->isShowFlowImg)
        cv::waitKey(1000);

    return 0;
}

