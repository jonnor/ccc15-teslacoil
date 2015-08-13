

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "tesla.hpp"


#define CHECK_RETURN_IF_FAIL(topic, actual, expect) \
do { \
    if (actual != expect) { \
        const size_t MAX_STR = 100; \
        char *str = (char *)(malloc(MAX_STR+1)); \
        snprintf(str, MAX_STR, "FAIL %s: %d != %d", topic, actual, expect); \
        return str; \
    } \
} while(0)

// 

struct TestCase {
    const char *description;
    // in
    uint8_t note;
    uint8_t velocity;
    uint8_t dutymax;
    uint8_t pulsemax;
    // expect
    bool valid;
    int freq;
    int duty;
};

// Returns description string if failing, else NULL
char *
checkMidiToTesla(TestCase c) {
    int freq = -1;
    int duty = -1;
    const bool valid = midiToTesla(c.note, c.velocity, c.dutymax, c.pulsemax, &freq, &duty);
    CHECK_RETURN_IF_FAIL("valid", valid, c.valid);
    if (c.duty > -1) { CHECK_RETURN_IF_FAIL("duty", duty, c.duty); }
    if (c.freq > -1) { CHECK_RETURN_IF_FAIL("frequency", freq, c.freq); }
    return NULL;
}

// Run
const int TYP_PULSEMAX = 200; // microsec
const int TYP_DUTYMAX = 20; // promille

struct TestCase testcases[] =
{
    { "note with velocity 0, should be valid with dutycycle 0",
        69, 0, TYP_DUTYMAX, TYP_PULSEMAX, true, 440, 0 },
    { "note with velocity 127, should be valid with max dutycycle",
        57, 127, TYP_DUTYMAX, TYP_PULSEMAX, true, 220, TYP_DUTYMAX },
    { "note with medium velocity, should be valid with medium dutycycle",
        40, 100, TYP_DUTYMAX, TYP_PULSEMAX, true, 82, 15 },

//    { "low note with high velocity should not be valid",
//        21, 127, TYP_DUTYMAX, TYP_PULSEMAX, false, -1, -1 },
};

struct PwmTestCase {
    struct Iteration {
        uint addTicks;
        bool expectedState;
    };

    const char *description;
    // initialization
    int frequency;
    int dutycycle;
    int timebase;
    // to check
    Iteration iterations[3];
};

struct PwmTestCase pwm_testcases[] = {
    { "initializing with 0% dutycyle, should be false forever",
        (int)(1e6/10000), 0, 4, { { 0, false }, { 1000, false }, { 3000, false } }
    },
    { "100Hz, initializing with 50% dutycyle, should be true halfway, then false",
        (int)(1e6/10000), 500, 4, { { 0, true }, { 12000, true }, { 500, false } }
    },
    { "initializing with 100% dutycyle, should be true forever",
        (int)(1e6/10000), 1000, 4, { { 0, true }, { 1000, true }, { 2000, true } }
    },
    { "100Hz, initializing with 1% dutycyle, should be true 1%, then false till 100%, then true",
        (int)(1e6/10000), 10, 4, { { 240, true }, { 24000, false }, { 1500, true } }
    },
};

#define CHECK_ITER(iter, description) \
do { \
    pwm.addTicks(iter.addTicks); \
    CHECK_RETURN_IF_FAIL(description, pwm.isOn(), iter.expectedState); \
} while(0)

// Returns description string if failing, else NULL
char *
checkPwm(PwmTestCase c)
{
    FrequencyAdjustablePWM pwm;
    pwm.setFrequency(c.frequency).setDutycycle(c.dutycycle).setTimebase(c.timebase);
    CHECK_ITER(c.iterations[0], "iteration 0");
    CHECK_ITER(c.iterations[1], "iteration 1");
    CHECK_ITER(c.iterations[2], "iteration 2");
    return NULL;
}

bool
checkFail(const char *fail, const char *description)
{
    if (fail) {
        fprintf(stderr, "✗ %s\n\t %s\n", description, fail);
        return true;
    } else {
        fprintf(stdout, "✓ %s\n", description);
        return false;
    }
}

#define ARRAY_STATIC_LENGTH(array) sizeof(array)/sizeof(array[0])

int
main(int argc, char *argv[])
{
    int failures = 0;

    // MIDI
    printf("MidiToTesla()\n");
    const int8_t n_midi_cases = ARRAY_STATIC_LENGTH(testcases);
    for (int i=0; i<n_midi_cases; i++) {
        TestCase tc = testcases[i];
        if (checkFail(checkMidiToTesla(tc), tc.description)) {
          failures += 1;
        }
    }

    // PWM
    printf("class FrequencyAdjustablePWM\n");
    const int8_t n_pwm_cases = ARRAY_STATIC_LENGTH(pwm_testcases);
    for (int i=0; i<n_pwm_cases; i++) {
        PwmTestCase tc = pwm_testcases[i];
        if (checkFail(checkPwm(tc), tc.description)) {
          failures += 1;
        }
    }

    // Report
    const int n_cases = n_midi_cases + n_pwm_cases;
    printf("PASSED: %d, FAILED: %d\n", n_cases-failures, failures);
    return (failures > 0) ? 1 : 0;
}
