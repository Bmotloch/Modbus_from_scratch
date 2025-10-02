#include "frame.hpp"

Frame::Frame()
{
}

Frame::~Frame()
{
}

std::string Frame::frameToString()
{
    return start + address + function + data + LRC + end;
}


void Frame::calculateLRC()
{
   // std::stringstream LRC_value;
    std::string data_string = address + function + data;
  //  int sum = 0;
    char lrc=0;
   // for (long unsigned int i = 0; i < data_string.size(); i += 2)
     //   sum += std::stoul(data_string.substr(i, 2), 0, 16);

  //  LRC_value << std::hex << std::uppercase << (~sum + 0x01);
  //  LRC = LRC_value.str().substr(LRC_value.str().size() - 2, 2);
    for (char c : data_string) {
        lrc += c;
    }

    lrc = (~lrc) + 1;  // Two's complement

   // return lrc;
   LRC=lrc;
}

bool Frame::checkLRC()
{
    Frame f_rec = *this;

    f_rec.calculateLRC();
    if (this->LRC == f_rec.LRC)
    {
        return true;
    }
    return false;
}

void Frame::sending(std::FILE *serial_port)
{
    std::string tasiemiec{this->frameToString()};
    int len = tasiemiec.length();
    char *msg = new char[len + 1];
    strcpy(msg, tasiemiec.c_str());
    int fd = fileno(serial_port);
    std::cout << "Message sent: "; // debug
    for (int i = 0; i < len; i++)
    {
        write(fd, &msg[i], sizeof(msg[i]));
        std::cout << msg[i];
    }
    std::cout << std::endl;
}