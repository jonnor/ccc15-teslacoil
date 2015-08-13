#include <math.h>

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
    const int freq = pow(2, exp)*440;

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

class FrequencyAdjustablePWM {
    typedef uint32_t Ticks;
    typedef FrequencyAdjustablePWM Self;
public:
    FrequencyAdjustablePWM()
        : counter(0)
        , frequency(1)
        , dutycyclePromille(0)
        , onPeriod(0)
        , period(0)
    {}

    // Config
    // Returns this, to be chainable
    Self &setTimebase(int d)
    {
        timebaseUs = d;
        recalculate();
//        counter = 0;
        return *this;
    }

    Self &setFrequency(int d)
    {
        frequency = d;
        recalculate();
//        counter = 0;
        return *this;
    }
    Self &setDutycycle(int d)
    {
        dutycyclePromille = d;
        recalculate();
//        counter = 0;
        return *this;
    }

    // Drive PWM forward.
    // Should be done in interrupt for guaranteed timings
    void addTicks(Ticks ticks=1) {
        counter += ticks;
//        printf("addTicks. counter=%d, onperiod=%d, period=%d\n", counter, onPeriod, period);
        if (counter > period) {
            counter = 0;
            changeState(true);
        }
        if (!isOn()) {
            changeState(false);
        }
    }

    // Check current state
    bool isOn() const {
        return (dutycyclePromille && (counter < onPeriod));
    }

private:
    void recalculate() {
//        printf("config. frequency=%d, dutycycle=%d, timebase=%d\n",
//            frequency, dutycyclePromille, timebaseUs);
        period = 1e6/(frequency*timebaseUs);
        onPeriod = (period/1000)*dutycyclePromille;
    }
    void changeState(bool newState) {
        // TODO: fire notification callback?
        // Should keep track of current state to only notify on actual transitions
        // Would have to make sure that the callback function is interrupt-safe...
    }

private:
    // internal state
    Ticks counter;
    // configuration
    int frequency;
    int dutycyclePromille;
    int timebaseUs;
    // cached, derived from config
    Ticks onPeriod;
    Ticks period;
};
