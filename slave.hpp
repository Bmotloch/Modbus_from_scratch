#pragma once

#ifndef sLAVE_H_
#define sLAVE_H_

#include "frame.hpp"
#include "exceptionCodes.hpp"
#include "memory.hpp"
#include <iostream>
#include <sstream>

enum class SlaveStatus
{
    Idle,
    CheckingRequest,
    ProcessingRequiredAction,
    FormattingNormalReply,
    FormattingErrorReply
};

class Slave : public ExceptionCodes
{
private:
    SlaveStatus getSlaveStatus();
    void setSlaveStatus(SlaveStatus status);
    
    std::string m_address{"07"};
    std::string data_start_address{"0000"};
    std::string data_end_address{"007D"};
    int memory_size{32};
    std::FILE *m_serial_port;
    SlaveStatus m_status;
    Frame rec_frame;
    Frame sen_frame;
    Memory slaveMemory;
    std::string ExceptionCode{ExceptionCodes::DEFAULT_STATE};

    void Idle();
    void CheckingRequest();
    void ProcessingRequiredAction();
    void FormattingNormalReply();
    void FormattingErrorReply();
    bool ValidateFunction();
    bool ValidateDataAddress();
    bool ValidateDataValue();
    int str_hexToint(std::string &hexstring);
    std::string intToHex(int value);
    bool function_03();
    bool function_16();

public:
    Slave(std::FILE *serial_port);
    ~Slave();
    void slaveCycle();
};

#endif
