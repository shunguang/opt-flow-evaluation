#include "CapImg.h"

using namespace std;
using namespace app;

CapImg ::CapImg( const std::string &dir_, const std::string &fileExt_, const bool isColor_)
: m_isFromVideoFile(false)
, m_dir(dir_)
, m_ext(fileExt_)
, m_isColor(isColor_)
{
    APP_LOG("CapImg::CapImg(): cap images from folder: %s", m_dir.c_str() );
    probe();
    APP_LOG( "isProbeSuccess()=%d", isProbeSuccess());
}

CapImg :: CapImg( const std::string &videoFile_)
: m_isFromVideoFile(true)
, m_videoFile(videoFile_)
{
    APP_LOG("CapImg::CapImg(): cap image from video file:%s", m_videoFile.c_str() );
    m_cvCap = std::make_shared<cv::VideoCapture>(videoFile_);
    if (!m_cvCap->isOpened()){
        m_cvCap = nullptr;
        //error in opening the video input
        APP_LOG("CapImg::CapImg(): video file cannot be opened!");
    }
    else{
      probe();
    }
}

CapImg ::~CapImg()
{
  if( m_isFromVideoFile ){
    m_cvCap->release();
  }
}

bool CapImg::getNextFrame(cv::Mat &I)
{
  if( m_isFromVideoFile ){
      m_cvCap->read( I );
      if (I.empty()){
        return false;
      }
  }
  else{
    //from an image folder
    if ( m_nextFrmIdx>=m_nTotFrames ){
      return false;
    }

    std::string curImgFile = m_dir + "/" + m_vImgFileNames[m_nextFrmIdx];

    APP_LOG( "curImgFile=%s, m_isColor=%d", curImgFile.c_str(), m_isColor);

    I = cv::imread(curImgFile, m_isColor ? cv::IMREAD_COLOR : cv::IMREAD_COLOR);
    if (I.cols != m_imgW || I.rows != m_imgH){
      APP_LOG("apImg::getNextFrame(): wrong iamge Size");
    }
  }

  m_nextFrmIdx++;
  return true;
}


void CapImg::probe()
{
  if(m_isFromVideoFile && m_cvCap){
    m_imgW = m_cvCap->get( cv::CAP_PROP_FRAME_WIDTH  );
    m_imgH = m_cvCap->get( cv::CAP_PROP_FRAME_HEIGHT );
    m_nTotFrames = m_cvCap->get( cv::CAP_PROP_FRAME_COUNT );
  }
  else{
    m_nTotFrames = getFileNameList( m_dir, m_ext, m_vImgFileNames);
    if( m_nTotFrames > 0 ){
      std::string  curImgFile = m_dir + "/" + m_vImgFileNames[m_nextFrmIdx];
      APP_LOG( "curImgFile=%s\n", curImgFile.c_str());

      cv::Mat I = cv::imread( curImgFile, m_isColor ? cv::IMREAD_COLOR : cv::IMREAD_GRAYSCALE );
      m_imgW = I.cols;
      m_imgH = I.rows;
    }
  }
  APP_LOG( "CapImg::probe(): m_nTotFrames=%d, m_imgW=%d, m_imgH=%d, m_nextFrmIdx=%d\n", m_nTotFrames, m_imgW, m_imgH, m_nextFrmIdx) ;
}

bool CapImg::rewind()
{
    if(m_isFromVideoFile && m_cvCap){
      m_cvCap->set(cv::CAP_PROP_POS_FRAMES, 0);
    }
    m_nextFrmIdx = 0;
}
