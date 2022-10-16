#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "BLELogService.h"
#include "ble_srv_common.h"

#include "pxt.h"
#include "MicroBit.h"

// Advertising includes
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_conn_params.h"
#include "ble_dis.h"

#include "debug.h"

using namespace pxt;

//////////////// Initialize static members

const uint16_t BLELogService::bleLogService = 0xFFFF; 

const uint16_t BLELogService::charUUID[mbbs_cIdxCOUNT] = {  
  // Alternate values to facilitate debugging
   0x2A4E,  //  ProtocolMode
//   0x2A4A,  //  HIDInfo
//   0x2A4B,  //  Report Map
//   0x2A4D,  //  Report 0
//   0x2A4D,  //  Report 1
//   0x2A4D,  //  Report 2
//   0x2A4D,  //  Report 3
};

uint16_t BLELogService::HIDInfo[2] = { 
  0x0111,
  0x0002
};

const int BLELogService::EVT_STATUS = 1;  // Event for connect / disconnect; 

BLELogService *BLELogService::service = NULL; // Singleton reference to the service

// Facilitate debugging Peer_manager events
// static const char * m_event_str[] =
// {
//     "PM_EVT_BONDED_PEER_CONNECTED",
//     "PM_EVT_CONN_SEC_START",
//     "PM_EVT_CONN_SEC_SUCCEEDED",
//     "PM_EVT_CONN_SEC_FAILED",
//     "PM_EVT_CONN_SEC_CONFIG_REQ",
//     "PM_EVT_CONN_SEC_PARAMS_REQ",
//     "PM_EVT_STORAGE_FULL",
//     "PM_EVT_ERROR_UNEXPECTED",
//     "PM_EVT_PEER_DATA_UPDATE_SUCCEEDED",
//     "PM_EVT_PEER_DATA_UPDATE_FAILED",
//     "PM_EVT_PEER_DELETE_SUCCEEDED",
//     "PM_EVT_PEER_DELETE_FAILED",
//     "PM_EVT_PEERS_DELETE_SUCCEEDED",
//     "PM_EVT_PEERS_DELETE_FAILED",
//     "PM_EVT_LOCAL_DB_CACHE_APPLIED",
//     "PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED",
//     "PM_EVT_SERVICE_CHANGED_IND_SENT",
//     "PM_EVT_SERVICE_CHANGED_IND_CONFIRMED",
//     "PM_EVT_SLAVE_SECURITY_REQ",
//     "PM_EVT_FLASH_GARBAGE_COLLECTED",
//     "PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED",
// };




// Static method for peer_manager events (Bounce it to the instance, which has access to member vars)
void BLELogService::static_pm_events(const pm_evt_t* p_event) {
  getInstance()->pm_events(p_event);
}


void BLELogService::pm_events(const pm_evt_t* p_event) {
  //DEBUG("PM Event %s conn %d, peer %d\n",m_event_str[p_event->evt_id], p_event->conn_handle,  p_event->peer_id );
  if(p_event->evt_id == PM_EVT_PEER_DATA_UPDATE_SUCCEEDED) {
    // DEBUG("data %d, action %d, token %d, flash changed %d\n", 
    //   p_event->params.peer_data_update_succeeded.data_id,
    //   p_event->params.peer_data_update_succeeded.action,
    //   p_event->params.peer_data_update_succeeded.token,
    //   p_event->params.peer_data_update_succeeded.flash_changed);

    // TODO / REVIEW:  This works, but I'm not entirely sure it's correct. 
    //   It assumes that CCCDs are re-set to 0 sometime (when disconnecting or when connecting to an unbonded device)

    // // Iterate through the report characteristics to see if any have CCCD enabled
    // for(int i=mbbs_cIdxReport1, idx=0; i<mbbs_cIdxCOUNT;i++, idx++) {

    //   // Get the CCCD
    //   ble_gatts_value_t data;
    //   memset(&data, 0, sizeof(ble_gatts_value_t));
    //   uint16_t value;
    //   data.len = 2;
    //   data.p_value = (uint8_t*)&value;
    //   sd_ble_gatts_value_get(p_event->conn_handle, charHandles(i)->cccd, &data); 

    // //   // Update the reporters
    // //   int reporterIdx = i-mbbs_cIdxReport1;
    // //   if(reporters[reporterIdx]) {
    // //     reporters[i-mbbs_cIdxReport1]->setEnabled(value ? true : false);
    // //   }
    //   // Update the internal characteristic flags
    //   chars[i].setCCCD(value);
    // }
  }
}

/**
 */
BLELogService *BLELogService::getInstance()
{
    if (service == NULL)
    {
        service = new BLELogService();
    }
    return service;
}

/** 
 * Constructor.
 * Create a representation of the Bluetooth Data Logger Service
 */
BLELogService::BLELogService() 
{

  DEBUG("BLELog Serv starting\n");

  advertise();


  // Register the base UUID and create the service.
  bs_uuid_type = BLE_UUID_TYPE_BLE;  // Set the UUID type to 0x01, which should be Bluetooth SIG ID
  CreateService( bleLogService );

  // Create the data structures that represent each of our characteristics in Soft Device.
  // iOS needs protocol mode characteristic for keyboard support
  CreateCharacteristic( mbbs_cIdxProtocolMode, charUUID[ mbbs_cIdxProtocolMode ],
                      (uint8_t *)&protocolMode,
                      sizeof(protocolMode), sizeof(protocolMode),
                      microbit_propREAD | microbit_propWRITE_WITHOUT ); 

//   CreateCharacteristic( mbbs_cIdxHIDInfo, charUUID[ mbbs_cIdxHIDInfo ],
//                       (uint8_t *)HIDInfo,
//                       sizeof(HIDInfo), sizeof(HIDInfo),
//                       microbit_propREAD );

//   memset(reportMap, 0, reportMapMaxSize);
//   CreateCharacteristic( mbbs_cIdxReportMap, charUUID[ mbbs_cIdxReportMap ],
//                       (uint8_t *)reportMap,
//                       0, reportMapMaxSize,
//                       microbit_propREAD | microbit_propREADAUTH );

//   for(int i=mbbs_cIdxReport1, idx=0; i<mbbs_cIdxCOUNT;i++, idx++) {
//     memset(reports[idx], 0, reportMaxSize);
//     CreateCharacteristic(i, charUUID[i],
//                         reports[idx],
//                         0, reportMaxSize,
//                         microbit_propREAD  | microbit_propNOTIFY | microbit_propREADAUTH);
//     // Must have report discriptor for OS detection
//     // NOTE: Assuming INPUT reports
//     // Report indices are 1-based and in order of addition
//     addReportDescriptor(charHandles(i)->value, i-mbbs_cIdxReport1+1, 1 /* Input report */);
//   } 
  pm_register(static_pm_events); 

  advertise();

}

/**
  * Invoked when BLE connects.
  */
void BLELogService::onConnect( const microbit_ble_evt_t *p_ble_evt)
{
  DEBUG("BLE Log onConnect\n");
  // Reload Peer data 
}

/**
  * Invoked when BLE disconnects.
  */
void BLELogService::onDisconnect( const microbit_ble_evt_t *p_ble_evt)
{
    DEBUG("BLE Log onDisconnect\n");
}

void BLELogService::onDataRead( microbit_onDataRead_t *params) {
      DEBUG("BLE Log onDataRead\n");
      debugAttribute(params->handle);
      microbit_charattr_t type;
    //   int index = charHandleToIdx(params->handle, &type);
    //   int offset = params->offset;
    //   if(index == mbbs_cIdxReportMap && type == microbit_charattrVALUE) {
    //     params->data = &(reportMap[offset]);
    //     params->length = max(reportMapUsed-offset,0);  // Remaining data
    //   }
}

/**
  * Callback. Invoked when any of our attributes are written via BLE.
  */
void BLELogService::onDataWritten( const microbit_ble_evt_write_t *params)
{
    DEBUG("BLE Log onDataWritten\n");
    debugAttribute(params->handle);

//     microbit_charattr_t type;
//     int index = charHandleToIdx(params->handle, &type);

//     if(index>=mbbs_cIdxReport1 && index<=mbbs_cIdxCOUNT && type == microbit_charattrCCCD) {
//       DEBUG("BLE Log Report CCCD Changed\n");
//       bool status = params->len>0 && params->data[0] ? true : false;
//       int reporterIdx = index-mbbs_cIdxReport1;
//       HIDReporter *theReporter = reporters[reporterIdx];
//       if(theReporter!=NULL) {
//         theReporter->setEnabled(status);
//       }
//   } 
}

bool BLELogService::onBleEvent(const microbit_ble_evt_t *p_ble_evt) {
    //DEBUG("onBleEvent id = %d\n", p_ble_evt->header.evt_id);
    // Let usual process handle it. 
    return MicroBitBLEService::onBleEvent(p_ble_evt);
}

bool BLELogService::notifyChrValue( int idx, const uint8_t *data, uint16_t length) {
    // Throttle the BLE traffic to avoid flooding
    // static unsigned lastSend = 0;
    // unsigned now = uBit.systemTime();
    // int diff = now-lastSend;
    // if(diff<minTimeBetweenNotifies) {
    //     uBit.sleep(diff);
    // }
    // lastSend = now;
    return MicroBitBLEService::notifyChrValue( idx, data, length);
}

void BLELogService::setName() {
    // set fixed gap name
    // Name has to be <= 12 chars (to fit Adv packet)  
    //  Maybe:  "mcrobt XXXXX"   or "uBit [XXXXX]"   
    int len = sprintf(gapName, "uBit [%s]", microbit_friendly_name());
    ble_gap_conn_sec_mode_t permissions;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS( &permissions);
    MICROBIT_BLE_ECHK( sd_ble_gap_device_name_set( &permissions, (uint8_t *)gapName, len) );
}

void BLELogService::advertise() {
    DEBUG("advertise() Starting\n");
      // Stop any active advertising
        uBit.bleManager.stopAdvertising();
        setName();
        // m_advdata _must_ be static / retained!
        static ble_advdata_t m_advdata;
        // m_enc_advdata _must_ be static / retained!
        static uint8_t  m_enc_advdata[BLE_GAP_ADV_SET_DATA_SIZE_MAX];
        static ble_uuid_t uuid;  // UUID Struct
        uint8_t m_adv_handle;
 
        uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN ;
        uuid.uuid = 0x182; // 1812 is HID 
        m_advdata.uuids_complete.uuid_cnt = 1;
        m_advdata.uuids_complete.p_uuids = &uuid;
        m_advdata.include_appearance = true;
        // Name needed to be identified by Android
        m_advdata.name_type = BLE_ADVDATA_FULL_NAME;
        
        // Appearance isn't strictly needed for detection 
        sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HID );

        // The flags below ensure "pairing mode" so it shows up in Android
        m_advdata.flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED | BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE;

        ble_gap_adv_params_t    gap_adv_params;
        memset( &gap_adv_params, 0, sizeof( gap_adv_params));
        gap_adv_params.properties.type  = true /* connectable */
                                        ? BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED
                                        : BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;
        gap_adv_params.interval         = ( 1000 * MICROBIT_BLE_ADVERTISING_INTERVAL/* interval_ms */) / 625;  // 625 us units
        if ( gap_adv_params.interval < BLE_GAP_ADV_INTERVAL_MIN) gap_adv_params.interval = BLE_GAP_ADV_INTERVAL_MIN;
        if ( gap_adv_params.interval > BLE_GAP_ADV_INTERVAL_MAX) gap_adv_params.interval = BLE_GAP_ADV_INTERVAL_MAX;
        gap_adv_params.duration         = MICROBIT_BLE_ADVERTISING_TIMEOUT /* timeout_seconds */ * 100;              //10 ms units
        gap_adv_params.filter_policy    = false /* whitelist */
                                        ? BLE_GAP_ADV_FP_FILTER_BOTH
                                        : BLE_GAP_ADV_FP_ANY;
        gap_adv_params.primary_phy      = BLE_GAP_PHY_1MBPS;
                    
        ble_gap_adv_data_t  gap_adv_data;
        memset( &gap_adv_data, 0, sizeof( gap_adv_data));
        gap_adv_data.adv_data.p_data    = m_enc_advdata;
        gap_adv_data.adv_data.len       = BLE_GAP_ADV_SET_DATA_SIZE_MAX;

        MICROBIT_BLE_ECHK( ble_advdata_encode( &m_advdata, gap_adv_data.adv_data.p_data, &gap_adv_data.adv_data.len));
        MICROBIT_BLE_ECHK( sd_ble_gap_adv_set_configure( &m_adv_handle, &gap_adv_data, &gap_adv_params));

        // Restart advertising
        // TODO / FIXME / REVIEW / WARNING: This will start adv using the static handle in the BLE Manager. 
        // Hopefully the same handle is used as the one returned by sd_ble_gap_adv_set_configure
        uBit.bleManager.advertise();
    } 

void BLELogService::debugAttribute(int handle) {
#ifdef DEBUG_ENABLED
      microbit_charattr_t type;
      int index = charHandleToIdx(handle, &type);

      const char *typeName;
      switch(type) {
        case microbit_charattrVALUE:
          typeName = "Value";
          break;
        case microbit_charattrDESC:
          typeName = "Desc";
          break;
        case microbit_charattrCCCD:
          typeName = "CCCD";
          break;
        case microbit_charattrSCCD:
          typeName = "SCCD";
          break;
        default:
          typeName = "UNKNOWN";
      }
      if(index<0 || index>=mbbs_cIdxCOUNT) index = 4;
      char const *charNames[] = {"Protocol", "Info", "Map", "Report", "Invalid"};
    //   if(index>=mbbs_cIdxReport1 && index<mbbs_cIdxCOUNT) 
    //   {
    //       int report = index-mbbs_cIdxReport1;
    //       index=3;
    //       if(reporters[report]) {
    //         DEBUG("     %s %s (%d : %s)\n", charNames[index], typeName, report, reporters[report]->name);
    //       } else {
    //         DEBUG("     %s %s (%d) (BAD INDEX)\n", charNames[index], typeName, report);
    //       }
    //   } else {
    //     DEBUG("     %s %s\n", charNames[index], typeName);
    //   }
#endif
}
#endif