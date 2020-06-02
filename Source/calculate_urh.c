
#include "calculate_urh.h"

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// Линейная аппроксимация на основе графика из статьи http://sxem.org/2-vse-stati/24-izmereniya/213-radiometr-dozimetr
// https://www.wolframalpha.com/input/?i=linear+fit+%7B200,7200%7D,%7B400,15000%7D
// credit: https://github.com/avdeevsv91/esp_radiation_meter/blob/master/firmware/firmware.c#L78

uint32 calculate_urh_sbm20(float cps) {
    if (cps < 200) {
        return round(cps * (36)); // 36
    } else if (cps >= 200 && cps < 400) {
        return round(cps * ((39 * cps - 600))); // 37.5
    } else if (cps >= 400 && cps < 800) {
        return round(cps * ((56.25 * cps - 7500))); // 46.875
    } else if (cps >= 800 && cps < 1400) {
        return round(cps * ((66.6667 * cps - 15833.3))); // 55.357
    } else if (cps >= 1400 && cps < 2000) {
        return round(cps * ((87.5 * cps - 45000))); // 65
    } else {
        return 130000; // Предел измерений
    }
}

uint32 calculate_urh_sbm19(float cps) {

    if (cps < 200) {
        return round(cps * (9)); // 9
    } else if (cps >= 200 && cps < 400) {
        return round(cps * ((9.75 * cps - 150))); // 9.375
    } else if (cps >= 400 && cps < 800) {
        return round(cps * ((14.0625 * cps - 1875))); // 11.719
    } else if (cps >= 800 && cps < 1400) {
        return round(cps * ((16.6667 * cps - 3958.33))); // 13.839
    } else if (cps >= 1400 && cps < 2000) {
        return round(cps * ((21.875 * cps - 11250))); // 16.25
    } else {
        return 130000; // Предел измерений
    }
}