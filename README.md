# Modbus_from_scratch
C++11 Modbus Serial Communication (RS-485)

## Overview

This is a C++ demo of a Modbus Master/Slave system for exchanging data between two or more devices.
It uses a 2-wire RS-485 half-duplex serial connection (tested using USB-to-RS485 adapters).

## Design Overview

The project uses a State Design Pattern (Finite State Machine) to handle communication flow.
This approach helps manage timing issues and asynchronous events common in serial communication.

## Key Features

Serial Timing Control – Handles transmit/receive switching delays for RS-485.

Protocol Flow – Manages sending requests, waiting for responses, and checking replies (with LRC validation).

Modular Design – Separates communication logic from application code for easier maintenance and scalability.

## Skills & Technologies

C++: Object-oriented design, State Design Pattern (FSM).

Industrial Protocols: Modbus RTU (Master/Slave behavior, function codes, framing).

Hardware: Linux serial port programming (termios) for 2-wire RS-485 control.
