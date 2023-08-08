
#ifndef _OPT_FLOW_VIS_H_
#define _OPT_FLOW_VIS_H_

#include "DataTypes.h"
namespace app
{
    class OptFlowVis{
        public:
            //static void sortKeypoints(VPIArray keypoints, VPIArray scores, int max);
            static int  drawArrows(cv::Mat& frame, const std::vector<cv::Point2f>& prevPts, const std::vector<cv::Point2f>& nextPts, const std::vector<uchar>& status, 
                        const cv::Scalar &line_color = cv::Scalar(0, 0, 255), const uchar sucFlag=1 );

            static cv::Vec3b computeColor(float fx, float fy);
            static bool isFlowCorrect(cv::Point2f u);
            static void drawDenseFlow(cv::Mat& dst, const cv::Mat_<float>& flowx, const cv::Mat_<float>& flowy, const float maxmotion = -1);
            static void drawDenseFlow(cv::Mat& dst, const cv::cuda::GpuMat& d_flow, const float maxmotion = -1);

            static void printFeatures( const std::vector<cv::Point2f> &v );

            void static dumpImg( const cv::Mat &img, const std::string &baseFileName, const int32_t frameCounter);

        public:
            OptFlowVis( const int w, const int h, int maxFeatrurePointsToTrack_);

            void saveFileToDisk( const cv::Mat &img, const cv::Mat &cvMask, const std::string &baseFileName, const int32_t frameCounter);


            int updateMask(cv::Mat &cvMask, const std::vector<cv::Point2f> &prevFeatures,
                      const std::vector<cv::Point2f> &curFeatures, const std::vector<uchar> &status);

            int resetColor(const std::vector<cv::Point2f> &startingFrameFeatures );

        protected:
            int     imgW;
            int     imgH;
            int     maxFeatrurePointsToTrack{1000}; 
            cv::Mat cvMask;
            cv::Mat outImg;
            std::vector<cv::Scalar> trackColors;
    };
}
#endif


