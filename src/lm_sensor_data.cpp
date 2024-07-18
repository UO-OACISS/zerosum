/*
 * Copyright (c) 2014-2021 Kevin Huck
 * Copyright (c) 2014-2021 University of Oregon
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#include "lm_sensor_data.h"
#include <sensors/sensors.h>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

namespace zerosum {

const char * feature_name[] = {
    "In", //      = 0x00,
    "Fan", //     = 0x01,
    "Temp", //        = 0x02,
    "Power", //       = 0x03,
    "Joules", //      = 0x04,
    "Amps", //        = 0x05,
    "Relative Humidity", //    = 0x06,
    "Max Main", //,
    "",
    "",
    "Vid", //     = 0x10,
    "Intrusion", //   = 0x11,
    "Other", //,
    "",
    "",
    "",
    "",
    "",
    "Beep Enable", // = 0x18,
    ""
};

const char * feature_units[] = {
    "", //      = 0x00,
    "RPM", //     = 0x01,
    "degrees C", //        = 0x02,
    "W", //       = 0x03,
    "J", //      = 0x04,
    "AMP", //        = 0x05,
    "%RH", //    = 0x06,
    "", //,
    "",
    "",
    "", //     = 0x10,
    "", //   = 0x11,
    "", //,
    "",
    "",
    "",
    "",
    "",
    "", // = 0x18,
    ""
};

static const char *sprintf_chip_name(const sensors_chip_name *name)
{
#define BUF_SIZE 200
    static char buf[BUF_SIZE];

    if (sensors_snprintf_chip_name(buf, BUF_SIZE, name) < 0)
        return nullptr;
    return buf;
}

sensor_data::sensor_data()
{
   sensors_init(nullptr);
}

sensor_data::~sensor_data()
{
    sensors_cleanup();
}

string sensor_data::get_version()
{
    ostringstream Converter;
    Converter<<"Version: "<<libsensors_version;
    return Converter.str();
}

std::map<std::string,std::string> sensor_data::read_sensors()
{
    std::map<std::string,std::string> fields;
    sensors_chip_name const * cn;
    int c = 0;
    while ((cn = sensors_get_detected_chips(0, &c)) != 0) {
        sensors_feature const *feat;
        int f = 0;
        while ((feat = sensors_get_features(cn, &f)) != 0) {
            char * label = sensors_get_label(cn, feat);
            sensors_subfeature const *subf;
            int s = 0;
            while ((subf = sensors_get_all_subfeatures(cn, feat, &s)) != 0) {
                if (subf->type == feat->type << 8) {  // we only want inputs
                    std::stringstream ss;
                    ss << feature_name[feat->type] << ": ";
                    ss << sprintf_chip_name(cn) << ", ";
                    ss << label << " (";
                    ss << feature_units[feat->type] << ")";
                    double val;
                    if (subf->flags & SENSORS_MODE_R) {
                        int rc = sensors_get_value(cn, subf->number, &val);
                        if (rc < 0) {
                            // some error
                        } else {
                            //apex::sample_value(ss.str(), val);
                            //printf("%s = %f\n", ss.str().c_str(), val);
                            fields.insert(std::pair<std::string,std::string>(ss.str(),std::to_string(val)));
                        }
                    }
                }
            }
            free(label);
        }
    }
    return fields;
}

}
