#include "master.hpp"

Master::Master(std::FILE *serial_port)
{
    m_serial_port = serial_port;
    setMasterStatus(MasterStatus::Idle);
}

Master::~Master()
{
}

void Master::printMasterOptions()
{
    std::cout << "Available functions:\n"
              << "Function \"03\" read holding registers\n"
              << "Function \"16\" write multiple holding registers\n";
}

/*
std::string Master::correctSlaveAdress() // do przerobienia pod obsluge bledu przy czytaniu ramki
{
    bool valid = false;
    int input;
    do
    {
        if (input > 1 && input < 247)
        {
            valid = true;
        }
        else
        {
        }
    } while (!valid);
    std::string slaveNumber{std::to_string(input)};
    slaveNumber = std::stoul(slaveNumber, 0, 16);
    return slaveNumber;
}*/

MasterStatus Master::getMasterStatus()
{
    return m_status;
}

void Master::setMasterStatus(MasterStatus status)
{
    m_status = status;
}

void Master::masterCycle()
{
    while (true)
    {
        MasterStatus status = getMasterStatus();
        switch (status)
        {
        case MasterStatus::Idle:
            std::cout << "Idle";
            Idle();
            break;
        case MasterStatus::WaitingForReply:
            std::cout << "WaitingForReply";
            WaitingForReply();
            break;
        case MasterStatus::ProcessingReply:
            std::cout << "ProcessingReply";
            ProcessingReply();
            break;
        case MasterStatus::WaitingTurnaround:
            std::cout << "WaitingTurnaround";
            WaitingTurnaround();
            break;
        case MasterStatus::ProcessingError:
            std::cout << "ProcessingError";
            ProcessingError();
            break;
        }
    }
}

void Master::Idle()
{
    timeout_state = false;
    printMasterOptions();
    std::cout << "Insert address code: ";
    std::cin >> sen_frame.address;
    std::cout << "Insert function code: ";
    std::cin >> sen_frame.function;
    std::cout << "Insert function data: ";
    std::cin >> sen_frame.data;
    sen_frame.calculateLRC();
    sen_frame.sending(m_serial_port);
    if (sen_frame.address == BROADCAST)
    {
        setMasterStatus(MasterStatus::WaitingTurnaround);
    }
    else
    {
        setMasterStatus(MasterStatus::WaitingForReply);
    }
}

void Master::WaitingForReply()
{
    char buff[1];
    int fd = fileno(m_serial_port);
    auto Start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> Elapsed{0.0};
    while (!rec_frame.checkLRC())
    {
        std::string msg;
        memset(&buff, '\0', sizeof(buff));
        int num_bytes, i{0};
        num_bytes = read(fd, &buff, sizeof(buff));

        if (num_bytes < 0)
        {
            printf("Error reading: %s", strerror(errno));
            return;
        }

        if (buff[0] == ':')
        {
            std::string end = "k";
            i++;
            while (num_bytes != 0 && i < 513 && end != "0A")
            {
                msg += buff[0];
                i++;
                num_bytes = read(fd, &buff, sizeof(buff));
                end = msg.substr(msg.size() - 2, 2);
            }
            rec_frame.start = msg.substr(0, 1);
            rec_frame.address = msg.substr(1, 2);
            rec_frame.function = msg.substr(3, 2);
            rec_frame.data = msg.substr(5, msg.size() - 11);
            rec_frame.LRC = msg.substr(msg.size() - 6, 2);
            rec_frame.end = msg.substr(msg.size() - 4, 4);
            if (sen_frame.address != rec_frame.address)
            {
                setMasterStatus(MasterStatus::ProcessingError);
            }
            else
            {
                setMasterStatus(MasterStatus::ProcessingReply);
            }

            auto End = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> Elapsed = End - Start;
            if (Elapsed.count() < 2000.0)
            {
                timeout_state = true;
                setMasterStatus(MasterStatus::ProcessingError);
                break;
            }
        }
    }
    std::string tasiemiec = rec_frame.start + rec_frame.address + rec_frame.function + rec_frame.data + rec_frame.LRC + rec_frame.end;
    std::cout << "Received reply: " << tasiemiec << std::endl;
}

void Master::ProcessingReply()
{
    if (sen_frame.function != rec_frame.function)
    {
        setMasterStatus(MasterStatus::ProcessingError);
    }
    else
    {
        std::cout << "Received data: " << rec_frame.data << std::endl;
    }
}

void Master::ProcessingError()
{
    if (timeout_state == true)
    {
        std::cout << "Timeout" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == ILLEGAL_FUNCTION)
    {
        std::cout << "Exception code: 01 - Illegal function" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == ILLEGAL_DATA_ADDRESS)
    {
        std::cout << "Exception code: 02 - Illegal data address" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == ILLEGAL_DATA_VALUE)
    {
        std::cout << "Exception code: 03 - Illegal data value" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == SLAVE_DEVICE_FAILURE)
    {
        std::cout << "Exception code: 04 - Slave device failure" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == ACKNOWLEDGE)
    {
        std::cout << "Exception code: 05 - Acknowledge" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == SLAVE_DEVICE_BUSY)
    {
        std::cout << "Exception code: 06 - Slave device busy" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == MEMORY_PARITY_ERROR)
    {
        std::cout << "Exception code: 08 - memory parity error" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == GATEWAY_PATH_UNAVAILABLE)
    {
        std::cout << "Exception code: 0A - Gateway path unavailable" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
    if (rec_frame.data == GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND)
    {
        std::cout << "Exception code: 0B - Gateway target device failed to respond" << std::endl;
        setMasterStatus(MasterStatus::Idle);
        return;
    }
}

void Master::WaitingTurnaround()
{
    auto Start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> Elapsed{0.0};
    while (true)
    {
        auto End = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> Elapsed = End - Start;
        if (Elapsed.count() < 200.0)
        {
            setMasterStatus(MasterStatus::Idle);
            break;
        }
    }
}