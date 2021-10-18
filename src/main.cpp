#include <Arduino.h>
#include "ReceiveState.h"

#define RECEIVER PD2
#define RECEIVE_PIN_VAL (PIND & (1 << RECEIVER)) >> RECEIVER

uint8_t interval;

// Is the size 1 start bit, 8 bit data frame, no parity bit, 1 stop bit aka 8n1
const uint8_t TOTAL_PACKET_SIZE = 10;
bool bitBuffer[TOTAL_PACKET_SIZE];
uint8_t index;
uint8_t characterValue;

volatile ReceiveState receiveState = IDLE;

void convertToBaseTen();
void printDetails();

void setup()
{
    Serial.begin(9600);
    // RECEIVER input pin with internal pullup.
    DDRD &= ~(1 << RECEIVER);
    PORTD |= (1 << RECEIVER);

    sei();
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT18);
}

ISR(PCINT2_vect)
{
    // Turn off pin change interrupt for input pin.
    PCMSK2 &= ~(1 << PCINT18);

    // Timer 1 setup:
    TCCR1A = 0;
    // CTC mode for OCR1A and pre-scaler 1
    TCCR1B = 0 | (1 << WGM12) | (1 << CS10);
    // Set compare value for interrupts
    // 0.05199 / (1 / 16000000 * 1000) rounded 832
    OCR1A = 832;
    // Reset timer1 counter value
    TCNT1 = 0;
    // Enable timer compare interrupt
    TIMSK1 = 0 | (1 << OCIE1A);

    // Reset interval and index value since new receival
    interval = 0;
    index = 0;

    // Update receiver state
    receiveState = RECEIVING;
}

ISR(TIMER1_COMPA_vect)
{
    ++interval;

    if (interval >= 20)
    {
        receiveState = COMPLETED;
    }
    else if (interval % 2 == 1)
    {
        bitBuffer[index] = RECEIVE_PIN_VAL;
        index++;
    }
}

void loop()
{
    if (receiveState == COMPLETED)
    {
        // Turn off timer interrupts
        TIMSK1 &= ~(1 << OCIE1A);
        receiveState = IDLE;
        convertToBaseTen();
        printDetails();
        // Turn on pin change interrupts for receive pin again
        PCMSK2 |= (1 << PCINT18);
    }
}

void convertToBaseTen()
{
    uint8_t base = 1;
    characterValue = 0;

    for (uint8_t i = 1; i < TOTAL_PACKET_SIZE - 1; i++)
    {
        characterValue += base * bitBuffer[i];
        base *= 2;
    }
}

void printDetails()
{
    Serial.print("BYTE: ");
    for (uint8_t i = TOTAL_PACKET_SIZE - 2; i > 0; i--)
    {
        Serial.print(bitBuffer[i]);
    }
    Serial.println();

    Serial.print("INTEGER: ");
    Serial.println(characterValue);

    Serial.print("CHARACTER: ");
    Serial.println((char)characterValue);

    Serial.println();
}
