#pragma once
#include <stdint.h>
#include "TransmitState.h"
#include "OutputPin.h"
#include "Timer.h"

#define LSB 0

class Transmitter
{
private:
    TransmitState state;
    OutputPin *pin;
    Timer *timer;
    uint8_t transmitBit;

    char data;

public:
    Transmitter(OutputPin *pin, Timer *timer) : pin(pin), timer(timer), transmitBit(LSB)
    {
        this->setState(IDLE);
    }

    ~Transmitter()
    {
        delete this->pin;
    }

    void high();
    void low();
    TransmitState getState();
    void setState(TransmitState);
    void transmit(char);
    void transmit(char *);
    bool isBusy();
    void act(char);
    uint8_t bitValue(char);
    void start();
};
