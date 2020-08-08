
#include "AF.h"
#include "OSAL.h"
#include "OSAL_Clock.h"
#include "OSAL_PwrMgr.h"
#include "ZComDef.h"
#include "ZDApp.h"
#include "ZDObject.h"
#include "math.h"

#include "nwk_util.h"
#include "zcl.h"
#include "zcl_app.h"
#include "zcl_diagnostic.h"
#include "zcl_general.h"
#include "zcl_lighting.h"
#include "zcl_ms.h"

#include "bdb.h"
#include "bdb_interface.h"
#include "bdb_touchlink.h"
#include "bdb_touchlink_target.h"

#include "gp_interface.h"

#include "Debug.h"

#include "onboard.h"

/* HAL */
#include "hal_drivers.h"
#include "hal_key.h"
#include "hal_led.h"
#include "isr_counter.h"

#include "version.h"

#include "calculate_urh.h"

#include "DebugTrace.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

extern bool requestNewTrustCenterLinkKey;
byte zclApp_TaskID;

/*********************************************************************
 * GLOBAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTIONS
 */


static void zclApp_Report(void);
static void zclApp_BasicResetCB(void);
static void zclApp_RestoreAttributesFromNV(void);
static void zclApp_SaveAttributesToNV(void);

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper);
static void zclApp_RadioactiveEventCB(uint8 portNum);

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclApp_CmdCallbacks = {
    zclApp_BasicResetCB, // Basic Cluster Reset command
    NULL,                // Identify Trigger Effect command
    NULL,                // On/Off cluster commands
    NULL,                // On/Off cluster enhanced command Off with Effect
    NULL,                // On/Off cluster enhanced command On with Recall Global Scene
    NULL,                // On/Off cluster enhanced command On with Timed Off
    NULL,                // RSSI Location command
    NULL                 // RSSI Location Response command
};
void zclApp_Init(byte task_id) {
    // debug_str("Hello \r\n");
    DebugInit();
    zclApp_RestoreAttributesFromNV();
    LREP("Sensivity %d led %d buzzer %d \r\n", zclApp_Config.SensorSensivity, zclApp_Config.LedFeedback, zclApp_Config.BuzzerFeedback);
    // this is important to allow connects throught routers
    // to make this work, coordinator should be compiled with this flag #define TP2_LEGACY_ZC
    requestNewTrustCenterLinkKey = FALSE;

    zclApp_TaskID = task_id;

    bdb_RegisterSimpleDescriptor(&zclApp_FirstEP);
    zclGeneral_RegisterCmdCallbacks(zclApp_FirstEP.EndPoint, &zclApp_CmdCallbacks);

    zcl_registerAttrList(zclApp_FirstEP.EndPoint, zclApp_AttrsCount, zclApp_AttrsFirstEP);

    zcl_registerReadWriteCB(zclApp_FirstEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);
    zcl_registerForMsg(zclApp_TaskID);


    ZMacSetTransmitPower(APP_TX_POWER);


    LREP("Build %s \r\n", zclApp_DateCodeNT);

    zclApp_InitCounter();
    zclApp_RegisterCounterCallback(zclApp_RadioactiveEventCB);
    osal_start_reload_timer(zclApp_TaskID, APP_REPORT_EVT, APP_REPORT_DELAY);
}

uint16 zclApp_event_loop(uint8 task_id, uint16 events) {
    LREP("events 0x%x \r\n", events);
    if (events & APP_REPORT_EVT) {
        LREPMaster("APP_REPORT_EVT\r\n");
        zclApp_Report();
        return (events ^ APP_REPORT_EVT);
    }

    if (events & APP_SAVE_ATTRS_EVT) {
        LREPMaster("APP_SAVE_ATTRS_EVT\r\n");
        zclApp_SaveAttributesToNV();
        return (events ^ APP_SAVE_ATTRS_EVT);
    }

    if (events & APP_BLINK_EVT) {
        LREPMaster("APP_BLINK_EVT\r\n");
        HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
        return (events ^ APP_BLINK_EVT);
    }
    return 0;
}


bool lastAlertStatus = FALSE;

static void zclApp_Report(void) {
    halIntState_t intState;

    HAL_ENTER_CRITICAL_SECTION(intState);
    zclApp_RadiationEventsPerMinute = zclApp_Port0CounterValue;
    zclApp_Port0CounterValue = 0;
    HAL_EXIT_CRITICAL_SECTION(intState);

    float countsPerSecond = zclApp_RadiationEventsPerMinute / 60.0 / zclApp_Config.SensorsCount;

    switch (zclApp_Config.SensorType) {
    case SBM_19:
        zclApp_RadiationLevelParrotsPerHour = calculate_urh_sbm19(countsPerSecond);
        break;

    case SBM_20:
        zclApp_RadiationLevelParrotsPerHour = calculate_urh_sbm20(countsPerSecond);
        break;

        // TODO: Add other sensors here

    default:
        zclApp_RadiationLevelParrotsPerHour = (uint32) (countsPerSecond * zclApp_Config.SensorSensivity);
        break;
    }
    bool alertStatus = zclApp_RadiationLevelParrotsPerHour > zclApp_Config.AlertTreshold;
    afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};
    LREP("AlertTreshold=%ld radiationLevel=%ld\r\n", zclApp_Config.AlertTreshold, zclApp_RadiationLevelParrotsPerHour);
    LREP("alertStatus=%d lastAlertStatus=%d\r\n", alertStatus, lastAlertStatus);

    if (alertStatus != lastAlertStatus) {
        if (alertStatus) {
            zclGeneral_SendOnOff_CmdOn(zclApp_FirstEP.EndPoint, &inderect_DstAddr, FALSE, bdb_getZCLFrameCounter());
        } else {
            zclGeneral_SendOnOff_CmdOff(zclApp_FirstEP.EndPoint, &inderect_DstAddr, FALSE, bdb_getZCLFrameCounter());
        }
        lastAlertStatus = alertStatus;
    }

    LREP("RadiationEventsPerMinute=%d \r\n", zclApp_RadiationEventsPerMinute);
    LREP("s_type=%d s_count=%d radiation(mR/h)=%ld\r\n", zclApp_Config.SensorType, zclApp_Config.SensorsCount, zclApp_RadiationLevelParrotsPerHour);

    bdb_RepChangedAttrValue(zclApp_FirstEP.EndPoint, ILLUMINANCE, ATTRID_RADIATION_EVENTS_PER_MINUTE);
}

static void zclApp_BasicResetCB(void) {
    LREPMaster("BasicResetCB\r\n");
    zclApp_ResetAttributesToDefaultValues();
    zclApp_SaveAttributesToNV();
}

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper) {
    LREPMaster("AUTH CB called\r\n");

    osal_start_timerEx(zclApp_TaskID, APP_SAVE_ATTRS_EVT, 2000);
    return ZSuccess;
}

static void zclApp_SaveAttributesToNV(void) {
    uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    LREP("Saving attributes to NV write=%d\r\n", writeStatus);
}

static void zclApp_RestoreAttributesFromNV(void) {
    uint8 status = osal_nv_item_init(NW_APP_CONFIG, sizeof(application_config_t), NULL);
    LREP("Restoring attributes from NV  status=%d \r\n", status);
    if (status == NV_ITEM_UNINIT) {
        uint8 writeStatus = osal_nv_write(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
        LREP("NV was empty, writing %d\r\n", writeStatus);
    }
    if (status == ZSUCCESS) {
        LREPMaster("Reading from NV\r\n");
        osal_nv_read(NW_APP_CONFIG, 0, sizeof(application_config_t), &zclApp_Config);
    }
}

void zclApp_RadioactiveEventCB(uint8 portNum) {
    LREP("zclApp_RadioactiveEventCB port=%d led=%d buzzer=%d\r\n", portNum, zclApp_Config.LedFeedback, zclApp_Config.BuzzerFeedback);
    if (portNum == 0) {
        if (zclApp_Config.LedFeedback) {
            osal_set_event(zclApp_TaskID, APP_BLINK_EVT);
        }

        if (zclApp_Config.BuzzerFeedback) {
            // TODO: buzzer feedback
        }
    }
}
/****************************************************************************
****************************************************************************/
