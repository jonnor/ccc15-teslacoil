
#include <MIDI.h>
#include "tesla.hpp"

static const int8_t TESLA_PIN = 9;
static const int8_t SOFTERROR_PIN = 13;
static const uint8_t MAX_DUTYCYCLE_PROMILLE = 10;
static const uint8_t MAX_PULSE_MICROS = 200;
static const int SERIAL_RATE = 9600;
//midi::MidiInterface<HardwareSerial> midiInterface(Serial);
MIDI_CREATE_DEFAULT_INSTANCE(); // industry-standard crack (tm)
FrequencyAdjustablePWM pwm;

// Drive timer/PWM
ISR(TIMER2_COMPA_vect) {
	pwm.addTicks(256/64);
    digitalWrite(TESLA_PIN, pwm.isOn()); // maybe use registers directly?
}

static void
setupTimer() {
    const uint8_t timebaseuS = 4;
    const int16_t prescale = 256;
    byte mode = 0x01;
    switch (prescale) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
    }

	TIMSK2 &= ~(1<<OCIE2A); // disable timer iterrupt
	ASSR &= ~(1<<AS2);      // Set clock, not pin.
	TCCR2A = (1<<WGM21);    // Set Timer2 to CTC mode.
    TCCR2B = TCCR2B & 0b11111000 | mode;
//	TCCR2B = (1<<CS22);     // Set Timer2 prescaler to 64 (4uS/tick, 4uS-1020uS range).
	TCNT2 = 0;             // Reset Timer2 counter.
//	OCR2A = min((timebaseuS>>2) - 1, 255); // Every count is 4uS, so divide by 4 (bitwise shift right 2) subtract one, then make sure we don't go over 255 limit.
	TIMSK2 |= (1<<OCIE2A);                // Enable Timer2 interrupt.
    pwm.setTimebase(timebaseuS);
    
}

// FIXME: actually respect frequency changes
static void
updateTelsa(int freq, int dutyPromille)
{  
    pwm.setDutycycle(dutyPromille).setFrequency(freq);
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

//    if (!valid) { duty = 0; }
//    freq = 100;
//    duty = 10;
    updateTelsa(freq, duty);

    Serial.println("set freq,duty");
    Serial.println(freq); Serial.println(duty);
}

void
setup()
{
//    setPwmFrequency(TESLA_PIN, 256); // 122 Hz
    Serial.begin(SERIAL_RATE);
    while (!Serial) {
      // wait for computer connection
    }
    MIDI.begin(1);
    Serial.begin(115200);
    MIDI.setHandleNoteOff(changeNote);
    MIDI.setHandleNoteOn(changeNote);
    MIDI.turnThruOff(); // so we can use Serial for debug

    pinMode(TESLA_PIN, OUTPUT);
    setupTimer();
    updateTelsa(139, 0); // default off

    Serial.println("Setup complete");
}

void
loop()
{
    // everything else is callback-driven
    //Serial.write("main iter");
    MIDI.read();
}
