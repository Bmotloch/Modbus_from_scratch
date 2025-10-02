#pragma once

#ifndef MEMORY_H_
#define MEMORY_H_

#include <iomanip>

class Memory
{
public:
    Memory()
    {
        data = {
            "2568",
            "5632",
            "1587",
            "0024",
            "00AB",
            "1452",
            "5364",
            "1452",
            "0234",
            "0097",
            "5600",
            "125A",
            "000A",
            "00A0",
            "50FC",
            "0024",
            "7419",
            "52AD",
            "2354",
            "1548",
            "0028",
            "150A",
            "3697",
            "0123",
            "0045",
            "01C6",
            "0070",
            "ABDF",
            "000C",
            "12BC",
            "3FDF",
            "0AAA",
        };
    }

    std::array<std::string, 32> data;
};

#endif