#include "OptFlowVis.h"

using namespace std;
using namespace cv;
using namespace app;
 OptFlowVis::OptFlowVis( const int w, const int h, int maxFeatrurePointsToTrack_)
    : imgW(w)
    , imgH(h)
    , maxFeatrurePointsToTrack( maxFeatrurePointsToTrack_)
    , cvMask(h, w, CV_8UC3, cv::Scalar(0,0,0))
    , outImg(h, w, CV_8UC3, cv::Scalar(0,0,0))
    , trackColors()
 {
     std::vector<cv::Vec3b> tmpTrackColors;

     for (int i = 0; i < maxFeatrurePointsToTrack; i++)
     {
         // Track hue depends on its initial position
         int x = rand() % imgW;
         int y = rand() % imgH;
         int hue = (x ^ y) % 180;
         tmpTrackColors.push_back(cv::Vec3b(hue, 255, 255));
     }

     cvtColor(tmpTrackColors, tmpTrackColors, cv::COLOR_HSV2BGR);

     for (size_t i = 0; i < tmpTrackColors.size(); i++)
     {
         trackColors.push_back(cv::Scalar(tmpTrackColors[i]));
     }
 }

 int OptFlowVis::resetColor(const std::vector<cv::Point2f> &startingFrameFeatures)
 {
     const std::vector<cv::Point2f> &curFeatures = startingFrameFeatures;
     // Create some random colors
     trackColors.clear();
     {
         std::vector<cv::Vec3b> tmpTrackColors;
         for (int i = 0; i < curFeatures.size(); i++)
         {
             // Track hue depends on its initial position
             int hue = ((int)curFeatures[i].x ^ (int)curFeatures[i].y) % 180;

             tmpTrackColors.push_back(cv::Vec3b(hue, 255, 255));
         }

         cvtColor(tmpTrackColors, tmpTrackColors, cv::COLOR_HSV2BGR);

         for (size_t i = 0; i < tmpTrackColors.size(); i++)
         {
             trackColors.push_back(cv::Scalar(tmpTrackColors[i]));
         }
     }
 }

void OptFlowVis::dumpImg( const cv::Mat &img, const std::string &baseFileName, const int32_t frameCounter)
{
     // Create the output file name
     std::string fname = baseFileName;
     int ext = fname.rfind('.');

     char buffer[512] = {};
     snprintf(buffer, sizeof(buffer) - 1, "%s_%04d%s", fname.substr(0, ext).c_str(), frameCounter,
              fname.substr(ext).c_str());

     printf("buffer=%s\n", buffer);
     // Finally, write frame to disk
     if (!imwrite(buffer, img, {cv::IMWRITE_JPEG_QUALITY, 85}))
     {
         printf("Can't write to file: %s\n", buffer);
     }
}

 void OptFlowVis::saveFileToDisk(const cv::Mat &img, const cv::Mat &cvMask, const std::string &baseFileName, const int32_t frameCounter)
 {
     if (img.channels() == 1)
     {
         cvtColor(img, outImg, cv::COLOR_GRAY2BGR);
     }
     else
     {
         img.copyTo(outImg);
     }

     cv::add(outImg, cvMask, outImg);

     // Create the output file name
     OptFlowVis::dumpImg( outImg, baseFileName, frameCounter);
 }

 int OptFlowVis::updateMask(cv::Mat &cvMask, const std::vector<cv::Point2f> &prevFeatures,
                            const std::vector<cv::Point2f> &curFeatures, const std::vector<uchar> &status)
 {
     // Now that optical flow is completed, there are usually two approaches to take:
     // 1. Add new feature points from current frame using a feature detector such as
     //    \ref algo_harris_corners "Harris Corner Detector"
     // 2. Keep using the points that are being tracked.
     //
     // The sample app uses the valid feature point and continue to do the tracking.

     // Lock the input and output arrays to draw the tracks to the output mask.

     int numTrackedKeypoints = 0;
     int totKeypoints = status.size();
     bool hasPreFeatures = !prevFeatures.empty();
     for (int i = 0; i < totKeypoints; i++)
     {
         // keypoint is being tracked?
         if (status[i] == 0)
         {
             int colorId = i % maxFeatrurePointsToTrack;

             // draw the tracks
             cv::Point curPoint{(int)round(curFeatures[i].x), (int)round(curFeatures[i].y)};
             if (hasPreFeatures)
             {
                 cv::Point2f prevPoint{prevFeatures[i].x, prevFeatures[i].y};
                 cv::line(cvMask, prevPoint, curPoint, colorId, 2);
             }

             cv::circle(cvMask, curPoint, 5, colorId, -1);

             numTrackedKeypoints++;
         }
     }
     return numTrackedKeypoints;
 };

 int OptFlowVis::drawArrows(Mat &frame, const vector<Point2f> &prevPts, const vector<Point2f> &nextPts, const vector<uchar> &status, const Scalar &line_color, const uchar sucFlag)
 {
     int numTrackedKeypoints = 0;
     for (size_t i = 0; i < prevPts.size(); ++i)
     {
         if (sucFlag==status[i])
         {
             int line_thickness = 1;

             Point p = prevPts[i];
             Point q = nextPts[i];

             double angle = atan2((double)p.y - q.y, (double)p.x - q.x);

             double hypotenuse = sqrt((double)(p.y - q.y) * (p.y - q.y) + (double)(p.x - q.x) * (p.x - q.x));

             if (hypotenuse < 1.0)
                 continue;

             // Here we lengthen the arrow by a factor of three.
             q.x = (int)(p.x - 6 * hypotenuse * cos(angle));
             q.y = (int)(p.y - 6 * hypotenuse * sin(angle));

             // Now we draw the main line of the arrow.
             line(frame, p, q, line_color, line_thickness);

             // Now draw the tips of the arrow. I do some scaling so that the
             // tips look proportional to the main line of the arrow.

             p.x = (int)(q.x + 18 * cos(angle + CV_PI / 4));
             p.y = (int)(q.y + 18 * sin(angle + CV_PI / 4));
             line(frame, p, q, line_color, line_thickness);

             p.x = (int)(q.x + 18 * cos(angle - CV_PI / 4));
             p.y = (int)(q.y + 18 * sin(angle - CV_PI / 4));
             line(frame, p, q, line_color, line_thickness);

             numTrackedKeypoints++;
         }
     }
     return numTrackedKeypoints;
 }

//----------------------------------------------------------------------
// Tools for dense flow
//----------------------------------------------------------------------
 Vec3b OptFlowVis::computeColor(float fx, float fy)
 {
     static bool first = true;

     // relative lengths of color transitions:
     // these are chosen based on perceptual similarity
     // (e.g. one can distinguish more shades between red and yellow
     //  than between yellow and green)
     const int RY = 15;
     const int YG = 6;
     const int GC = 4;
     const int CB = 11;
     const int BM = 13;
     const int MR = 6;
     const int NCOLS = RY + YG + GC + CB + BM + MR;
     static Vec3i colorWheel[NCOLS];

     if (first)
     {
         int k = 0;

         for (int i = 0; i < RY; ++i, ++k)
             colorWheel[k] = Vec3i(255, 255 * i / RY, 0);

         for (int i = 0; i < YG; ++i, ++k)
             colorWheel[k] = Vec3i(255 - 255 * i / YG, 255, 0);

         for (int i = 0; i < GC; ++i, ++k)
             colorWheel[k] = Vec3i(0, 255, 255 * i / GC);

         for (int i = 0; i < CB; ++i, ++k)
             colorWheel[k] = Vec3i(0, 255 - 255 * i / CB, 255);

         for (int i = 0; i < BM; ++i, ++k)
             colorWheel[k] = Vec3i(255 * i / BM, 0, 255);

         for (int i = 0; i < MR; ++i, ++k)
             colorWheel[k] = Vec3i(255, 0, 255 - 255 * i / MR);

         first = false;
     }

     const float rad = sqrt(fx * fx + fy * fy);
     const float a = atan2(-fy, -fx) / (float)CV_PI;

     const float fk = (a + 1.0f) / 2.0f * (NCOLS - 1);
     const int k0 = static_cast<int>(fk);
     const int k1 = (k0 + 1) % NCOLS;
     const float f = fk - k0;

     Vec3b pix;

     for (int b = 0; b < 3; b++)
     {
         const float col0 = colorWheel[k0][b] / 255.0f;
         const float col1 = colorWheel[k1][b] / 255.0f;

         float col = (1 - f) * col0 + f * col1;

         if (rad <= 1)
             col = 1 - rad * (1 - col); // increase saturation with radius
         else
             col *= .75; // out of range

         pix[2 - b] = static_cast<uchar>(255.0 * col);
     }

     return pix;
 }

bool OptFlowVis::isFlowCorrect(Point2f u)
 {
     return !cvIsNaN(u.x) && !cvIsNaN(u.y) && fabs(u.x) < 1e9 && fabs(u.y) < 1e9;
 }

 void OptFlowVis::drawDenseFlow(Mat &dst, const Mat_<float> &flowx, const Mat_<float> &flowy, float maxmotion)
 {
     dst.create(flowx.size(), CV_8UC3);
     dst.setTo(Scalar::all(0));

     // determine motion range:
     float maxrad = maxmotion;

     if (maxmotion <= 0)
     {
         maxrad = 1;
         for (int y = 0; y < flowx.rows; ++y)
         {
             for (int x = 0; x < flowx.cols; ++x)
             {
                 Point2f u(flowx(y, x), flowy(y, x));

                 if (!isFlowCorrect(u))
                     continue;

                 maxrad = max(maxrad, sqrt(u.x * u.x + u.y * u.y));
             }
         }
     }

     for (int y = 0; y < flowx.rows; ++y)
     {
         for (int x = 0; x < flowx.cols; ++x)
         {
             Point2f u(flowx(y, x), flowy(y, x));

             if (isFlowCorrect(u))
                 dst.at<Vec3b>(y, x) = OptFlowVis::computeColor(u.x / maxrad, u.y / maxrad);
         }
     }
 }

void OptFlowVis::drawDenseFlow(Mat &dst, const cv::cuda::GpuMat& d_flow, const float maxmotion )
{
    cv::cuda::GpuMat planes[2];
    cuda::split(d_flow, planes);

    Mat flowx(planes[0]);
    Mat flowy(planes[1]);

    OptFlowVis::drawDenseFlow(dst, flowx, flowy, maxmotion);

    //imwrite("/home/maven/wus1/opt-flow-evaluation/build/bin/tmp.jpg", out);
    //imshow(name, out);
}


void OptFlowVis::printFeatures( const  std::vector<cv::Point2f> &v )
{
    int i=0;
    for( const cv::Point2f &p : v) {
        cout << "i=" << i++ << ":" << p.x <<", " << p.y << endl;
    }
}

