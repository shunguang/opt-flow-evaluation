/*
 * main.cpp
 *
 *  Created on: Nov 18, 2022
 *      Author: wus1
 */
#include "libUtil/util.h"

using namespace std;
using namespace app;

int test_utils(int argc, char *argv[]);
int test_of_cv_cpu_timing(const CfgApp &cfg);
int test_of_cv_gpu_timing(const CfgApp &cfg);
int test_of_nv_vpi_timing(const CfgApp &cfg);

int test_of_cv_cpu_1frm(const CfgApp &cfg);
int test_of_cv_gpu_1frm(const CfgApp &cfg);
int test_of_nv_vpi_1frm(const CfgApp &cfg);

int test_of_cv_cpu_multi_frm(const CfgApp &cfg);
int test_of_cv_gpu_multi_frm(const CfgApp &cfg);
int test_of_nv_vpi_multi_frm(const CfgApp &cfg);

//int pyrlk_optical_flow(int argc, char* argv[]);
int test_cal_feature_pts_and_save(const CfgApp &cfg0);
void app_usage();

int main(int argc, char *argv[])
{
    int x = 0;
    app_usage();
    if (argc < 2)
    {
        printf("to few input params\n");
        return -1;
    }

    //---set lo gfile -----
    const std::string logFilename = "./log.txt";
    std::cout << logFilename << std::endl;
    startLogThread(logFilename, true);
    dumpLog("---log start---");

    //--- read cfg -----
    const std::string cfgFile(argv[1]);
    CfgApp cfg;
    if (!fileExists(cfgFile))
    {
        appExit("Cfg file <%s> does not exist!", cfgFile.c_str());
    }
    cfg.readFromFile(cfgFile.c_str());
    cout << "cfg=" << cfg.toString() << endl;

    if (0==cfg.of->ofTestSuite )
    {
        switch (cfg.of->alg)
        {
        case OF_LK_SPARSE_CV_CPU:
            x = test_of_cv_cpu_1frm(cfg);
            break;
        case OF_LK_SPARSE_CV_GPU:
            x = test_of_cv_gpu_1frm(cfg);
            break;
        case OF_LK_SPARSE_NV_VPI:
            x = test_of_nv_vpi_1frm(cfg);
            break;
        case OF_UTILS:
            x = test_cal_feature_pts_and_save(cfg);
            break;
        }
    }
    else if (1==cfg.of->ofTestSuite )
    {
        switch (cfg.of->alg)
        {
        case OF_LK_SPARSE_CV_CPU:
            x = test_of_cv_cpu_multi_frm(cfg);
            break;
        case OF_LK_SPARSE_CV_GPU:
            x = test_of_cv_gpu_multi_frm(cfg);
            break;
        case OF_LK_SPARSE_NV_VPI:
            x = test_of_nv_vpi_multi_frm(cfg);
            break;
        }
    }
    else{
        switch (cfg.of->alg)
        {
        case OF_LK_SPARSE_CV_CPU:
            x = test_of_cv_cpu_timing(cfg);
            break;
        case OF_LK_SPARSE_CV_GPU:
            x = test_of_cv_gpu_timing(cfg);
            break;
        case OF_LK_SPARSE_NV_VPI:
            x = test_of_nv_vpi_timing(cfg);
            break;
        }
    }

    app::dumpLog("---log end---");
    app::endLogThread();
    return x;
}

void app_usage()
{
    printf("./test.out <cfgFileName>\n");
    printf("examples: \n");
    printf("./test.out ../../src/test/cfg/cfg-opt-flow.xml\n");
}
