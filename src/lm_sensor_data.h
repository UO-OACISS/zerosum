/*
 * Copyright (c) 2014-2021 Kevin Huck
 * Copyright (c) 2014-2021 University of Oregon
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#pragma once

#include <string>
#include <map>

namespace zerosum {

class sensor_data {
    public:
        sensor_data(void);
        ~sensor_data(void);
        std::string get_version(void);
        std::map<std::string,std::string> read_sensors(void);
};

}

