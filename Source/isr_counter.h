#ifndef ISR_COUNTER_H
#define ISR_COUNTER_H

typedef void (*isr_counter_CB_t)( uint8 portNum );


extern uint16 zclApp_Port0CounterValue;
extern uint16 zclApp_Port1CounterValue;
extern uint16 zclApp_Port2CounterValue;
extern void zclApp_InitCounter(void);

extern void zclApp_RegisterCounterCallback(isr_counter_CB_t callback);


#endif