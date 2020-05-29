#define TC_LINKKEY_JOIN
#define NV_INIT
#define NV_RESTORE
#define INT_HEAP_LEN 2688

#define TP2_LEGACY_ZC
//patch sdk
// #define ZDSECMGR_TC_ATTEMPT_DEFAULT_KEY TRUE

#define NWK_AUTO_POLL
#define MULTICAST_ENABLED FALSE

#define ZCL_READ
#define ZCL_WRITE

#define ZCL_BASIC
#define ZCL_IDENTIFY
#define ZCL_REPORTING_DEVICE

#define DISABLE_GREENPOWER_BASIC_PROXY
#define BDB_FINDING_BINDING_CAPABILITY_ENABLED 1
#define BDB_REPORTING TRUE

#ifdef DEFAULT_CHANLIST
#undef DEFAULT_CHANLIST
#endif
#define DEFAULT_CHANLIST 0x07FFF800

#define ISR_KEYINTERRUPT
#define HAL_BUZZER FALSE
#define HAL_KEY FALSE

#define HAL_LED TRUE
#define HAL_ADC FALSE

#define BLINK_LEDS TRUE

#define INTER_PAN
#define BDB_TL_TARGET


//one of this boards
// #define HAL_BOARD_TARGET
// #define HAL_BOARD_CHDTECH_DEV

#if !defined(HAL_BOARD_TARGET) && !defined(HAL_BOARD_CHDTECH_DEV)
#error "Board type must be defined"
#endif





#if defined(HAL_BOARD_TARGET)
    #define HAL_UART FALSE
    #define HAL_PA_LNA
    #define HAL_PA_LNA_CC2592
    #define APP_TX_POWER TX_PWR_PLUS_19

#elif defined(HAL_BOARD_CHDTECH_DEV)
    #define HAL_UART TRUE
    #define HAL_UART_ISR 2
    #define HAL_UART_DMA 1
    #define APP_TX_POWER TX_PWR_PLUS_4
#endif


#include "hal_board_cfg.h"
