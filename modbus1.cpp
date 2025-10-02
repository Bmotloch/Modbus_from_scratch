#include "modbus1.hpp"

Modbus::Modbus(const std::string &dev) : serial_port(nullptr)
{
    serial_port = std::fopen(dev.c_str(), "r+");
    if (serial_port == nullptr)
    {
        std::cerr << "Error opening device: " << strerror(errno) << std::endl;
        return;
    }

    struct termios tty;
    set_port(tty);
}

Modbus::~Modbus()
{
    if (serial_port != nullptr)
    {
        std::fclose(serial_port);
    }
}

int Modbus::set_port(termios &tty)
{
    int fd = fileno(serial_port);

    if (tcgetattr(fd, &tty) != 0)
    {
        std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
    }

    tty.c_cflag &= ~PARENB;        // Disabling parity
    tty.c_cflag |= CSTOPB;         // Setting two stop bits
    tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
    tty.c_cflag |= CS8;            // Setting 8 bits per byte
    tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)
    tty.c_lflag &= ~ICANON;
    tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT, and SUSP
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes
    tty.c_oflag &= ~OPOST;                                                       // Prevent special interpretation of output bytes (e.g., newline chars)
    tty.c_oflag &= ~ONLCR;                                                       // Prevent conversion of newline to carriage return/line feed
    tty.c_cc[VTIME] = 10;                                                        // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    tty.c_cc[VMIN] = 0;

    // Set in/out baud rate to be 9600
    cfsetispeed(&tty, B9600);
    cfsetospeed(&tty, B9600);

    // Save tty settings, also checking for error
    if (tcsetattr(fd, TCSANOW, &tty) != 0)
    {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
    }

    usleep(10000);
    tcflush(fd, TCIOFLUSH);
    return EXIT_SUCCESS;
}

int Modbus::programCycle()
{
    while (true)
    {
        printOptions();
        std::cout << "Insert option: ";
        char option;
        if (!(std::cin >> option))
        {
            std::cerr << "Error reading input." << std::endl;
            return EXIT_FAILURE;
        }
        switch (option)
        {
        case '1':
        {
            Master master(getFileDescriptor());
            master.masterCycle();
            break;
        }
        case '2':
        {
            Slave slave(getFileDescriptor());
            slave.slaveCycle();
            break;
        }
        case '3':
            std::cout << "Exiting program." << std::endl;
            return EXIT_SUCCESS;
        default:
            std::cout << "Invalid option. Please try again." << std::endl;
            break;
        }
    }
}

void Modbus::printOptions()
{
    std::cout << "\n"
              << "1: send as Master\n"
              << "2: receive as Slave\n"
              << "3: exit\n";
}

std::FILE *Modbus::getFileDescriptor()
{
    return serial_port;
}