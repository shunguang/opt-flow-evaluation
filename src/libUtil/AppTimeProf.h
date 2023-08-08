#ifndef _APP_TIME_PROF_H_
#define _APP_TIME_PROF_H_

#include "DataTypes.h"
#include "AppMeanStd.h"
namespace app
{
    class AppTimeProf
    {
    public:
        AppTimeProf(const size_t thdCntTpPrintf=100 ) 
        : thdCntToPrintOut(thdCntTpPrintf)
        , wall("us"), ucpu("us"), scpu("us") 
        {
            reset();
        }

        void setThdCntToPrintOut( const size_t nFrms ){
            thdCntToPrintOut = nFrms;
        }

        void reset()
        {
            cnt=0;
            wall.reset();
            ucpu.reset();
            scpu.reset();
        }   

        size_t calMeanStd(std::string & out, const std::string &msg = "")
        {
            size_t nSmps = 0;
            nSmps = wall.calMeanStd();
            nSmps = ucpu.calMeanStd();
            nSmps = scpu.calMeanStd();

            char buf[1024];
            snprintf(buf, 1024, "%s frmCnt=%ld, wall(mu=%.0f, std=%.0f) (us), usrCpu(mu=%.0f, std=%.0f) (us), sysCpu(mu=%.0f, std=%.0f) (us)",
                                    msg.c_str(), nSmps, wall.mu, wall.std, ucpu.mu, ucpu.std, scpu.mu, scpu.std);

            out = std::string(buf);

            return nSmps;
        }

        void addSamples(const app_smp_t dt_wall, const app_smp_t dt_ucpu, const app_smp_t dt_scpu)
        {
            cnt++;
            wall.addSample(dt_wall);
            ucpu.addSample(dt_ucpu);
            scpu.addSample(dt_scpu);
            if( 0 == cnt % thdCntToPrintOut)
            {
                std::string result;
                calMeanStd(result, "perFrameStatistics");
                printf("current: dt_wall=%ld,dt_ucpu=%ld,dt_scpu=%ld, %s\n", dt_wall, dt_ucpu, dt_scpu, result.c_str());
            }

        }

    private:
        size_t thdCntToPrintOut{100};
        size_t cnt{0};
        AppMeanStd wall;
        AppMeanStd ucpu;
        AppMeanStd scpu;
    };
}

#endif
