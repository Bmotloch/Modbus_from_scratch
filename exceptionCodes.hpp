#pragma once

#ifndef EXCEPTIONCODES_H_
#define EXCEPTIONCODES_H_

#include <string>

class ExceptionCodes
{
private:
public:
    ExceptionCodes(){};
    ~ExceptionCodes(){};
    const std::string DEFAULT_STATE{"00"};                           // 00
    const std::string ILLEGAL_FUNCTION{"01"};                        // 01
    const std::string ILLEGAL_DATA_ADDRESS{"02"};                    // 02
    const std::string ILLEGAL_DATA_VALUE{"03"};                      // 03
    const std::string SLAVE_DEVICE_FAILURE{"04"};                    // 04
    const std::string ACKNOWLEDGE{"05"};                             // 05
    const std::string SLAVE_DEVICE_BUSY{"06"};                       // 06
    const std::string MEMORY_PARITY_ERROR{"08"};                     // 08
    const std::string GATEWAY_PATH_UNAVAILABLE{"0A"};                // 0A
    const std::string GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND{"0B"}; // 0B
};

#endif