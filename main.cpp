// #include "modbus.hpp"
#include "modbus1.hpp"

// kompilacja g++ main.cpp modbus.cpp -o modbus

int main()
{
  // path name do the device
  const std::string dev{"/dev/ttyUSB0"};
  // Open port/file
  Modbus connection(dev);
  Memory data;

  connection.programCycle();
  return 0;
}