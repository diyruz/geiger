#include "isr_counter.h"
#include "Debug.h"

#define ISR_COUNTER_P0_PINS (BV(1) | BV(4))
#define ISR_COUNTER_P1_PINS 0x00
#define ISR_COUNTER_P2_PINS 0x00

#define ISR_COUNTER_RISING_EDGE 0
#define ISR_COUNTER_FALLING_EDGE 1

#define ISR_COUNTER_P0_EDGE ISR_COUNTER_FALLING_EDGE
#define ISR_COUNTER_P1_EDGE ISR_COUNTER_FALLING_EDGE
#define ISR_COUNTER_P2_EDGE ISR_COUNTER_FALLING_EDGE

#define UINT32_MAX 4294967295U
isr_counter_CB_t zclApp_Port0CounterCB = NULL;
isr_counter_CB_t zclApp_Port1CounterCB = NULL;
isr_counter_CB_t zclApp_Port2CounterCB = NULL;

void zclApp_RegisterCounterCallback(uint8 portNum, isr_counter_CB_t callback) {
    if (portNum == 0) {
        zclApp_Port0CounterCB = callback;
    } else if (portNum == 1) {
        zclApp_Port1CounterCB = callback;
    } else if (portNum == 2) {
        zclApp_Port2CounterCB = callback;
    }
}

#if ISR_COUNTER_P0_PINS != 0x00
uint16 zclApp_Port0CounterValue = 0;

#endif
#if ISR_COUNTER_P1_PINS != 0x00
uint16 zclApp_Port1CounterValue = 0;

#endif
#if ISR_COUNTER_P2_PINS != 0x00
uint16 zclApp_Port2CounterValue = 0;

#endif
void zclApp_InitCounter(void) {
#if ISR_COUNTER_P0_PINS != 0x00
    P0SEL &= ~ISR_COUNTER_P0_PINS;
    P0DIR &= ~(ISR_COUNTER_P0_PINS);
#if ISR_COUNTER_P0_EDGE == ISR_COUNTER_RISING_EDGE
    PICTL &= ~BV(0); // Rising edge on input gives interrupt.
#else
    PICTL |= BV(0); // Falling edge on input gives interrupt
#endif
    P0IEN |= ISR_COUNTER_P0_PINS;

    IEN1 |= BV(5);
#endif

#if ISR_COUNTER_P1_PINS != 0x00
    P1SEL &= ~ISR_COUNTER_P1_PINS;
    P1DIR &= ~(ISR_COUNTER_P1_PINS);
#if ISR_COUNTER_P1_EDGE == ISR_COUNTER_RISING_EDGE
    PICTL &= ~BV(1); // Port 1, inputs 3 to 0 interrupt configuration.
    PICTL &= ~BV(2); // Port 1, inputs 7 to 4 interrupt configuration.
#else
    PICTL |= BV(1); // Port 1, inputs 3 to 0 interrupt configuration.
    PICTL |= BV(2); // Port 1, inputs 7 to 4 interrupt configuration.
#endif
    P1IEN |= ISR_COUNTER_P1_PINS;
    IEN2 |= BV(4);
#endif

#if ISR_COUNTER_P2_PINS != 0x00
    P2SEL &= ~ISR_COUNTER_P2_PINS;
    P2DIR &= ~(ISR_COUNTER_P2_PINS);
#if ISR_COUNTER_P2_EDGE == ISR_COUNTER_RISING_EDGE
    PICTL &= ~BV(3); // Port 2, inputs 4 to 0 interrupt configuration.
#else
    PICTL |= BV(3); // Port 2, inputs 4 to 0 interrupt configuration.
#endif
    P2IEN |= ISR_COUNTER_P2_PINS;
    IEN2 |= BV(1);
#endif
}

#if ISR_COUNTER_P0_PINS != 0x00
HAL_ISR_FUNCTION(halKeyPort0Isr, P0INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P0IFG & ISR_COUNTER_P0_PINS) {
        // if (zclApp_Port0CounterValue >= UINT32_MAX) { //should be dead already
        //     zclApp_Port0CounterValue = 0;
        // }
        zclApp_Port0CounterValue += 1;
        P0IFG &= ~ISR_COUNTER_P0_PINS;
        if (zclApp_Port0CounterCB != NULL) {
            (*zclApp_Port0CounterCB)();
        }
    }
    P0IF = 0;

    HAL_EXIT_ISR();
}
#endif

#if ISR_COUNTER_P1_PINS != 0x00
HAL_ISR_FUNCTION(halKeyPort1Isr, P1INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P1IFG & ISR_COUNTER_P1_PINS) {
        zclApp_Port1CounterValue += 1;
        P1IFG &= ~ISR_COUNTER_P1_PINS;
        if (zclApp_Port1CounterCB != NULL) {
            (*zclApp_Port1CounterCB)();
        }
    }

    P1IF = 0;

    HAL_EXIT_ISR();
}
#endif

#if ISR_COUNTER_P2_PINS != 0x00
HAL_ISR_FUNCTION(halKeyPort2Isr, P2INT_VECTOR) {
    HAL_ENTER_ISR();

    if (P2IFG & ISR_COUNTER_P2_PINS) {
        zclApp_Port2CounterValue += 1;
        P2IFG &= ~ISR_COUNTER_P2_PINS;
        if (zclApp_Port2CounterCB != NULL) {
            (*zclApp_Port2CounterCB)();
        }
    }

    P2IF = 0;

    HAL_EXIT_ISR();
}
#endif