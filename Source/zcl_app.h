#ifndef ZCL_APP_H
#define ZCL_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * INCLUDES
 */
#include "version.h"
#include "zcl.h"

/*********************************************************************
 * CONSTANTS
 */
#define APP_REPORT_DELAY ((uint32)60 * (uint32)1000) // 1 minute
// Application Events
#define APP_REPORT_EVT 0x0001
#define APP_SAVE_ATTRS_EVT 0x0002
#define APP_BLINK_EVT 0x0004

/*********************************************************************
 * MACROS
 */

#define NW_APP_CONFIG 0x0401

#define R ACCESS_CONTROL_READ
//ACCESS_CONTROL_AUTH_WRITE
#define RW (R | ACCESS_CONTROL_WRITE | ACCESS_CONTROL_AUTH_WRITE)
#define RR (R | ACCESS_REPORTABLE)

#define BASIC ZCL_CLUSTER_ID_GEN_BASIC
#define GEN_ON_OFF ZCL_CLUSTER_ID_GEN_ON_OFF
#define POWER_CFG ZCL_CLUSTER_ID_GEN_ON
#define ILLUMINANCE ZCL_CLUSTER_ID_MS_ILLUMINANCE_MEASUREMENT
#define ILLUMINANCE_CONFIG ZCL_CLUSTER_ID_MS_ILLUMINANCE_LEVEL_SENSING_CONFIG


#define ATTRID_RADIATION_SENSOR_SENSIVITY 0xF000
#define ATTRID_RADIATION_LED_FEEDBACK 0xF001
#define ATTRID_RADIATION_BUZZER_FEEDBACK 0xF002
#define ATTRID_RADIATION_SENSORS_COUNT 0xF003
#define ATTRID_RADIATION_SENSORS_TYPE 0xF004
#define ATTRID_RADIATION_ALERT_TRESHOLD 0xF005

#define ATTRID_RADIATION_EVENTS_PER_MINUTE 0xF001
#define ATTRID_RADIATION_LEVEL_PER_HOUR 0xF002


#define ZCL_UINT8 ZCL_DATATYPE_UINT8
#define ZCL_UINT16 ZCL_DATATYPE_UINT16
#define ZCL_UINT32 ZCL_DATATYPE_UINT32
/*********************************************************************
 * TYPEDEFS
 */
typedef enum {
    SBM_20, // СБМ-20/СТС-5/BOI-33
    SBM_19 // СБМ-19/СТС-6
} RadiationSensorType_t;

typedef struct
{
    uint16 SensorSensivity;
    uint32 AlertTreshold;
    uint8 LedFeedback;
    uint8 BuzzerFeedback;
    uint8 SensorsCount;
    RadiationSensorType_t SensorType;
}  application_config_t;

/*********************************************************************
 * VARIABLES
 */

extern SimpleDescriptionFormat_t zclApp_FirstEP;
extern CONST zclAttrRec_t zclApp_AttrsFirstEP[];
extern CONST uint8 zclApp_AttrsCount;

extern const uint8 zclApp_ManufacturerName[];
extern const uint8 zclApp_ModelId[];
extern const uint8 zclApp_PowerSource;
extern uint16 zclApp_RadiationEventsPerMinute;
extern uint32 zclApp_RadiationLevelParrotsPerHour;
extern application_config_t zclApp_Config;

// APP_TODO: Declare application specific attributes here

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialization for the task
 */
extern void zclApp_Init(byte task_id);

/*
 *  Event Process for the task
 */
extern UINT16 zclApp_event_loop(byte task_id, UINT16 events);

extern void zclApp_ResetAttributesToDefaultValues(void);

/*********************************************************************
 *********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* ZCL_APP_H */
