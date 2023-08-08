#include "Cfg.h"

using namespace std;
using namespace app;
CfgBase::CfgBase()
{
}


std::string CfgBase::toString(const boost::property_tree::ptree &pt)
{
	std::ostringstream oss;
	boost::property_tree::xml_parser::xml_writer_settings<std::string> settings(' ', 4);
	boost::property_tree::xml_parser::write_xml(oss, pt, settings);

	return oss.str();
}


//-----------------CfgOf ----------------------
CfgOf::CfgOf()
: CfgBase()
{
}


void CfgOf::fromPropertyTree(const boost::property_tree::ptree &pt)
{
	alg = (app::ofAlg)pt.get<int32_t>("alg");
	nv_vpi_backend=pt.get<std::string>("nv_vpi_backend");
	
	inputImgDir = pt.get<std::string>("inputImgDir");
	inputImgFileExt = pt.get<std::string>("inputImgFileExt");
	inputImgGray=(bool)pt.get<uint32_t>("inputImgGray");
	inputImgW=pt.get<int32_t>("inputImgW");
	inputImgH=pt.get<int32_t>("inputImgH");
	numFramesToProcess = pt.get<uint32_t>("numFramesToProcess");

	initFeaturePts=pt.get<int32_t>("initFeaturePts");

	isShowFlowImg = (bool)pt.get<int32_t>("isShowFlowImg");
	isDumpFlowImg = (bool)pt.get<int32_t>("isDumpFlowImg");
	ofTestSuite = pt.get<int32_t>("ofTestSuite");
	outputImgDir = pt.get<std::string>("outputImgDir");

	resizedImgW = pt.get<int32_t>("resizedImgW");
	resizedImgH = pt.get<int32_t>("resizedImgH");
	useGray = (bool)pt.get<int32_t>("useGray");
	winSize = pt.get<int32_t>("winSize");
	maxPyrLevel = pt.get<int32_t>("maxPyrLevel");
	iters = pt.get<int32_t>("iters");
	epsilon=pt.get<double>("epsilon");
	maxFeaturePoints = pt.get<int32_t>("maxFeaturePoints");
	minFeaturePointDist = pt.get<int32_t>("minFeaturePointDist");

	isForVincentDataSet = (bool)pt.get<int>("isForVincentDataSet");
}

boost::property_tree::ptree CfgOf::toPropertyTree() const
{
	boost::property_tree::ptree pt;

	pt.put("alg", (int)alg);
	pt.put("nv_vpi_backend",  nv_vpi_backend);

	pt.put("inputImgDir", inputImgDir);
	pt.put("inputImgFileExt", inputImgFileExt);
	pt.put("inputImgGray", (int)inputImgGray);
	pt.put("inputImgW", (int)inputImgW);
	pt.put("inputImgH", (int)inputImgH);
	
	pt.put("initFeaturePts", initFeaturePts);

	pt.put("numFramesToProcess", numFramesToProcess);
	pt.put("ofTestSuite", ofTestSuite);
	pt.put("isShowFlowImg",  isShowFlowImg);
	pt.put("isDumpFlowImg",  isDumpFlowImg);
	pt.put("outputImgDir",  outputImgDir);

	pt.put("resizedImgW",  resizedImgW);
	pt.put("resizedImgH",  resizedImgH);
	pt.put("useGray",  useGray);
	pt.put("winSize",  winSize);
	pt.put("maxPyrLevel",  maxPyrLevel);
	pt.put("iters",  iters);
	pt.put("epsilon",  epsilon);
	
	pt.put("maxFeaturePoints",  maxFeaturePoints);
	pt.put("minFeaturePointDist",  minFeaturePointDist);

	pt.put("isForVincentDataSet", (int)isForVincentDataSet);

	return pt;
}

std::string CfgOf::toString() const
{
	boost::property_tree::ptree pt = toPropertyTree();
	return CfgBase::toString(pt);
}


//-------------- CfgApp --------------------
CfgApp::CfgApp()
	: of( new CfgOf() )
{
}

void CfgApp::readFromFile(const char *fname)
{
	boost::property_tree::ptree pt;
	boost::property_tree::xml_parser::read_xml( fname, pt);
	this->fromPropertyTree(pt.get_child("cfg"));
}

void CfgApp::writeToFile(const char *fname)
{
	boost::property_tree::ptree pt = toPropertyTree();
	boost::property_tree::xml_parser::xml_writer_settings<std::string> settings(' ', 4);
	boost::property_tree::xml_parser::write_xml(fname, pt, std::locale(), settings);
}

std::string CfgApp::toString()
{
	boost::property_tree::ptree pt = toPropertyTree();

	std::ostringstream oss;
	boost::property_tree::xml_parser::xml_writer_settings<std::string> settings(' ', 4);
	boost::property_tree::xml_parser::write_xml(oss, pt, settings);
	return oss.str();
}

void CfgApp::fromPropertyTree(const boost::property_tree::ptree &pt)
{
	of->fromPropertyTree(pt.get_child("of"));
}


boost::property_tree::ptree CfgApp::toPropertyTree()
{
	boost::property_tree::ptree ptOf = of->toPropertyTree();

	boost::property_tree::ptree pt;

	pt.add_child("cfg.of", ptOf);
	return pt;
}
