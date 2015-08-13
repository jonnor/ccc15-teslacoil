

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
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
    // description
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

int
main(int argc, char *argv[])
{
    int failures = 0;
    const int8_t n_cases = sizeof(testcases)/sizeof(testcases[0]);
    for (int i=0; i<n_cases; i++) {
        TestCase tc = testcases[i];
        char *fail = checkMidiToTesla(tc);
        if (fail) {
            fprintf(stderr, "✗ %s\n\t %s\n", tc.description, fail);
            failures += 1;
        } else {
            fprintf(stdout, "✓ %s\n", tc.description);
        }
    }
    printf("PASSED: %d, FAILED: %d\n", n_cases-failures, failures);
    return (failures > 0) ? 1 : 0;
}
