#pragma once

#ifndef MASTER_H_
#define MASTER_H_

#include "frame.hpp"
#include "exceptionCodes.hpp"
#include "memory.hpp"
#include <iostream>
#include <chrono>

enum class MasterStatus
{
    Idle,
    WaitingForReply,
    ProcessingReply,
    ProcessingError,
    WaitingTurnaround
};

class Master : public ExceptionCodes 
{
private:
    MasterStatus getMasterStatus();
    void printMasterOptions();
    void setMasterStatus(MasterStatus status);
    std::string address{"01"};
    std::FILE *m_serial_port;
    MasterStatus m_status;
    bool timeout_state{false};
    Frame rec_frame;
    Frame sen_frame;
    void Idle();
    void WaitingTurnaround();
    void WaitingForReply();
    void ProcessingReply();
    void ProcessingError();

public:
    Master(std::FILE *serial_port);
    ~Master();
    void masterCycle();
    std::string correctSlaveAdress();
};
#endif
