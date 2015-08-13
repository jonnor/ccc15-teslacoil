#define MAPVALUE(data, inmin, inmax, outmin, outmax) (data-inmin) * (outmax-outmin) / (inmax-inmin) + outmin;

// midiToTesla:
// Convert a MIDI note (number, velocity)
// 
// and check that it is within the safety bounds
bool
midiToTesla(uint8_t noteNumber, uint8_t velocity,
        uint8_t maxDutyPromille, uint8_t maxPulseLengthMicros,
        int *freq_out, int *duty_out)
{
    const float exp = (noteNumber-69)/12.f;
    const int freq = powf(2, exp)*440;

    // Velocity to dutycycle
    const int dutyCycle = MAPVALUE(velocity, 0, 127, 0, maxDutyPromille);
    const float pulseLength = (dutyCycle == 0) ? 0 : ((10e3/freq) * (dutyCycle/1000.));

    //printf("duty=%d, pulselength=%f\n", dutyCycle, pulseLength);

    // Sanity checking
    const bool valid = (dutyCycle <= maxDutyPromille && pulseLength <= maxPulseLengthMicros);
    if (valid) {
        if (freq_out) *freq_out = freq;
        if (duty_out) *duty_out = dutyCycle;
    }
    return valid;
}
