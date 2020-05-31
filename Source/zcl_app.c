
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

afAddrType_t inderect_DstAddr = {.addrMode = (afAddrMode_t)AddrNotPresent, .endPoint = 0, .addr.shortAddr = 0};

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void zclApp_BindNotification(bdbBindNotificationData_t *data);
static void zclApp_Report(void);
// static void zclApp_Rejoin(void);

static void zclApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg);
static void zclApp_ProcessTouchlinkTargetEnable(uint8 enable);

static void zclApp_BasicResetCB(void);
static void zclApp_BasicIdentifyCB(zclIdentifyTriggerEffect_t *pCmd);
static void zclApp_RestoreAttributesFromNV(void);
static void zclApp_SaveAttributesToNV(void);

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper);
static void zclApp_RadioactiveEventCB(void);

/*********************************************************************
 * ZCL General Profile Callback table
 */
static zclGeneral_AppCallbacks_t zclApp_CmdCallbacks = {
    zclApp_BasicResetCB,    // Basic Cluster Reset command
    zclApp_BasicIdentifyCB, // Identify Trigger Effect command
    NULL,                   // On/Off cluster commands
    NULL,                   // On/Off cluster enhanced command Off with Effect
    NULL,                   // On/Off cluster enhanced command On with Recall Global Scene
    NULL,                   // On/Off cluster enhanced command On with Timed Off
    NULL,                   // RSSI Location command
    NULL                    // RSSI Location Response command
};

void zclApp_Init(byte task_id) {
    DebugInit();
    zclApp_RestoreAttributesFromNV();
    // this is important to allow connects throught routers
    // to make this work, coordinator should be compiled with this flag #define TP2_LEGACY_ZC
    requestNewTrustCenterLinkKey = FALSE;

    zclApp_TaskID = task_id;

    zclGeneral_RegisterCmdCallbacks(zclApp_FirstEP.EndPoint, &zclApp_CmdCallbacks);

    zcl_registerAttrList(zclApp_FirstEP.EndPoint, zclApp_AttrsCount, zclApp_AttrsFirstEP);
    bdb_RegisterSimpleDescriptor(&zclApp_FirstEP);

    zcl_registerReadWriteCB(zclApp_FirstEP.EndPoint, NULL, zclApp_ReadWriteAuthCB);

    zcl_registerForMsg(zclApp_TaskID);

    bdb_RegisterBindNotificationCB(zclApp_BindNotification);
    bdb_RegisterCommissioningStatusCB(zclApp_ProcessCommissioningStatus);
    bdb_RegisterTouchlinkTargetEnableCB(zclApp_ProcessTouchlinkTargetEnable);
    LREP("zgReadStartupOptions %d \r\n", zgReadStartupOptions());
    if (zgReadStartupOptions() & ZCD_STARTOPT_DEFAULT_NETWORK_STATE) {
        LREPMaster("BDB_COMMISSIONING_MODE_NWK_STEERING\r\n");
        bdb_StartCommissioning(BDB_COMMISSIONING_MODE_NWK_STEERING);
    } else {
        LREPMaster("BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP\r\n");
        bdb_StartCommissioning(BDB_COMMISSIONING_REJOIN_EXISTING_NETWORK_ON_STARTUP);
    }

    touchLinkTarget_EnableCommissioning(TOUCHLINK_TARGET_PERPETUAL);

    LREP("Started build %s \r\n", zclApp_DateCodeNT);

    ZMacSetTransmitPower(APP_TX_POWER);

    zclApp_InitCounter();
    zclApp_RegisterCounterCallback(0, zclApp_RadioactiveEventCB);
    osal_start_reload_timer(zclApp_TaskID, APP_REPORT_EVT, APP_REPORT_DELAY);

    LREP("Sensivity %d led_feedback %d buzzer_feedback %d\r\n", zclApp_RadiationSensorSensivity, zclApp_RadiationLedFeedback,
         zclApp_RadiationBuzzerFeedback);
}

static void zclApp_ProcessCommissioningStatus(bdbCommissioningModeMsg_t *bdbCommissioningModeMsg) {
    LREP("bdbCommissioningMode=%d bdbCommissioningStatus=%d bdbRemainingCommissioningModes=0x%X\r\n",
         bdbCommissioningModeMsg->bdbCommissioningMode, bdbCommissioningModeMsg->bdbCommissioningStatus,
         bdbCommissioningModeMsg->bdbRemainingCommissioningModes);
    switch (bdbCommissioningModeMsg->bdbCommissioningMode) {
    case BDB_COMMISSIONING_INITIALIZATION:
        switch (bdbCommissioningModeMsg->bdbCommissioningStatus) {
        case BDB_COMMISSIONING_NO_NETWORK:
            LREP("No network\r\n");
            HalLedBlink(HAL_LED_1, 3, 50, 500);
            break;

        default:
            break;
        }
        break;
    case BDB_COMMISSIONING_NWK_STEERING:
        switch (bdbCommissioningModeMsg->bdbCommissioningStatus) {
        case BDB_COMMISSIONING_SUCCESS:
            HalLedBlink(HAL_LED_1, 5, 50, 500);
            LREPMaster("BDB_COMMISSIONING_SUCCESS\r\n");
            break;

        default:
            HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
            break;
        }

        break;

    case BDB_COMMISSIONING_PARENT_LOST:
        LREPMaster("BDB_COMMISSIONING_PARENT_LOST\r\n");
        switch (bdbCommissioningModeMsg->bdbCommissioningStatus) {
        case BDB_COMMISSIONING_NETWORK_RESTORED:

            break;

        default:
            HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
            break;
        }
        break;
    default:
        break;
    }
}

uint16 zclApp_event_loop(uint8 task_id, uint16 events) {

    LREP("events 0x%x \r\n", events);
    afIncomingMSGPacket_t *MSGpkt;

    if (events & SYS_EVENT_MSG) {
        while ((MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(zclApp_TaskID))) {
            LREP("MSGpkt->hdr.event %d\r\n", MSGpkt->hdr.event);
            switch (MSGpkt->hdr.event) {
            default:
                break;
            }

            // Release the memory
            osal_msg_deallocate((uint8 *)MSGpkt);
        }

        // return unprocessed events
        return (events ^ SYS_EVENT_MSG);
    }

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
    return 0;
}

static void zclApp_BindNotification(bdbBindNotificationData_t *data) {
    HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);

    LREP("Recieved bind request clusterId=0x%X dstAddr=0x%X ep=%d \r\n", data->clusterId, data->dstAddr, data->ep);
    uint16 maxEntries = 0, usedEntries = 0;
    bindCapacity(&maxEntries, &usedEntries);
    LREP("bindCapacity %d %usedEntries %d \r\n", maxEntries, usedEntries);
}

static void zclApp_Report(void) {
    halIntState_t intState;
    uint16 port0Value = 0;
    HAL_ENTER_CRITICAL_SECTION(intState);
    port0Value = zclApp_Port0CounterValue;
    zclApp_Port0CounterValue = 0;
    HAL_EXIT_CRITICAL_SECTION(intState);
    LREP("pulse counter: port0Value=%d \r\n", port0Value);

    zclApp_RadiationEventsPerMinute = port0Value;
    zclApp_RadiationLevelParrotsPerHour = zclApp_RadiationEventsPerMinute * zclApp_RadiationSensorSensivity;

    bdb_RepChangedAttrValue(zclApp_FirstEP.EndPoint, ILLUMINANCE, ATTRID_RADIATION_EVENTS_PER_MINUTE);
}

static void zclApp_ProcessTouchlinkTargetEnable(uint8 enable) { LREP("zclApp_ProcessTouchlinkTargetEnable %d \r\n", enable); }

static void zclApp_BasicResetCB(void) {
    LREPMaster("BasicResetCB\r\n");
    zclApp_ResetAttributesToDefaultValues();
    zclApp_SaveAttributesToNV();
}

static void zclApp_BasicIdentifyCB(zclIdentifyTriggerEffect_t *pCmd) {
    HalLedSet(HAL_LED_1, HAL_LED_MODE_FLASH);
    LREP("zclApp_BasicIdentifyCB %d \r\n", pCmd->srcAddr);
}

static ZStatus_t zclApp_ReadWriteAuthCB(afAddrType_t *srcAddr, zclAttrRec_t *pAttr, uint8 oper) {
    LREPMaster("AUTH CB called\r\n");

    osal_start_timerEx(zclApp_TaskID, APP_SAVE_ATTRS_EVT, 2000);
    return ZSuccess;
}

static void zclApp_SaveAttributesToNV(void) {
    LREPMaster("Saving attributes to NV\r\n");

    if (osal_nv_item_init(NW_RadiationSensorSensivity, sizeof(zclApp_RadiationSensorSensivity), &zclApp_RadiationSensorSensivity) ==
        SUCCESS) {
        osal_nv_write(NW_RadiationSensorSensivity, 0, sizeof(zclApp_RadiationSensorSensivity), &zclApp_RadiationSensorSensivity);
    }

    if (osal_nv_item_init(NW_RadiationSensorLedFeedback, sizeof(zclApp_RadiationLedFeedback), &zclApp_RadiationLedFeedback) == SUCCESS) {
        osal_nv_write(NW_RadiationSensorLedFeedback, 0, sizeof(zclApp_RadiationLedFeedback), &zclApp_RadiationLedFeedback);
    }

    if (osal_nv_item_init(NW_RadiationSensorBuzzerFeedback, sizeof(zclApp_RadiationBuzzerFeedback), &zclApp_RadiationBuzzerFeedback) ==
        SUCCESS) {
        osal_nv_write(NW_RadiationSensorBuzzerFeedback, 0, sizeof(zclApp_RadiationBuzzerFeedback), &zclApp_RadiationBuzzerFeedback);
    }
}

static void zclApp_RestoreAttributesFromNV(void) {
    LREPMaster("Restoring attributes to NV\r\n");
    if (osal_nv_item_init(NW_RadiationSensorSensivity, sizeof(zclApp_RadiationSensorSensivity), &zclApp_RadiationSensorSensivity) ==
        ZSUCCESS) {
        osal_nv_read(NW_RadiationSensorSensivity, 0, sizeof(zclApp_RadiationSensorSensivity), &zclApp_RadiationSensorSensivity);
    }

    if (osal_nv_item_init(NW_RadiationSensorLedFeedback, sizeof(zclApp_RadiationLedFeedback), &zclApp_RadiationLedFeedback) == ZSUCCESS)
    {
        osal_nv_read(NW_RadiationSensorLedFeedback, 0, sizeof(zclApp_RadiationLedFeedback), &zclApp_RadiationLedFeedback);
    }

    if (osal_nv_item_init(NW_RadiationSensorBuzzerFeedback, sizeof(zclApp_RadiationBuzzerFeedback), &zclApp_RadiationBuzzerFeedback) ==
        ZSUCCESS) {
        osal_nv_read(NW_RadiationSensorBuzzerFeedback, 0, sizeof(zclApp_RadiationBuzzerFeedback), &zclApp_RadiationBuzzerFeedback);
    }
}

void zclApp_RadioactiveEventCB(void) {
    LREPMaster("Event \r\n");
    if (zclApp_RadiationLedFeedback) {
        HalLedSet(HAL_LED_1, HAL_LED_MODE_BLINK);
    }

    if (zclApp_RadiationBuzzerFeedback) {
        // TODO: buzzer feedback
    }
}


// ZStatus_t zclApp_ReadWriteAttrCB( uint16 clusterId, uint16 attrId, uint8 oper,
//                                          uint8 *pValue, uint16 *pLen )
// {

//     ZStatus_t status = ZSuccess;
//   uint16 tempAttr;
//   uint32 attrValue;
//   afIncomingMSGPacket_t *origPkt;

//   origPkt = zcl_getRawAFMsg();

//   switch ( oper )
//   {
//     case ZCL_OPER_LEN:
//     switch (attrId)
//     {
//     case ATTRID_RADIATION_SENSOR_SENSIVITY:
//         break;
//         case ATTRID_RADIATION_LED_FEEDBACK:
//         case 
    
//     default:
//         break;
//     }
//       if ( ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI ) ||
//            ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI ) )
//       {
//         *pLen = 1;
//       }
//       else if ( attrId == ATTRID_DIAGNOSTIC_AVERAGE_MAC_RETRY_PER_APS_MESSAGE_SENT )
//       {
//         *pLen = 2;
//       }
//       // The next function call only returns the length for attributes that are defined
//       // in lower layers
//       else if ( zclDiagnostic_GetAttribData( attrId, &tempAttr, pLen ) != ZSuccess )
//       {
//         *pLen = 0;
//         status = ZFailure;  // invalid length
//       }
//       break;

//     case ZCL_OPER_READ:
//       // Identify if incoming msg is LQI or RSSI attribute
//       // and return the LQI and RSSI of the incoming values
//       if ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_LQI )
//       {
//         *pLen = 1;
//         attrValue = origPkt->LinkQuality;
//       }
//       else if ( attrId == ATTRID_DIAGNOSTIC_LAST_MESSAGE_RSSI )
//       {
//         //origPkt = zcl_getRawAFMsg();
//         *pLen = 1;
//         attrValue = origPkt->rssi;
//       }
//       else if ( zclDiagnostic_GetStatsAttr( attrId, &attrValue, pLen ) == ZSuccess )
//       {
//         if ( ( attrId == ATTRID_DIAGNOSTIC_MAC_TX_UCAST_RETRY ) ||
//              ( attrId == ATTRID_DIAGNOSTIC_MAC_TX_UCAST_FAIL  ) )
//         {
//           // The lower layer counter is a 32 bit counter, report the higher 16 bit value
//           // util the lower layer counter wraps-up
//           if ( attrValue > 0x0000FFFF )
//           {
//             attrValue = 0x0000FFFF;
//           }
//         }
//       }
//       else
//       {
//         *pLen = 0;
//         status = ZFailure;  // invalid attribute
//       }

//       if ( *pLen == 1 )
//       {
//         pValue[0] = BREAK_UINT32( attrValue, 0 );
//       }
//       else if ( *pLen == 2 )
//       {
//         pValue[0] = LO_UINT16( attrValue );
//         pValue[1] = HI_UINT16( attrValue );
//       }
//       else if ( *pLen == 4 )
//       {
//         pValue[0] = BREAK_UINT32( attrValue, 0 );
//         pValue[1] = BREAK_UINT32( attrValue, 1 );
//         pValue[2] = BREAK_UINT32( attrValue, 2 );
//         pValue[3] = BREAK_UINT32( attrValue, 3 );
//       }

//       break;

//     case ZCL_OPER_WRITE:
//       status = ZFailure;  // All attributes in Diagnostics cluster are READ ONLY
//       break;
//   }

//   return ( status );
// }

/****************************************************************************
****************************************************************************/
