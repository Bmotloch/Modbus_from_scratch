#include "slave.hpp"

Slave::Slave(std::FILE *serial_port)
{
    m_serial_port = serial_port;
    setSlaveStatus(SlaveStatus::Idle);
}

Slave::~Slave()
{
}

SlaveStatus Slave::getSlaveStatus()
{
    return m_status;
}

void Slave::setSlaveStatus(SlaveStatus status)
{
    m_status = status;
}

bool Slave::ValidateFunction()
{
    if (rec_frame.function == "03")
    {
        return true;
    }
    if (rec_frame.function == "10")
    {
        return true;
    }
    return false;
}

bool Slave::ValidateDataAddress() // do dokonczenia
{
    if (rec_frame.function == "03")
    {
        std::string number_of_registers = rec_frame.data.substr(rec_frame.data.size() - 4, 4);
        std::string starting_address = rec_frame.data.substr(rec_frame.data.size() - 8, 4);
        if (str_hexToint(starting_address) >= str_hexToint(data_start_address))
        {
            if (str_hexToint(starting_address) + str_hexToint(number_of_registers) <= str_hexToint(data_end_address))
                return true;
        }
        else
        {
            return false;
        }
    }
}

bool Slave::ValidateDataValue()
{
    std::string number_of_registers = rec_frame.data.substr(rec_frame.data.size() - 4, 4);

    if (1 <= str_hexToint(number_of_registers) && str_hexToint(number_of_registers) <= 125)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void Slave::slaveCycle()
{
    while (true)
    {
        SlaveStatus status = getSlaveStatus();
        switch (status)
        {
        case SlaveStatus::Idle:
            Idle();
            break;
        case SlaveStatus::CheckingRequest:
            CheckingRequest();
            break;
        case SlaveStatus::ProcessingRequiredAction:
            ProcessingRequiredAction();
            break;
        case SlaveStatus::FormattingNormalReply:
            FormattingNormalReply();
            break;
        case SlaveStatus::FormattingErrorReply:
            FormattingErrorReply();
            break;
        }
    }
}

void Slave::Idle()
{
    char buff[1];
    int fd = fileno(m_serial_port);
    rec_frame.LRC="25";
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
                if (msg.size() > 3)
                {
                    end = msg.substr(msg.size() - 2, 2);
                }
            }
            rec_frame.start = msg.substr(0, 1);
            rec_frame.address = msg.substr(1, 2);
            rec_frame.function = msg.substr(3, 2);
            rec_frame.data = msg.substr(5, msg.size() - 11);
            rec_frame.LRC = msg.substr(msg.size() - 6, 2);
            rec_frame.end = msg.substr(msg.size() - 4, 4);
        }
    }    
    std::string tasiemiec = rec_frame.start + rec_frame.address + rec_frame.function + rec_frame.data + rec_frame.LRC + rec_frame.end;
    std::cout << tasiemiec << std::endl;
    setSlaveStatus(SlaveStatus::CheckingRequest);
}

void Slave::CheckingRequest()
{
    if (rec_frame.address != m_address && rec_frame.address != BROADCAST)
    {
        setSlaveStatus(SlaveStatus::Idle);
        return;
    }
    if (ValidateFunction() == false)
    {
        ExceptionCode = ExceptionCodes::ILLEGAL_FUNCTION;
        setSlaveStatus(SlaveStatus::FormattingErrorReply);
        return;
    }
    if (ValidateDataAddress() == false)
    {
        ExceptionCode = ExceptionCodes::ILLEGAL_DATA_ADDRESS;
        setSlaveStatus(SlaveStatus::FormattingErrorReply);
        return;
    }
    if (ValidateDataValue() == false)
    {
        ExceptionCode = ExceptionCodes::ILLEGAL_DATA_VALUE;
        setSlaveStatus(SlaveStatus::FormattingErrorReply);
        return;
    }
    setSlaveStatus(SlaveStatus::ProcessingRequiredAction);
    return;
}

void Slave::ProcessingRequiredAction() // do dokonczenia
{
    if (rec_frame.function == "03" && rec_frame.address != BROADCAST)
    {
        if (function_03())
        {
            setSlaveStatus(SlaveStatus::FormattingNormalReply);
        }
        else
        {
            setSlaveStatus(SlaveStatus::FormattingErrorReply);
            ExceptionCode = ExceptionCodes::SLAVE_DEVICE_FAILURE;
        }
    }
    else
    {
        setSlaveStatus(SlaveStatus::Idle);
        return;
    }
    if (rec_frame.function == "10")
    {
        if (function_16())
        {
            setSlaveStatus(SlaveStatus::FormattingNormalReply);
        }
        else
        {
            setSlaveStatus(SlaveStatus::FormattingErrorReply);
            ExceptionCode = ExceptionCodes::SLAVE_DEVICE_FAILURE;
        }
    }
}

void Slave::FormattingNormalReply() // do dokonczenia
{
    std::cout<<"Normal"<<std::endl;
    sen_frame.address=rec_frame.address;
    sen_frame.function=rec_frame.function;
    sen_frame.calculateLRC();
    sen_frame.sending(m_serial_port);
    rec_frame.LRC[0]='0';
    rec_frame.LRC[1]='0';//clear of frame
    setSlaveStatus(SlaveStatus::Idle);
}

void Slave::FormattingErrorReply() // do dokonczenia
{
    std::cout<<"Error"<<std::endl;
    sen_frame.address=rec_frame.address;
    sen_frame.function = rec_frame.function;
    sen_frame.function[0] = '8';
    sen_frame.data = ExceptionCode;
    sen_frame.calculateLRC();
    sen_frame.sending(m_serial_port);
    rec_frame.LRC[0]='0';
    rec_frame.LRC[1]='0';//clear of frame
    setSlaveStatus(SlaveStatus::Idle);
}

int Slave::str_hexToint(std::string &hexstring)
{
    int result;
    std::stringstream ss;
    ss << std::hex << hexstring;
    ss >> result;
    return result;
}

std::string Slave::intToHex(int value)
{
    std::stringstream ss;
    ss << std::hex << value;
    std::string hexString = ss.str();
    return hexString;
}

bool Slave::function_03()
{
    std::string start = rec_frame.data.substr(0, 4);
    std::string end = rec_frame.data.substr(rec_frame.data.size() - 4, 4);
    int reg_count = str_hexToint(end) - str_hexToint(start) + 1;
    int bytes = 2 * reg_count;
    sen_frame.data = intToHex(bytes);
    if (sen_frame.data.size() == 1)
    {
        sen_frame.data = "0" + sen_frame.data;
    }
    for (int i = str_hexToint(start); i < str_hexToint(end); i++)
    {
        sen_frame.data += slaveMemory.data[i];
    }
    if ((sen_frame.data.size()-2)/2 == bytes)
    {
        
        return true;
    }
    else
    {
        return false;
    }
}

bool Slave::function_16()
{
    return true;
}