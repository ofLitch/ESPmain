#ifndef THRESHOLDS_H
#define THRESHOLDS_H

struct Thresholds {
    float light = 1000.0;
    float temperature = 24.0;
    float humidity = 80.0;
    float ppm = 100.0;
    float soilWet = 500.0;
} thresholds;

extern Thresholds thresholds;

#endif
