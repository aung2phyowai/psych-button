// Response button firmware for TeensyLC, built with PlatformIO.
//
// Latency is encoded with each data transmission, to be able
// to time-align different data sources. If each connected device
// follows this protocol, a network of devices can operate without
// a common clock, but produce synchronized timing data.
#include <Arduino.h>
#include <Bounce.h>

// MODEL
elapsedMicros loop_timer;
const uint9_t NUM_BUTTONS{4};
uint8_t button_pins[NUM_BUTTONS] = { 2, 3, 4, 5 }; // Teensy digital pins

const uint8_t DEBOUNCE_TIME{100}; // ms
Bounce buttons[NUM_BUTTONS] =
{
    Bounce(button_pins[0],DEBOUNCE_TIME),
    Bounce(button_pins[1],DEBOUNCE_TIME),
    Bounce(button_pins[2],DEBOUNCE_TIME),
    Bounce(button_pins[3],DEBOUNCE_TIME)
};

// FUNCTIONS

// PROGRAM
void setup() {
    for (uint8_t i = 0; i < NUM_BUTTONS; ++i) {
        // initialize buttons
        pinMode(button_pins[i], INPUT_PULLUP);
    }
}

void loop() {
    // from http://alexshroyer.com/Synchronized-Devices/
    // 1. Accept 'begin' signal (and system time) from host PC.
    // 2. Respond by setting uC clock to 0.
    // 3. Emit data with time == 0 + delta_time.
    // 4. PC responds with time it received data.
    // 5. Subsequent data includes round-trip latency.

    // UPDATE
    // Gather input data as fast as possible.
    // input sources include buttons, Serial, etc.

    
    // RENDER
    // Send output data at a controlled rate.
    const uint16_t LOOP_TIME{10000}; // us
    if (timer >= LOOP_TIME) {
        timer -= LOOP_TIME;
        // output data now
    }
}
