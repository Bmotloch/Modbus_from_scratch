#include "modbus.hpp"

bool set_port(termios &tty, int &serial_port)
{
    // Read in existing settings, and handle any error
    if (tcgetattr(serial_port, &tty) != 0)
    {
        std::cerr << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return false;
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
    if (tcsetattr(serial_port, TCSANOW, &tty) != 0)
    {
        std::cerr << "Error " << errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return false;
    }

    usleep(10000);
    tcflush(serial_port, TCIOFLUSH);

    return true;
}

void sending(int &serial_port, Frame &frame)
{
    std::string tasiemiec{sumFrame(frame)};
    int len = tasiemiec.length();
    char *msg = new char[len + 1];
    strcpy(msg, tasiemiec.c_str());
    for (int i = 0; i < len; i++)
    {
        write(serial_port, &msg[i], sizeof(msg[i]));
        std::cout << msg[i];
    }
    std::cout << std::endl;
}

void receiving(int &serial_port) // do zrobienia reading funkcji dostepnych, wysylanie bledow
{
    char buff[1];
    Frame f_rec;
    f_rec.LRC = "25";
    while (!checkLRC(f_rec))
    {
        std::string msg;
        memset(&buff, '\0', sizeof(buff));
        int num_bytes, i{0};
        num_bytes = read(serial_port, &buff, sizeof(buff));
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
                num_bytes = read(serial_port, &buff, sizeof(buff));
                end = msg.substr(size(msg) - 2, 2);
            }
            f_rec.start = msg.substr(0, 1);
            f_rec.address = msg.substr(1, 2);
            f_rec.function = msg.substr(3, 2);
            f_rec.data = msg.substr(5, size(msg) - 11);
            f_rec.LRC = msg.substr(size(msg) - 6, 2);
            f_rec.end = msg.substr(size(msg) - 4, 4);
        }
    }
    std::string tasiemiec = f_rec.start + f_rec.address + f_rec.function + f_rec.data + f_rec.LRC + f_rec.end;
    std::cout << tasiemiec << std::endl;
}

void writeTofile(std::string &data)
{
    std::ofstream fh("/home/bart/Documents/protocols/registry.txt", std::ios::app);
    if (fh.is_open())
    {
        fh << data << std::endl;
        fh.close();
    }
    else
    {
        std::cout << "Error could not find file." << std::endl;
    }
}

void read_from_registers(std::string s_add_hi, std::string s_add_lo, std::string reg_hi, std::string reg_lo, std::string data)
{
    std::string address = s_add_hi + s_add_lo;
    std::string registers = reg_hi + reg_lo;
    int num_reg = std::stoi(registers, nullptr, 16);

    // calculating number of bytes and putting it into data
    int no_bytes = 2 * num_reg;
    std::stringstream sstream;
    sstream << std::hex << no_bytes;
    if (sstream.str().size() == 1)
    {
        data = '0' + sstream.str();
    }
    else
    {
        data = sstream.str();
    }

    std::ifstream fd;
    fd.open("rejestry.txt", std::ifstream::in);
    std::string buff;
    fd >> buff;
    while (buff != address && fd.is_open() != false)
    {
        fd >> buff;
        fd >> buff;
    }
    for (int i{0}; i < num_reg; i++)
    {
        fd >> buff;
        data += buff;
        fd >> buff;
    }
    std::cout << data << std::endl;
}

void calculateLRC(Frame &frame)
{
    std::stringstream LRC_value;
    std::string data_string = frame.address + frame.function + frame.data;
    int sum = 0;

    for (int i = 0; i < data_string.size(); i += 2)
        sum += std::stoul(data_string.substr(i, 2), 0, 16);

    LRC_value << std::hex << std::uppercase << (~sum + 0x01);
    frame.LRC = LRC_value.str().substr(LRC_value.str().size() - 2, 2);
}

bool checkLRC(Frame &frame)
{
    Frame f_rec = frame;

    calculateLRC(f_rec);
    if (frame.LRC == f_rec.LRC)
    {
        return true;
    }
    return false;
}

std::string sumFrame(Frame &frame)
{
    return frame.start + frame.address + frame.function + frame.data + frame.LRC + frame.end;
}

void function03(int &serial_port, Frame &frame)
{
    std::cout << "Type slave adress: ";
    frame.address = correctSlaveAdress();
    frame.function = "03";
    std::cout << "Type adress of registry from where to start to read from: ";
    std::string dataArray[4];
    std::cin >> dataArray[0]; // wykryc bledne dane wpisywane
    dataArray[1] = "32";      // offset w rejestrze
    std::cout << "Type number of registries to read: ";
    std::cin >> dataArray[2]; // wykryc bledne dane wpisywane
    dataArray[3] = "32";      // offset w rejestrze
    frame.data = dataArray[0] + dataArray[1] + dataArray[2] + dataArray[3];
    calculateLRC(frame);
    sending(serial_port, frame);
}

void function16(int &serial_port, Frame &frame)
{
    std::cout << "Type slave adress: ";
    frame.address = correctSlaveAdress();
    frame.function = "F0"; // zrobic merge z funkcja writeToFile
    std::cout << "Type adress of registry from where to start to write to: ";
    std::string dataArray[5];
    std::cin >> dataArray[0]; // wykryc bledne dane wpisywane
    dataArray[1] = "32";      // offset w rejestrze
    std::cout << "Type number of registries to write: ";
    std::cin >> dataArray[2];                                 // wykryc bledne dane wpisywane
    dataArray[3] = "32";                                      // offset w rejestrze
    std::cout << "Type data to write separated by \";\" :\n"; // do zastanowienia sie jak wpisywane beda dane z konsoli
    std::cin >> dataArray[4];                                 // wykryc bledne dane wpisywane, zapis w hex? ewentualnie konwersja na hex, kwestia jeszcze rozmiaru calej ramki do wyslania, trzeba ograniczyc
    frame.data = dataArray[0] + dataArray[1] + dataArray[2] + dataArray[3] + dataArray[4];
    calculateLRC(frame);
    sending(serial_port, frame);
}

void printOptions()
{
    std::cout << "\n"
              << "1: send as Master\n"
              << "2: receive as Slave\n"
              << "3: exit\n";
}

void printMasterOptions()
{
    std::cout << "Available Master options:\n"
              << "1: Function \"03\" read holding registers\n"
              << "2: Function \"16\" write multiple holding registers\n"
              << "3: Back to role options\n";
}

int masterCycle(int &serial_port)
{
    Frame frame;
    char option{'\0'};
    while (option != '1' && option != '2' && option != '3')
    {
        printMasterOptions();
        std::cout << "Insert correct option: ";
        std::cin >> option;
        switch (option)
        {
        case '1':
            function03(serial_port, frame);
            break;
        case '2':
            function16(serial_port, frame);
            break;
        case '3':
            programCycle(serial_port);
            break;
        default:
            std::cout << "Invalid option. Please try again.\n";
            break;
        }
    }
    return 0;
}

int programCycle(int &serial_port)
{
    printOptions();
    char option{'\0'};
    do
    {
        if (std::cin >> option && option != '3')
        {
            switch (option)
            {
            case '1':
                masterCycle(serial_port);
                break;
            case '2':
                receiving(serial_port);
                break;
            }
        }
        else
        {
            std::cout << "Insert correct option: ";
        }

    } while (option != '3');
    return 0;
}

std::string correctSlaveAdress()
{
    bool valid = false;
    int input;
    do
    {
        if (std::cin >> input && input > 1 && input < 247)
        {
            valid = true;
        }
        else
        {
            std::cin.clear();
            std::cin.ignore(100, '\n');
            std::cout << "Slave adress can only be in (1,247) range\n"
                      << "Please type again: ";
        }
    } while (!valid);
    std::string slaveNumber{std::to_string(input)};
    slaveNumber = std::stoul(slaveNumber, 0, 16);
    return slaveNumber;
}
