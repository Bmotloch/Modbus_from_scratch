#pragma once

#ifndef MODBUS_H_
#define MODBUS_H_
// C library headers
#include <cstdio>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <time.h>
#include <chrono>

// C++ headers
#include <iostream>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <fstream>

#include "master.hpp"
#include "slave.hpp"

#define START ':'
#define BROADCAST "00"
#define ADDRESS "0B"
#define READREGISTRIES "03"
#define WRITEREGISTRIES "16"
#define END "0D0A"

class Modbus
{
public:
    Modbus(const std::string &dev);
    ~Modbus();
    int programCycle(); // dopisac case dla slavoja zizeka
    std::FILE *getFileDescriptor();

private:
    void printOptions();
    int set_port(termios &tty);
    std::FILE *serial_port;
};

#endif