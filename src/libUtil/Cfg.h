#ifndef __CFG_H__
#define __CFG_H__

#include "DataTypes.h"

namespace app {
	enum ofAlg{
		OF_LK_SPARSE_CV_CPU=0,
		OF_LK_SPARSE_CV_GPU,
		OF_LK_SPARSE_NV_VPI,
		OF_UTILS
	};

	static char *g_ofAlgName[]={"OF_LK_SPARSE_CV_CPU","OF_LK_SPARSE_CV_GPU", "OF_LK_SPARSE_NV_VPI", "OF_UTILS"};

	class CfgBase {
	public:
		CfgBase();
		CfgBase(const CfgBase &x) = default;
		CfgBase& operator = (const CfgBase &x) = default;
		virtual ~CfgBase() = default;

		virtual boost::property_tree::ptree toPropertyTree() const = 0;
		virtual void fromPropertyTree(const boost::property_tree::ptree &pt) = 0;
		virtual std::string toString() const = 0;
		static std::string toString(const boost::property_tree::ptree &pt); 
	};
	typedef std::shared_ptr<CfgBase>		CfgBasePtr;

	class CfgOf : public CfgBase
	{
	public:
		CfgOf();
		CfgOf(const CfgOf &x)=default;
		CfgOf &operator=(const CfgOf &x)=default;
		virtual ~CfgOf()=default;

		virtual boost::property_tree::ptree toPropertyTree() const;
		virtual void fromPropertyTree(const boost::property_tree::ptree &pt);
		virtual std::string toString() const;

		std::string getOutImgFilepath( const std::string &tag, int32_t frameNum ) const
		{
			std::ostringstream ss;
			ss 	<< outputImgDir  << "/"  << tag 
				<<  std::setfill('0') << std::setw(5) << frameNum << ".png";
			return ss.str();
		}
	public:
		//we only care about sparse in this project
	    const std::string flow_type{"sparse"};
		ofAlg   alg{app::OF_LK_SPARSE_NV_VPI};
		std::string  nv_vpi_backend{"gpu"};

		std::string inputImgDir{"."};
		std::string inputImgFileExt{"png"};
		bool		inputImgGray{true};
   		int32_t 	inputImgW{1920};
   		int32_t 	inputImgH{1920};

		int32_t 	initFeaturePts{100};

		bool 		isShowFlowImg{false};
		bool 		isDumpFlowImg{false};
		std::string outputImgDir{"."};

		int32_t numFramesToProcess{-1}; //if -1 we will process all
		int32_t resizedImgW{-1};   //if -1 we do not resize input image
		int32_t resizedImgH{-1};
    	bool 	useGray {true};   //if use gray image to calculate flow
    	int32_t winSize {21};     //1-d the windows size
    	int32_t maxPyrLevel{2};
    	int32_t iters{10};
		double   epsilon{0.03};
    	int32_t maxFeaturePoints{100};  	//# of points in good feature to track
    	double	minFeaturePointDist{10};	 	//min distance in good feature to track

		bool isForVincentDataSet={false};
		int ofTestSuite{0};
	};
	typedef std::shared_ptr<CfgOf> CfgOfPtr;



	class  CfgApp {
	public:
		CfgApp();
		CfgApp(const CfgApp &x) = default;
		CfgApp& operator = (const CfgApp &x) = default;
		~CfgApp()=default;

		void readFromFile(const char *fname);
		void writeToFile(const char *fname);
		std::string toString();

	private:
		boost::property_tree::ptree toPropertyTree();
		void fromPropertyTree(const boost::property_tree::ptree &pt) ;

	public:
		CfgOfPtr      of; 
	};

	typedef std::shared_ptr<CfgApp> CfgAppPtr;

}

#endif




