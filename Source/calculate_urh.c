
#include "calculate_urh.h"

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

// Линейная аппроксимация на основе графика из статьи http://sxem.org/2-vse-stati/24-izmereniya/213-radiometr-dozimetr
// https://www.wolframalpha.com/input/?i=linear+fit+%7B200,7200%7D,%7B400,15000%7D
// credit: https://github.com/avdeevsv91/esp_radiation_meter/blob/master/firmware/firmware.c#L78

uint32 calculate_urh_sbm20(float cpm) {
    if (cpm < 200) {
        return round(cpm * (36)); // 36
    } else if (cpm >= 200 && cpm < 400) {
        return round(cpm * ((39 * cpm - 600))); // 37.5
    } else if (cpm >= 400 && cpm < 800) {
        return round(cpm * ((56.25 * cpm - 7500))); // 46.875
    } else if (cpm >= 800 && cpm < 1400) {
        return round(cpm * ((66.6667 * cpm - 15833.3))); // 55.357
    } else if (cpm >= 1400 && cpm < 2000) {
        return round(cpm * ((87.5 * cpm - 45000))); // 65
    } else {
        return 130000; // Предел измерений
    }
}

uint32 calculate_urh_sbm19(float cpm) {

    if (cpm < 200) {
        return round(cpm * (9)); // 9
    } else if (cpm >= 200 && cpm < 400) {
        return round(cpm * ((9.75 * cpm - 150))); // 9.375
    } else if (cpm >= 400 && cpm < 800) {
        return round(cpm * ((14.0625 * cpm - 1875))); // 11.719
    } else if (cpm >= 800 && cpm < 1400) {
        return round(cpm * ((16.6667 * cpm - 3958.33))); // 13.839
    } else if (cpm >= 1400 && cpm < 2000) {
        return round(cpm * ((21.875 * cpm - 11250))); // 16.25
    } else {
        return 130000; // Предел измерений
    }
}