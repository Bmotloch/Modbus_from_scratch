#pragma once

#ifndef FRAME_H_
#define FRAME_H_

#define START ':'
#define BROADCAST "00"
#define ADDRESS "0B"
#define READREGISTRIES "03"
#define WRITEREGISTRIES "16"
#define END "0D0A"

#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <iostream>

class Frame
{
private:
public:
    Frame();
    ~Frame();
    std::string start{START};
    std::string address{BROADCAST};
    std::string function{READREGISTRIES};
    std::string data{""};
    std::string LRC{"25"};
    std::string end{END};
    std::string frameToString();
    void calculateLRC();
    bool checkLRC();
    void sending(std::FILE *serial_port);
};

#endif