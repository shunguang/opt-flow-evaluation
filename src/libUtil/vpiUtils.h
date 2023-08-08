#ifndef _VPI_UTILS_H__
#define _VPI_UTILS_H__ 1

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

//using namespace app;
//using namespace std;
//using namespace cv;

// Max number of corners detected by harris corner algo
constexpr int MAX_HARRIS_CORNERS = 8192;

// Max number of keypoints to be tracked
constexpr int MAX_KEYPOINTS = 300;

#define CHECK_STATUS(STMT)                                      \
    do                                                          \
    {                                                           \
        VPIStatus status__ = (STMT);                            \
        if (status__ != VPI_SUCCESS)                            \
        {                                                       \
            char buffer[VPI_MAX_STATUS_MESSAGE_LENGTH];         \
            vpiGetLastStatusMessage(buffer, sizeof(buffer));    \
            std::ostringstream ss;                              \
            ss << vpiStatusGetName(status__) << ": " << buffer; \
            throw std::runtime_error(ss.str());                 \
        }                                                       \
    } while (0);

void SortKeypoints(VPIArray keypoints, VPIArray scores, int max);

void downloadFeaturePts(const cv::cuda::GpuMat& d_mat, std::vector<cv::Point2f>& vec);
void downloadFeaturePts(const VPIArray curFeatures, const int maxNumPts, std::vector<cv::Point2f> &pts);

void downloadStatus(const VPIArray status, std::vector<uint8_t> &status_h);
void downloadStatus(const cv::cuda::GpuMat& d_mat, std::vector<uint8_t>& status_h);

void uploadFeaturePts(const std::vector<cv::Point2f>& vec, cv::cuda::GpuMat& d_pts );
void uploadFeaturePts(const std::vector<cv::Point2f> &curFeatures_h, VPIArray **curFeatures);

int calFeaturePointsByVpi(const cv::Mat &frameRgb_h, std::vector<cv::Point2f> &curFeatures_h);

void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &curFeatures_h, const uchar defaultSatus );
void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &curFeatures_h, const std::vector<uint8_t> &status_h );
void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &preFeatures_h,const std::vector<cv::Point2f> &curFeatures_h, const std::vector<uint8_t> &status_h );

void dumpFeaturePtsVincent(const std::string &dir,const std::string &filenameTag, const std::vector<cv::Point2f> &preFeatures_h, const std::vector<cv::Point2f> &gtFeatures_h, const std::vector<cv::Point2f> &curFeatures_h, const std::vector<uchar> &status_h);

void loadFeaturePts(const std::string &filename, const int nLines, std::vector<cv::Point2f> &curFeatures_h);
void loadFeaturePtsVincent(const std::string &filename, const int nLines, std::vector<cv::Point2f> &frm1_h, std::vector<cv::Point2f> &frm2_h);

void loadImg( const std::string &dir0,const int fn, cv::Mat &h_frame, cv::Mat &h_frame0Gray);

void loadImgAndFeaturePts( const std::string &dir0,const int fn, const bool isVincent,
    const int nLines,
    cv::Mat &h_frame0,
    cv::Mat &h_frame1,
    cv::Mat &h_frame0Gray,
    cv::Mat &h_frame1Gray,
    std::vector<cv::Point2f> &prevPts_h, std::vector<cv::Point2f> &currPts_h);

void selectTrackedPts(std::vector<cv::Point2f> &good_new, const std::vector<cv::Point2f> &vPts, const std::vector<uchar> &vStatus, const uint8_t goodStatus);

#endif

