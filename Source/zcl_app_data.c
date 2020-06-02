#include "AF.h"
#include "OSAL.h"
#include "ZComDef.h"
#include "ZDConfig.h"

#include "zcl.h"
#include "zcl_general.h"
#include "zcl_ha.h"
#include "zcl_ms.h"

#include "zcl_app.h"

#include "version.h"

#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"
#include "stub_aps.h"

/*********************************************************************
 * CONSTANTS
 */

#define APP_DEVICE_VERSION 2
#define APP_FLAGS 0

#define APP_HWVERSION 1
#define APP_ZCLVERSION 1

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Global attributes
const uint16 zclApp_clusterRevision_all = 0x0001;

// Basic Cluster
const uint8 zclApp_HWRevision = APP_HWVERSION;
const uint8 zclApp_ZCLVersion = APP_ZCLVERSION;
const uint8 zclApp_ApplicationVersion = 2;
const uint8 zclApp_StackVersion = 4;

//{lenght, 'd', 'a', 't', 'a'}
const uint8 zclApp_ManufacturerName[] = {9, 'm', 'o', 'd', 'k', 'a', 'm', '.', 'r', 'u'};
const uint8 zclApp_ModelId[] = {13, 'D', 'I', 'Y', 'R', 'u', 'Z', '_', 'G', 'e', 'i', 'g', 'e', 'r'};
const uint8 zclApp_PowerSource = POWER_SOURCE_MAINS_1_PHASE;

#define DEFAULT_SensorSensivity 65
#define DEFAULT_AlertTreshold 100
#define DEFAULT_LedFeedback TRUE
#define DEFAULT_BuzzerFeedback FALSE
#define DEFAULT_SensorsCount 1
#define DEFAULT_SensorType SBM_20

application_config_t zclApp_Config = {.SensorSensivity = DEFAULT_SensorSensivity,
                                      .AlertTreshold = DEFAULT_AlertTreshold,
                                      .LedFeedback = DEFAULT_LedFeedback,
                                      .BuzzerFeedback = DEFAULT_BuzzerFeedback,
                                      .SensorsCount = DEFAULT_SensorsCount,
                                      .SensorType = DEFAULT_SensorType};

uint16 zclApp_RadiationEventsPerMinute = 0;     // pulses per minute
uint32 zclApp_RadiationLevelParrotsPerHour = 0; // parrots per hour

/*********************************************************************
 * ATTRIBUTE DEFINITIONS - Uses REAL cluster IDs
 */

CONST zclAttrRec_t zclApp_AttrsFirstEP[] = {
    {BASIC, {ATTRID_BASIC_APPL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ApplicationVersion}},
    {BASIC, {ATTRID_BASIC_STACK_VERSION, ZCL_UINT8, R, (void *)&zclApp_StackVersion}},
    {BASIC, {ATTRID_BASIC_HW_VERSION, ZCL_UINT8, R, (void *)&zclApp_HWRevision}},
    {BASIC, {ATTRID_BASIC_ZCL_VERSION, ZCL_UINT8, R, (void *)&zclApp_ZCLVersion}},
    {BASIC, {ATTRID_BASIC_MANUFACTURER_NAME, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ManufacturerName}},
    {BASIC, {ATTRID_BASIC_MODEL_ID, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_ModelId}},
    {BASIC, {ATTRID_BASIC_POWER_SOURCE, ZCL_DATATYPE_ENUM8, R, (void *)&zclApp_PowerSource}},
    {BASIC, {ATTRID_CLUSTER_REVISION, ZCL_UINT16, R, (void *)&zclApp_clusterRevision_all}},
    {BASIC, {ATTRID_BASIC_DATE_CODE, ZCL_DATATYPE_CHAR_STR, R, (void *)zclApp_DateCode}},
    {BASIC, {ATTRID_BASIC_SW_BUILD_ID, ZCL_UINT8, R, (void *)&zclApp_ApplicationVersion}},
    {ILLUMINANCE, {ATTRID_RADIATION_EVENTS_PER_MINUTE, ZCL_UINT16, RR, (void *)&zclApp_RadiationEventsPerMinute}},
    {ILLUMINANCE, {ATTRID_RADIATION_LEVEL_PER_HOUR, ZCL_UINT32, RR, (void *)&zclApp_RadiationLevelParrotsPerHour}},

    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_SENSOR_SENSIVITY, ZCL_UINT16, RW, (void *)&zclApp_Config.SensorSensivity}},
    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_LED_FEEDBACK, ZCL_DATATYPE_BOOLEAN, RW, (void *)&zclApp_Config.LedFeedback}},
    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_BUZZER_FEEDBACK, ZCL_DATATYPE_BOOLEAN, RW, (void *)&zclApp_Config.BuzzerFeedback}},
    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_SENSORS_COUNT, ZCL_UINT8, RW, (void *)&zclApp_Config.SensorsCount}},
    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_SENSORS_TYPE, ZCL_DATATYPE_ENUM8, RW, (void *)&zclApp_Config.SensorType}},
    {ILLUMINANCE_CONFIG, {ATTRID_RADIATION_ALERT_TRESHOLD, ZCL_DATATYPE_UINT16, RW, (void *)&zclApp_Config.AlertTreshold}},

};

uint8 CONST zclApp_AttrsCount = (sizeof(zclApp_AttrsFirstEP) / sizeof(zclApp_AttrsFirstEP[0]));

const cId_t zclApp_InClusterList[] = {ZCL_CLUSTER_ID_GEN_BASIC};

#define APP_MAX_INCLUSTERS (sizeof(zclApp_InClusterList) / sizeof(zclApp_InClusterList[0]))

const cId_t zclApp_OutClusterList[] = {ILLUMINANCE};

#define APP_MAX_OUT_CLUSTERS (sizeof(zclApp_OutClusterList) / sizeof(zclApp_OutClusterList[0]))

SimpleDescriptionFormat_t zclApp_FirstEP = {
    1,                             //  int Endpoint;
    ZCL_HA_PROFILE_ID,             //  uint16 AppProfId[2];
    ZCL_HA_DEVICEID_SIMPLE_SENSOR, //  uint16 AppDeviceId[2];
    APP_DEVICE_VERSION,            //  int   AppDevVer:4;
    APP_FLAGS,                     //  int   AppFlags:4;
    APP_MAX_INCLUSTERS,            //  byte  AppNumInClusters;
    (cId_t *)zclApp_InClusterList, //  byte *pAppInClusterList;
    APP_MAX_OUT_CLUSTERS,          //  byte  AppNumInClusters;
    (cId_t *)zclApp_OutClusterList //  byte *pAppInClusterList;
};

void zclApp_ResetAttributesToDefaultValues(void) {

    zclApp_Config.SensorSensivity = DEFAULT_SensorSensivity;
    zclApp_Config.AlertTreshold = DEFAULT_AlertTreshold;
    zclApp_Config.LedFeedback = DEFAULT_LedFeedback;
    zclApp_Config.BuzzerFeedback = DEFAULT_BuzzerFeedback;
    zclApp_Config.SensorsCount = DEFAULT_SensorsCount;
    zclApp_Config.SensorType = DEFAULT_SensorType;
}