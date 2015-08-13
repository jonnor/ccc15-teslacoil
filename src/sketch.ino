/**
 * Divides a given PWM pin frequency by a divisor.
 *
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 *
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 *
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 *
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    //TCCR2B = TCCR2B & 0b11111000 | mode; // not defined on Leonardo
  }
}

#include <MIDI.h>
#include "tesla.hpp"

static const int8_t TESLA_PIN = 9;
static const int8_t SOFTERROR_PIN = 13;
static const uint8_t MAX_DUTYCYCLE_PROMILLE = 10;
static const uint8_t MAX_PULSE_MICROS = 200;
static const int SERIAL_RATE = 9600;
//midi::MidiInterface<HardwareSerial> midiInterface(Serial);
MIDI_CREATE_DEFAULT_INSTANCE(); // industry-standard crack (tm)

// FIXME: actually respect frequency changes
static void
updateTelsa(int8_t freq, int8_t dutyPromille)
{
    analogWrite(TESLA_PIN, ((int)dutyPromille*255)/1000);
}

// MIDI callback
static void 
changeNote(byte channel, byte note, byte velocity)
{
    int freq = -1;
    int duty = -1;
    const bool valid = midiToTesla(note, velocity,
        MAX_DUTYCYCLE_PROMILLE, MAX_PULSE_MICROS,
        &freq, &duty);
    digitalWrite(SOFTERROR_PIN, !valid);
    if (!valid) { duty = 0; }
    updateTelsa(freq, duty); // TEMP
    Serial.println("set freq,duty");
    Serial.println(freq); Serial.println(duty);
}

void
setup()
{
    setPwmFrequency(TESLA_PIN, 256); // 122 Hz
    Serial.begin(SERIAL_RATE);
    while (!Serial) {
      // wait for computer connection
    }
    MIDI.begin(1);
    Serial.begin(115200);
    MIDI.setHandleNoteOff(changeNote);
    MIDI.setHandleNoteOn(changeNote);
    MIDI.turnThruOff(); // so we can use Serial for debug
    updateTelsa(100, 0); // default off
    Serial.println("Setup complete");
}

void
loop()
{
    // everything else is callback-driven
    //Serial.write("main iter");
    //delay(100);
    MIDI.read();
}
