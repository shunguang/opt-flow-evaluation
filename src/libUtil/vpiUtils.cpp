
#include "vpiUtils.h"

using namespace app;
using namespace std;
using namespace cv;

int calFeaturePointsByVpi(const cv::Mat &framergb_h, std::vector<cv::Point2f> &vFeaturePts_h)
{
    // VPI objects that will be used
    VPIStream stream = NULL;
    VPIImage imgTempFrame = NULL; // RGB
    VPIImage imgFrame = NULL;     // Gray
    VPIArray curFeatures = NULL;
    VPIArray scores = NULL;
    VPIPayload harris = NULL;
    
    int retval = 0;

    // =================================================
    // Allocate VPI resources and do some pre-processing

    // Create the stream where processing will happen.
    CHECK_STATUS(vpiStreamCreate(0, &stream));

    CHECK_STATUS(vpiImageCreateWrapperOpenCVMat(framergb_h, 0, &imgTempFrame));

    // Create grayscale image representation of input.
    CHECK_STATUS(vpiImageCreate(framergb_h.cols, framergb_h.rows, VPI_IMAGE_FORMAT_U8, 0, &imgFrame));

    // Create the image pyramids used by the algorithm

    // Create input and output arrays
    CHECK_STATUS(vpiArrayCreate(MAX_HARRIS_CORNERS, VPI_ARRAY_TYPE_KEYPOINT_F32, 0, &curFeatures));

    // Parameters we'll use. No need to change them on the fly, so just define them here.
    // We're using the default parameters.
    //VPIOpticalFlowPyrLKParams lkParams;
    //CHECK_STATUS(vpiInitOpticalFlowPyrLKParams(&lkParams));

    // Gather feature points from first frame using Harris Corners on CPU.
    CHECK_STATUS(vpiArrayCreate(MAX_HARRIS_CORNERS, VPI_ARRAY_TYPE_U32, 0, &scores));

    VPIHarrisCornerDetectorParams harrisParams;
    CHECK_STATUS(vpiInitHarrisCornerDetectorParams(&harrisParams));
    harrisParams.strengthThresh = 0;
    harrisParams.sensitivity = 0.01;

    CHECK_STATUS(vpiCreateHarrisCornerDetector(VPI_BACKEND_CPU, framergb_h.cols, framergb_h.rows, &harris));

    // Convert input to grayscale to conform with harris corner detector restrictions
    CHECK_STATUS(vpiSubmitConvertImageFormat(stream, VPI_BACKEND_CUDA, imgTempFrame, imgFrame, NULL));

    CHECK_STATUS(vpiSubmitHarrisCornerDetector(stream, VPI_BACKEND_CPU, harris, imgFrame, curFeatures, scores,
                                               &harrisParams));

    CHECK_STATUS(vpiStreamSync(stream));

    SortKeypoints(curFeatures, scores, MAX_KEYPOINTS);
    downloadFeaturePts(curFeatures, MAX_KEYPOINTS, vFeaturePts_h);

    OptFlowVis::printFeatures(vFeaturePts_h);

    vpiStreamDestroy(stream);
    vpiPayloadDestroy(harris);
    vpiImageDestroy(imgTempFrame);
    vpiImageDestroy(imgFrame);
    vpiArrayDestroy(curFeatures);
    vpiArrayDestroy(scores);

    return retval;
}

// Sort keypoints by decreasing score, and retain only the first 'max'
void SortKeypoints(VPIArray keypoints, VPIArray scores, int max)
{
    VPIArrayData ptsData, scoresData;
    CHECK_STATUS(vpiArrayLockData(keypoints, VPI_LOCK_READ_WRITE, VPI_ARRAY_BUFFER_HOST_AOS, &ptsData));
    CHECK_STATUS(vpiArrayLockData(scores, VPI_LOCK_READ_WRITE, VPI_ARRAY_BUFFER_HOST_AOS, &scoresData));

    VPIArrayBufferAOS &aosKeypoints = ptsData.buffer.aos;
    VPIArrayBufferAOS &aosScores = scoresData.buffer.aos;

    std::vector<int> indices(*aosKeypoints.sizePointer);
    std::iota(indices.begin(), indices.end(), 0);

    stable_sort(indices.begin(), indices.end(), [&aosScores](int a, int b)
                {
                    uint32_t *score = reinterpret_cast<uint32_t *>(aosScores.data);
                    return score[a] >= score[b]; // decreasing score order
                });

    // keep the only 'max' indexes.
    indices.resize(std::min<size_t>(indices.size(), max));

    VPIKeypointF32 *kptData = reinterpret_cast<VPIKeypointF32 *>(aosKeypoints.data);

    // reorder the keypoints to keep the first 'max' with highest scores.
    std::vector<VPIKeypointF32> kpt;
    std::transform(indices.begin(), indices.end(), std::back_inserter(kpt),
                   [kptData](int idx)
                   { return kptData[idx]; });
    std::copy(kpt.begin(), kpt.end(), kptData);

    // update keypoint array size.
    *aosKeypoints.sizePointer = kpt.size();

    vpiArrayUnlock(scores);
    vpiArrayUnlock(keypoints);
}

void downloadFeaturePts(const cv::cuda::GpuMat& d_mat, std::vector<cv::Point2f>& vec)
{
    assert( d_mat.rows == 1 ); 
#if 0    
    vec.resize(d_mat.cols/2);
    Mat mat(1, d_mat.cols, CV_32FC2, (void*)&vec[0]);
    d_mat.download(mat);
#else
    cv::Mat h_mat(1, d_mat.cols, CV_32FC2); 
    d_mat.download(h_mat);
    vec.clear();
    cv::Point2f pt;
    const float *p = (const float *) h_mat.ptr(0);
    for(int i=0; i<h_mat.cols/2; i++){
        pt.x = *p++;
        pt.y = *p++;
        vec.push_back(pt);
    }
#endif    
}

void downloadFeaturePts(const VPIArray curFeatures, const int maxNumPts, std::vector<cv::Point2f> &vFeaturePts_h)
{
    VPIArrayData ptsData;
    CHECK_STATUS(vpiArrayLockData(curFeatures, VPI_LOCK_READ, VPI_ARRAY_BUFFER_HOST_AOS, &ptsData));

    const VPIArrayBufferAOS &aosKeypoints = ptsData.buffer.aos;

    const VPIKeypointF32 *pts = (VPIKeypointF32 *)aosKeypoints.data;

    vFeaturePts_h.clear();
    cv::Point2f p0;
    int n = min(maxNumPts, *aosKeypoints.sizePointer);
    for (int i = 0; i < n; i++)
    {
        p0.x = pts[i].x;
        p0.y = pts[i].y;
        vFeaturePts_h.push_back(p0);
    }
    CHECK_STATUS(vpiArrayUnlock(curFeatures));
}

void downloadStatus(const cv::cuda::GpuMat& d_mat, vector<uint8_t>& vec)
{
    assert( d_mat.rows == 1 ); 

#if 0    
    vec.resize(d_mat.cols);
    Mat h_mat(1, d_mat.cols, CV_8UC1, (void*)&vec[0]);
    d_mat.download(h_mat);
#else
    cv::Mat h_mat(1, d_mat.cols, CV_8UC1); 
    d_mat.download(h_mat);

    vec.clear();
    const uint8_t *p = (const uint8_t *) h_mat.ptr(0);
    for(int i=0; i<h_mat.cols; i++){
        vec.push_back( *p++ );
    }
#endif

}

void downloadStatus(const VPIArray status, std::vector<uint8_t> &status_h)
{
    VPIArrayData statusData;
    CHECK_STATUS(vpiArrayLockData(status, VPI_LOCK_READ, VPI_ARRAY_BUFFER_HOST_AOS, &statusData));

    const VPIArrayBufferAOS &aos = statusData.buffer.aos;
    const uint8_t *pStatus = (uint8_t *)aos.data;
    int totKeypoints = *(aos.sizePointer);

    status_h.clear();
    for (int i = 0; i < totKeypoints; i++)
    {
        status_h.push_back(pStatus[i]);
    }
    CHECK_STATUS(vpiArrayUnlock(status));
}

void uploadFeaturePts(const vector<Point2f>& vec, cv::cuda::GpuMat& d_pts )
{
    int n = vec.size();
    Mat tmp(1, 2*n, CV_32FC2, (void*)&vec[0]);
    d_pts.upload( tmp );
}

void uploadFeaturePts(const std::vector<cv::Point2f> &vFeaturePts_h, VPIArray **curFeatures)
{
    int32_t nPoints = vFeaturePts_h.size();
    VPIArrayData arraydata;  //in host
    arraydata.bufferType      = VPI_ARRAY_BUFFER_HOST_AOS;
    arraydata.buffer.aos.type = VPI_ARRAY_TYPE_KEYPOINT_F32;
    arraydata.buffer.aos.capacity = nPoints;
    arraydata.buffer.aos.strideBytes = sizeof(VPIKeypointF32);
    arraydata.buffer.aos.sizePointer = &nPoints;
    arraydata.buffer.aos.data = (VPIKeypointF32 *)&vFeaturePts_h[0]; 
    CHECK_STATUS(vpiArrayCreateWrapper(&arraydata, 0, *curFeatures));
}

void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &vFeaturePts_h, const uchar defaultSatus )
{
    std::vector<uchar> status_h(vFeaturePts_h.size(), defaultSatus);
    dumpFeaturePts( dir, filenameTag, frmNum, vFeaturePts_h, status_h);
}

void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &vFeaturePts_h, const std::vector<uchar> &status_h)
{
    char buf[1024]={0};
    snprintf(buf,1023, "%s/%05d-%s.txt", dir.c_str(), frmNum, filenameTag.c_str() );
    //printf("buf=%s, vFeaturePts_h.size()=%d, status_h.size()=%d\n", buf, vFeaturePts_h.size(), status_h.size());

    FILE *fp = fopen(buf, "w");
    fprintf(fp, "id, featurePt.x, featurePt.y, status\n");
    for( int i=0; i<vFeaturePts_h.size(); i++){ 
        fprintf(fp, "%d,%.1f,%.1f,%d\n", i, vFeaturePts_h[i].x, vFeaturePts_h[i].y, status_h[i]);
    }
    fclose(fp);
    printf("suc dumped file: %s\n", buf);
}

void dumpFeaturePts(const std::string &dir,const std::string &filenameTag, const int frmNum, const std::vector<cv::Point2f> &vPreFeaturePts_h, 
    const std::vector<cv::Point2f> &vCurFeaturePts_h, const std::vector<uchar> &status_h)
{
    assert(vCurFeaturePts_h.size() == vPreFeaturePts_h.size() );

    char buf[1024]={0};
    snprintf(buf,1023, "%s/%05d-%s.txt", dir.c_str(), frmNum, filenameTag.c_str() );
    printf("buf=%s, vFeaturePts_h.size()=%d, status_h.size()=%d\n", buf, vCurFeaturePts_h.size(), status_h.size());

    FILE *fp = fopen(buf, "w");
    fprintf(fp, "id, prevFrmFeaturePt.x, prevFrmFeaturePt.y, currFrmFeaturePt.x, currFrmFeaturePt.y, status\n");
    for( int i=0; i<vCurFeaturePts_h.size(); i++){ 
        fprintf(fp, "%d,%.1f,%.1f,%.1f,%.1f,%d\n", i, vPreFeaturePts_h[i].x, vPreFeaturePts_h[i].y, vCurFeaturePts_h[i].x, vCurFeaturePts_h[i].y, status_h[i]);
    }
    fclose(fp);
    printf("suc dumped file: %s\n", buf);
}

void dumpFeaturePtsVincent(const std::string &dir,const std::string &filenameTag, const std::vector<cv::Point2f> &preFeatures_h, 
    const std::vector<cv::Point2f> &gtFeatures_h, const std::vector<cv::Point2f> &curFeatures_h, const std::vector<uchar> &status_h)
{
    char buf[1024]={0};
    snprintf(buf,1023, "%s/%s-results.txt", dir.c_str(), filenameTag.c_str() );
    printf("buf=%s, vFeaturePts_h.size()=%d, status_h.size()=%d\n", buf, curFeatures_h.size(), status_h.size());

    FILE *fp = fopen(buf, "w");
    fprintf(fp, "id, frame1.x, frame1.y, gt_frame2.x, gt_frame2.y, est_frame2.x, est_frame2.y, status\n");
    for( int i=0; i<preFeatures_h.size(); i++){ 
        fprintf(fp, "%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%d\n", i, preFeatures_h[i].x, preFeatures_h[i].y, gtFeatures_h[i].x, gtFeatures_h[i].y, curFeatures_h[i].x, curFeatures_h[i].y, status_h[i]);
    }
    fclose(fp);
    printf("suc dumped file: %s\n", buf);
}


void loadFeaturePts(const std::string &filename, const int nLines, std::vector<cv::Point2f> &vFeaturePts_h)
{
    char buf[1024];
    int cnt;
    cv::Point2f pt;
    int status;

    FILE *fp = fopen(filename.c_str(), "r");
    fgets(buf,1024, fp);
    //printf("buf=%s\n", buf);
    vFeaturePts_h.clear();
    for( int i=0; i<nLines; i++){ 
        fscanf(fp, "%d,%f,%f,%d\n", &cnt, &pt.x, &pt.y, &status);
        //printf("%d,%f,%f,%d\n", cnt, pt.x, pt.y, status);

        vFeaturePts_h.push_back( pt );
    }
    fclose(fp);
}


void loadFeaturePtsVincent(const std::string &filename, const int nLines, std::vector<cv::Point2f> &frm1_h, std::vector<cv::Point2f> &frm2_h)
{
    char buf[1024];
    int cnt,x1,y1,x2,y2;
    cv::Point2f p1,p2;

    FILE *fp = fopen(filename.c_str(), "r");
    //fgets(buf,1024, fp);
    //printf("buf=%s\n", buf);
    frm1_h.clear();
    frm2_h.clear();
    for( int i=0; i<nLines; i++){ 
        fscanf(fp, "%d %d %d %d\n", &x1, &y1, &x2, &y2);
        printf("%d, %d,%d,%d,%d\n", i+1, x1, y1, x2, y2);
        if(x1>0 && y1>0){
            p1.x = x1;
            p1.y = y1;
            p2.x = x2;
            p2.y = y2;
            frm1_h.push_back( p1);
            frm2_h.push_back( p2);
        }
    }
    fclose(fp);
}

void loadImgAndFeaturePts(
    const std::string &dir0,
    const int fn, 
                          const bool isVincent,
                          const int nLines,
                          Mat &h_frame0,
                          Mat &h_frame1,
                          Mat &h_frame0Gray,
                          Mat &h_frame1Gray,
                          vector<Point2f> &prevPts_h,
                          vector<Point2f> &gtPts_h)
{
    std::string fname0, fname1, fnameFpts;
#if 0    
    string dir0 = "/home/maven/dataset/opticalFlow/sparse/cars";
    if (isVincent)
    {
        dir0 = "/home/maven/dataset/opticalFlow/sparse/Vincent-GT";
    }
#endif

    if (isVincent)
    {
        fname0 = dir0 + "/classroom/" + "1.bmp";
        fname1 = dir0 + "/classroom/" + "2.bmp";
        fnameFpts = dir0 + "/classroom_gt.txt";
    }
    else
    {
        fname0 = dir0 + "/img/" + app::getFileName("ori-frm_", "png", fn, "%04d");
        fname1 = dir0 + "/img/" + app::getFileName("ori-frm_", "png", fn + 1, "%04d");
        fnameFpts = dir0 + "/fpt/" + app::getFileName2("-featurePts", "txt", fn, "%05d");
    }
    printf("fname0=%s\n", fname0.c_str());
    printf("fname1=%s\n", fname1.c_str());
    printf("fnameFpts=%s\n", fnameFpts.c_str());

    h_frame0 = cv::imread(fname0, cv::IMREAD_COLOR);
    h_frame1 = cv::imread(fname1, cv::IMREAD_COLOR);

    cv::cvtColor(h_frame0, h_frame0Gray, COLOR_BGR2GRAY);
    cv::cvtColor(h_frame1, h_frame1Gray, COLOR_BGR2GRAY);

    if (isVincent)
    {
        loadFeaturePtsVincent( fnameFpts, nLines, prevPts_h, gtPts_h );
        std::vector<uchar> status_h(prevPts_h.size(), 1);
        dumpFeaturePtsVincent(".",  "tmp-vincent-input-fpts", prevPts_h, gtPts_h, gtPts_h, status_h);
    }
    else{
        loadFeaturePts(fnameFpts, nLines, prevPts_h);
        // dumpFeaturePts( "tmp-fpts", fn, prevPts_h, 1 );
    }

}

void loadImg(const std::string &dir0, const int fn, Mat &h_frame, Mat &h_frameGray)
{
    std::string fname;
    fname = dir0 + "/img/" + app::getFileName("ori-frm_", "png", fn, "%04d");
    printf("fname=%s\n", fname.c_str());

    h_frame = cv::imread(fname, cv::IMREAD_COLOR);
    cv::cvtColor(h_frame, h_frameGray, COLOR_BGR2GRAY);
}


void selectTrackedPts(std::vector<cv::Point2f> &good_new, const std::vector<cv::Point2f> &vPts, const std::vector<uchar> &vStatus, const uint8_t goodStatus)
{
    good_new.clear();
    
    assert(vPts.size() == vStatus.size() );

    for (uint i = 0; i < vPts.size(); i++)
    {
        if (vStatus[i] == goodStatus)
        {
            good_new.push_back(vPts[i]);
        }
    }
    printf("good_new.size()=%d\n", good_new.size());
}
