//------------------------------------------------------------------------------
// test_of_nv_vpi_1frm.cpp
// compute multi-frm optical flow by nv vpi alg: vpiSubmitOpticalFlowPyrLK()
// accuracy oriented, do not care about efficency
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

int test_of_nv_vpi_1frm(const CfgApp &cfg)
{
    // testVpi();
    // return 0;

    const CfgOfPtr prm = cfg.of;
    const bool isVincent = prm->isForVincentDataSet;

    // VPI objects that will be used
    VPIStream stream = NULL;
    VPIImage d_frameRgbTmp = NULL;
    VPIImage d_frameGrayTmp = NULL;
    VPIPyramid pyrPrevFrame = NULL, pyrCurrFrame = NULL;
    VPIArray currFeatures = NULL;
    VPIPayload optflow = NULL;

    std::vector<cv::Point2f> prevFeatures_h, curFeatures_h, gtFeatures_h;
    std::vector<uchar> status_h;

    int retval = 0;

    std::string strBackend = prm->nv_vpi_backend;
    VPIBackend backend = (strBackend == "gpu") ? VPI_BACKEND_CUDA : VPI_BACKEND_CPU;
    int32_t pyrLevel = prm->maxPyrLevel;

    int nLines = prm->initFeaturePts;
    int w0 = prm->inputImgW;
    int h0 = prm->inputImgH;

    cv::Size frameSize(w0, h0);
    Mat h_frame0(frameSize, CV_8UC3);
    Mat h_frame1(frameSize, CV_8UC3);
    Mat h_frame0Gray, h_frame1Gray;

    try
    {
        // Create the stream where processing will happen.
        CHECK_STATUS(vpiStreamCreate(0, &stream));

        CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(h_frame0, 0, &d_frameRgbTmp));

        // Create grayscale image representation of input.
        CHECK_STATUS(vpiImageCreate(w0, h0, VPI_IMAGE_FORMAT_U8, 0, &d_frameGrayTmp));

        // Create the image pyramids used by the algorithm
        CHECK_STATUS(vpiPyramidCreate(w0, h0, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5, 0, &pyrPrevFrame));
        CHECK_STATUS(vpiPyramidCreate(w0, h0, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5, 0, &pyrCurrFrame));

        // Create input and output arrays
        // CHECK_STATUS(vpiArrayCreate(MAX_KEYPOINTS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &prevFeatures));
        CHECK_STATUS(vpiArrayCreate(MAX_KEYPOINTS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &currFeatures));

        // Create Optical Flow payload
        CHECK_STATUS(vpiCreateOpticalFlowPyrLK(backend, w0, h0, VPI_IMAGE_FORMAT_U8, pyrLevel, 0.5, &optflow));

        // Parameters we'll use. No need to change them on the fly, so just define them here.
        // We're using the default parameters.
        VPIOpticalFlowPyrLKParams lkParams;
        lkParams.epsilon = prm->epsilon;
        lkParams.epsilonType = VPI_LK_ERROR_L1;
        lkParams.numIterations = prm->iters;
        lkParams.useInitialFlow = 0;
        lkParams.windowDimension = prm->winSize;
        lkParams.termination = VPI_TERMINATION_CRITERIA_ITERATIONS | VPI_TERMINATION_CRITERIA_EPSILON;
        CHECK_STATUS(vpiInitOpticalFlowPyrLKParams(&lkParams));

        for (int fn = 0; fn < (prm->numFramesToProcess - 1); fn++)
        {

            loadImgAndFeaturePts(prm->inputImgDir, fn, isVincent, nLines, h_frame0, h_frame1, h_frame0Gray, h_frame1Gray, prevFeatures_h, gtFeatures_h);

            // prepare <prevFeatures>
            VPIArray prevFeatures = NULL;

#if 1
            int32_t nSize = prevFeatures_h.size();
            VPIArrayData arraydata;
            arraydata.bufferType = VPI_ARRAY_BUFFER_HOST_AOS;
            arraydata.buffer.aos.type = VPI_ARRAY_TYPE_KEYPOINT_F32;
            arraydata.buffer.aos.capacity = nSize;
            arraydata.buffer.aos.sizePointer = &nSize;
            arraydata.buffer.aos.data = (VPIKeypointF32 *)prevFeatures_h.data(); // Work
            CHECK_STATUS(vpiArrayCreateWrapper(&arraydata, 0, &prevFeatures));
#else
            // todo: why dose not work
            uploadFeaturePts(prevFeatures_h, &prevFeatures);          
#endif

#if 0
{
            std::vector<cv::Point2f> vTmp_h;
            downloadFeaturePts(prevFeatures, MAX_KEYPOINTS, vTmp_h);
            dumpFeaturePts("./", "tmp-fpt", fn, vTmp_h, 1);
}
#endif

            // dumpFeaturePts("./", "tmp-fpts", fn, prevPts_h, 1 );
            VPIArray status = NULL;
            CHECK_STATUS(vpiArrayCreate(MAX_KEYPOINTS, VPI_ARRAY_TYPE_U8, 0, &status));

            // create pyrPrevFrame
            CHECK_STATUS(vpiImageSetWrappedOpenCVMat(d_frameRgbTmp, h_frame0));
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, backend, d_frameRgbTmp, d_frameGrayTmp, NULL))
            CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(stream, backend, d_frameGrayTmp, pyrPrevFrame, VPI_BORDER_CLAMP));
            CHECK_STATUS(vpiStreamSync(stream));

            // create pyrCurrFrame
            CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(h_frame1, 0, &d_frameRgbTmp));
            CHECK_STATUS(vpiImageSetWrappedOpenCVMat(d_frameRgbTmp, h_frame1));
            CHECK_STATUS(vpiSubmitConvertImageFormat(stream, backend, d_frameRgbTmp, d_frameGrayTmp, NULL))
            CHECK_STATUS(vpiSubmitGaussianPyramidGenerator(stream, backend, d_frameGrayTmp, pyrCurrFrame, VPI_BORDER_CLAMP));
            CHECK_STATUS(vpiStreamSync(stream));

            // Estimate the features' position in current frame given their position in previous frame
            CHECK_STATUS(vpiSubmitOpticalFlowPyrLK(stream, backend, optflow, pyrPrevFrame, pyrCurrFrame, prevFeatures,
                                                   currFeatures, status, &lkParams));
            // Wait for processing to finish.
            CHECK_STATUS(vpiStreamSync(stream));

            // Update the output mask
            downloadFeaturePts(currFeatures, MAX_KEYPOINTS, curFeatures_h);
            downloadStatus(status, status_h);

            if (isVincent)
            {
                dumpFeaturePtsVincent(prm->outputImgDir, "classRoom-result-nv-vpi", prevFeatures_h, gtFeatures_h, curFeatures_h, status_h);
            }
            else
            {
                dumpFeaturePts(prm->outputImgDir, "fpt-nv-vpi", fn + 1, prevFeatures_h, curFeatures_h, status_h);
            }

            vpiArrayDestroy(prevFeatures);
            vpiArrayDestroy(status);

            if (isVincent)
            {
                break;
            }
        } // while(1)
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        retval = 1;
    }

    //----------------------------------------
    vpiPayloadDestroy(optflow);
    vpiPyramidDestroy(pyrPrevFrame);
    vpiPyramidDestroy(pyrCurrFrame);
    vpiImageDestroy(d_frameRgbTmp);
    vpiImageDestroy(d_frameGrayTmp);
    vpiArrayDestroy(currFeatures);
    //----------------------------------------

    vpiStreamDestroy(stream);

    return retval;
}

