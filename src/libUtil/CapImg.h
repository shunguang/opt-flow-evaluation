#ifndef _CAP_IMG_H_
#define _CAP_IMG_H_

#include "libUtil/util.h""
namespace app
{
    class CapImg
	{
    public:
        CapImg( const std::string &dir_, const std::string &fileExt_, const bool isColor_);
        CapImg( const std::string &videoFile_);
        ~CapImg();

        bool isProbeSuccess() const {
            return (m_nTotFrames>0);
        } 
        
        bool getNextFrame( cv::Mat &I );
        bool rewind();
        
    private:
        void probe();

    public:        
        bool        m_isFromVideoFile={false};
        std::string m_dir{"."};
        std::string m_ext{"png"};
        std::string m_videoFile{""};

        bool        m_isColor{true};
        uint32_t    m_nTotFrames{0};
        uint32_t    m_nextFrmIdx{0};
        uint32_t    m_imgW{0};
        uint32_t    m_imgH{0};
        

        std::shared_ptr<cv::VideoCapture> m_cvCap{nullptr};
        std::vector<std::string> m_vImgFileNames{};
    };

    typedef std::shared_ptr<CapImg> CapImgPtr;
}

#endif
