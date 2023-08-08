#ifndef _APP_MEAN_STD_H_
#define _APP_MEAN_STD_H_

#include "DataTypes.h"

#define app_smp_t int64_t
//todo: use template
namespace app
{
    class AppMeanStd
    {
    public:
        AppMeanStd(const std::string &unit_ = "")
            : unit(unit_), v()
        {
        }

        void reset()
        {
            v.clear();
        }

        size_t calMeanStd()
        {
            if (v.empty()){
                mu = APP_NAN;
                std = APP_NAN;
                return 0;
            }

            size_t n = v.size();
            double s=0;
            for( app_smp_t e : v){
                s += (double)e;
            }
            mu = s/n;

            s = 0;
            for( app_smp_t e : v){
                s += ((double)e - mu ) * ((double)e - mu );
            }
            std = sqrt(s/n);
            return n;
        }

        void addSample(const app_smp_t x)
        {
            v.push_back(x);
        }

    public:
        std::string unit{""};
        std::vector<app_smp_t> v{};
        double mu{0};
        double std{0};
    };
}

#endif
