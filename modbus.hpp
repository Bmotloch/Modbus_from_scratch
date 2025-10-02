#pragma once
// C library headers
#include <stdio.h>
#include <string.h>

// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <time.h>
#include <chrono>

// C++ headers
#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <sstream>
#include <fstream>
#include <cstring>

#define START ':'
#define BROADCAST "00"
#define ADDRESS "0B"
#define READREGISTRIES "03"
#define WRITEREGISTRIES "16"
#define END "0D0A"

// default lrc "FD"

enum class master_status
{
    idle,
    waiting_for_reply,
    processing_reply,
    processing_error,
    waiting_turnaround
};

enum class slave_statue
{
    idle,
    checking_request,
    processing_req_action,
    formatting_normal_reply,
    formatting_error_reply
};

struct Frame
{
    std::string start{START};
    std::string address{BROADCAST};
    std::string function{READREGISTRIES};
    std::string data;
    std::string LRC;
    std::string end{END};
};

struct Memory
{
    std::string data[32] = {
        "2568",
        "56320",
        "1587",
        "0024",
        "0015AB",
        "1452",
        "5364",
        "1452",
        "0105234",
        "0009097",
        "524600",
        "125A",
        "000A",
        "00A0",
        "5260FC",
        "0024",
        "7419",
        "0052AD",
        "2354",
        "1548",
        "0028",
        "14750A",
        "3697",
        "0123",
        "0045",
        "0001C6",
        "0070",
        "ABDF",
        "000C",
        "12BC",
        "45D3FDF",
        "3AB00AAA"};
};

std::string sumFrame(Frame &frame);
void calculateLRC(Frame &frame);
bool set_port(termios &tty, int &serial_port);
void sending(int &serial_port, Frame &frame);
void receiving(int &serial_port);
void writeTofile(std::string &data);
void read_from_registers(std::string s_add_hi, std::string s_add_lo, std::string reg_hi, std::string reg_lo, std::string data);
bool checkLRC(Frame &frame);
int programCycle(int &serial_port);
int masterCycle(int &serial_port);
void printOptions();
void printMasterOptions();
void function03(int &serial_port, Frame &frame);
void function16(int &serial_port, Frame &frame);
std::string correctSlaveAdress();
