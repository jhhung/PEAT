#ifndef IOHANDLER_SETTING_HPP_
#define IOHANDLER_SETTING_HPP_

//#include "../../thread_pool_update.hpp"
//#include "../../ThreadPool/thread_pool.h"
//#include "../../header/thread_pool.h"
#include "../../thread_control_version.hpp"

struct BaseSpaceLocalParameter
{ 
    typedef boost::mpl::int_<0> CurlSendMaxLength;
    typedef boost::mpl::int_<1> CurlSendLastLength;
};

ThreadPool BS_pool (2, 2);

#endif
