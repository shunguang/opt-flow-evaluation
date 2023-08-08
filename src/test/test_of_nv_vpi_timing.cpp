//------------------------------------------------------------------------------
// test_of_nv_vpi_timing.cpp
// compute multi-frm optical flow by nv vpi alg: vpiSubmitOpticalFlowPyrLK()
// author: Shunguang Wu (shunguang.wu@jhuapl.edu)
// date:   08/08/2023
//------------------------------------------------------------------------------

#include "libUtil/util.h"

#include <vpi/OpenCVInterop.hpp>
#include <vpi/Array.h>
#include <vpi/Image.h>
#include <vpi/Pyramid.h>
#include <vpi/Status.h>
#include <vpi/Stream.h>
#include <vpi/algo/ConvertImageFormat.h>
#include <vpi/algo/GaussianPyramid.h>
#include <vpi/algo/HarrisCorners.h>
#include <vpi/algo/OpticalFlowPyrLK.h>

using namespace app;
using namespace std;
using namespace cv;

#define VIS_MODE 2
int test_of_nv_vpi_timing(const CfgApp &cfg)
{
     int retval = 0;

#if 0    
    const CfgOfPtr prm = cfg.of;
    if (prm->flow_type != "sparse" ){
        dumpLog("we onl;y support sparse flow!");
        return -1;
    }
    
    const uint8_t goodStatus=0;

    // OpenCV image that will be wrapped by a VPIImage.
    // Define it here so that it's destroyed *after* wrapper is destroyed
    cv::Mat h_frame0, h_frame1, h_frame0Gray, h_frame1Gray;
    std::vector<cv::Point2f> h_prevPts, h_currPts, h_gtPts;
    std::vector<uchar> h_status;

    // VPI objects that will be used
    VPIStream stream = NULL;
    VPIImage imgTempFrame = NULL;
    VPIImage imgFrame = NULL;
    VPIPyramid pyrPrevFrame = NULL, pyrCurrFrame = NULL;
    VPIArray prevFeatures = NULL, currFeatures = NULL, status = NULL;
    VPIPayload optflow = NULL;
    VPIArray scores = NULL;
    VPIPayload harris = NULL;

    std::string strBackend = cfg.of->nv_vpi_backend;
    std::string strInputImgDir = cfg.of->inputImgDir;
    int32_t pyrLevel = cfg.of->maxPyrLevel;
    std::string strOutputFiles = cfg.of->outputImgDir + "/nv-vpi-" + strBackend + ".jpg";

    VPIBackend backend;
    if (strBackend == "cpu")
    {
        backend = VPI_BACKEND_CPU;
    }
    else if (strBackend == "cuda")
    {
        backend = VPI_BACKEND_CUDA;
    }
    else
    {
        throw std::runtime_error("Backend '" + strBackend + "' not recognized, it must be either cpu or cuda.");
    }

#if VIS_MODE == 1
    OptFlowVis vis( prm->inputImgW, prm->inputImgH, 1000);
#endif

    int nLines = prm->initFeaturePts;

    try
    {
        // =================================================
        // Allocate VPI resources and do some pre-processing

        // Create the stream where processing will happen.
        CHECK_STATUS(vpiStreamCreate(0, &stream));
        CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(h_frame1, 0, &imgTempFrame));

        // Create grayscale image representation of input.
        CHECK_STATUS(vpiImageCreate(h_frame1.cols, h_frame1.rows, VPI_IMAGE_FORMAT_U8, 0, &imgFrame));

        // Create the image pyramids used by the algorithm
        CHECK_STATUS(vpiPyramidCreate(h_frame1.cols, h_frame1.rows, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5, 0, &pyrPrevFrame));
        CHECK_STATUS(vpiPyramidCreate(h_frame1.cols, h_frame1.rows, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5, 0, &pyrCurrFrame));

        // Create input and output arrays
        CHECK_STATUS(vpiArrayCreate(MAX_HARRIS_CORNERS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &prevFeatures));
        CHECK_STATUS(vpiArrayCreate(MAX_HARRIS_CORNERS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &currFeatures));
        CHECK_STATUS(vpiArrayCreate(MAX_HARRIS_CORNERS, VPI_ARRAY_TYPE_U8, 0, &status));

        // Create Optical Flow payload
        CHECK_STATUS(vpiCreateOpticalFlowPyrLK(backend, h_frame1.cols, h_frame1.rows, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5,
                                               &optflow));

        // Parameters we'll use. No need to change them on the fly, so just define them here.
        // We're using the default parameters.
        VPIOpticalFlowPyrLKParams lkParams;
        lkParams.epsilon=0.1;
        lkParams.epsilonType = VPI_LK_ERROR_L1;
        lkParams.numIterations = cfg.of->iters;
        lkParams.useInitialFlow = 0;
        lkParams.windowDimension =  cfg.of->winSize;
        lkParams.termination = VPI_TERMINATION_CRITERIA_ITERATIONS | VPI_TERMINATION_CRITERIA_EPSILON;

        CHECK_STATUS(vpiInitOpticalFlowPyrLKParams(&lkParams));

        // Create a mask image for drawing purposes
        cv::Mat cvMask = cv::Mat::zeros(h_frame1.size(), CV_8UC3);

        // Gather feature points from first frame using Harris Corners on CPU.
        {
            OptFlowVis::printFeatures(h_currPts);
        }


        loadImgAndFeaturePts(prm->inputImgDir, 0, false, nLines,  h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, h_prevPts, h_gtPts);
        {
           //set init frame as curr
            std::swap(h_prevPts, h_currPts);
            h_frame0.copyTo(h_frame1);

#if VIS_MODE == 1
        vis.resetColor(h_currPts);
        // Update the mask with info from first frame.
        h_status.resize(h_currPts.size(), goodStatus);
        int numTrackedKeypoints = vis.updateMask(cvMask, h_prevPts, h_currPts, h_status);
#endif
            CHECK_STATUS(vpiImageSetWrappedOpenCVMat(imgTempFrame, h_frame1));
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, backend, imgTempFrame, imgFrame, NULL))
            CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(stream, backend, imgFrame, pyrCurrFrame, VPI_BORDER_CLAMP));
            CHECK_STATUS(vpiStreamSync(stream));

            int32_t nSize = h_currPts.size();
            VPIArrayData arraydata;
            arraydata.bufferType = VPI_ARRAY_BUFFER_HOST_AOS;
            arraydata.buffer.aos.type = VPI_ARRAY_TYPE_KEYPOINT_F32;
            arraydata.buffer.aos.capacity = nSize;
            arraydata.buffer.aos.sizePointer = &nSize;
            arraydata.buffer.aos.data = (VPIKeypointF32 *)h_currPts.data(); 
            CHECK_STATUS(vpiArrayCreateWrapper(&arraydata, 0, &currFeatures));

            h_frame0.copyTo(h_frame1);
        }

        AppTicToc tt;
        AppMeanStd dt1("us");
        AppMeanStd dt2("us");
        AppMeanStd dt3("us");
        for (uint32_t fn = 1; fn<prm->numFramesToProcess; fn++)
        {
            // Last iteration's current frame/features become this iteration's prev frame/features.
            // The former will contain information gathered in this iteration.
            std::swap(prevFeatures, currFeatures);
            std::swap(pyrPrevFrame, pyrCurrFrame);
            std::swap(h_prevPts, h_currPts);
            h_frame1.copyTo(h_frame0);

            // Save frame to disk
#if VIS_MODE == 1
            if (cfg.of->isDumpFlowImg)
            {
                vis.saveFileToDisk(h_frame0, cvMask, strOutputFiles, fn-1);
                printf("Frame id=%d: %d points tracked. \n", fn-1, numTrackedKeypoints);
            }
#endif

            loadImg(prm->inputImgDir, fn, h_frame1, h_frame1Gray);


            //upload image
            tt.tic();
            // Wrap frame into a VPIImage, reusing the existing imgFrame.
            CHECK_STATUS(vpiImageSetWrappedOpenCVMat(imgTempFrame, h_frame1));
            // Convert it to grayscale
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, backend, imgTempFrame, imgFrame, NULL))
            CHECK_STATUS(vpiStreamSync(stream));

            dt1.addSample(tt.toc());
            tt.tic();

            // Generate a pyramid out of it
            CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(stream, backend, imgFrame, pyrCurrFrame, VPI_BORDER_CLAMP));
            // Estimate the features' position in current frame given their position in previous frame
            CHECK_STATUS(vpiSubmitOpticalFlowPyrLK(stream, 0, optflow, pyrPrevFrame, pyrCurrFrame, prevFeatures,
                                                   currFeatures, status, &lkParams));
            // Wait for processing to finish.
            CHECK_STATUS(vpiStreamSync(stream));

            dt2.addSample(tt.toc());
            tt.tic();

            // Update the output mask
            downloadFeaturePts(currFeatures, MAX_KEYPOINTS, h_currPts);
            downloadStatus(status, h_status);
            dt3.addSample(tt.toc());


            int numTrackedKeypoints;
#if VIS_MODE == 1
            numTrackedKeypoints = vis.updateMask(cvMask, h_prevPts, h_currPts, h_status);
#else
            cv::Scalar line_color = cv::Scalar(0, 0, 255);
            numTrackedKeypoints = OptFlowVis::drawArrows(h_frame0, h_prevPts, h_currPts, h_status, line_color, goodStatus);
            if (cfg.of->isDumpFlowImg)
            {
                OptFlowVis::dumpImg(h_frame0, "nv-vpi-OF.jpg", fn-1);
            }
            if (cfg.of->isShowFlowImg)
            {
                imshow("nv-vpi-flow", h_frame0);
                int keyboard = waitKey(20);
                if (keyboard == 'q' || keyboard == 27)
                    break;
            }
#endif

            dumpFeaturePts("./", "fpt-nv_vpi", fn-1, h_prevPts, h_currPts, h_status);

            // No more keypoints being tracked?
            if (numTrackedKeypoints == 0)
            {
                printf("No keypoints to track.\n");
                break; // we can finish procesing.
            }

            std::vector<cv::Point2f> good_new;
            selectTrackedPts( good_new, h_currPts, h_status, goodStatus);     
            h_currPts = good_new;

            {
                int32_t nSize = h_currPts.size();
                VPIArrayData arraydata;
                arraydata.bufferType = VPI_ARRAY_BUFFER_HOST_AOS;
                arraydata.buffer.aos.type = VPI_ARRAY_TYPE_KEYPOINT_F32;
                arraydata.buffer.aos.capacity = nSize;
                arraydata.buffer.aos.sizePointer = &nSize;
                arraydata.buffer.aos.data = (VPIKeypointF32 *)h_currPts.data();
                CHECK_STATUS(vpiArrayCreateWrapper(&arraydata, 0, &currFeatures));
            }

        }  // end or loop
        dt1.calMeanStd();
        dt2.calMeanStd();
        dt3.calMeanStd();  
        printf("OF alg: NV_VPI: totFrame=%u, dt1(mu=%.1f, std=%.1f), dt2(mu=%.1f, std=%.1f), dt3(mu=%.1f, std=%.1f) (us)\n",
             prm->numFramesToProcess, dt1.mu,  dt1.std, dt2.mu,  dt2.std, dt3.mu,  dt3.std);
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retval = 1;
    }

    vpiStreamDestroy(stream);
    vpiPayloadDestroy(harris);
    vpiPayloadDestroy(optflow);

    vpiPyramidDestroy(pyrPrevFrame);
    vpiImageDestroy(imgTempFrame);
    vpiImageDestroy(imgFrame);
    vpiArrayDestroy(prevFeatures);
    vpiArrayDestroy(currFeatures);
    vpiArrayDestroy(status);
    vpiArrayDestroy(scores);
#endif
    return retval;
}

