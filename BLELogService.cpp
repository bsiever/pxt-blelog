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

const uint16_t BLELogService::bleLogService = 0x4ce4; 

const uint16_t BLELogService::charUUID[mbbs_cIdxCOUNT] = {  
  0x4f64,   // mbls_cIdxSecurity,     // Read / Notify 
  0x50a4,   // mbls_cIdxPassphrase    // Write
  0x520c,   // mbls_cIdxDataLength,   // Read/Notify 
  0x53ba,   // mbls_cIdxData,         // Notify 
  0x552c,   // mbls_cIdxDataRequest   // Write
  0x5946,   // mbls_cIdxErase,        // Write (request)
  0x5be4,   // mbls_cIdxUsage,        // Read/Notify
  0x5dd8    // mbls_cIdxTime          // Read
  // ?? FULL???
};

/*

service:  accb4ce4-8a4b-11ed-a1eb-0242ac120002

accb4f64-8a4b-11ed-a1eb-0242ac120002
accb50a4-8a4b-11ed-a1eb-0242ac120002
accb520c-8a4b-11ed-a1eb-0242ac120002
accb53ba-8a4b-11ed-a1eb-0242ac120002
accb552c-8a4b-11ed-a1eb-0242ac120002
accb5946-8a4b-11ed-a1eb-0242ac120002
accb5be4-8a4b-11ed-a1eb-0242ac120002
accb5dd8-8a4b-11ed-a1eb-0242ac120002

Spares:
accb5f72-8a4b-11ed-a1eb-0242ac120002
accb613e-8a4b-11ed-a1eb-0242ac120002
accb6332-8a4b-11ed-a1eb-0242ac120002

*/


BLELogService *BLELogService::service = NULL; // Singleton reference to the service

// Facilitate debugging Peer_manager events
static const char * m_event_str[] =
{
    "PM_EVT_BONDED_PEER_CONNECTED",
    "PM_EVT_CONN_SEC_START",
    "PM_EVT_CONN_SEC_SUCCEEDED",
    "PM_EVT_CONN_SEC_FAILED",
    "PM_EVT_CONN_SEC_CONFIG_REQ",
    "PM_EVT_CONN_SEC_PARAMS_REQ",
    "PM_EVT_STORAGE_FULL",
    "PM_EVT_ERROR_UNEXPECTED",
    "PM_EVT_PEER_DATA_UPDATE_SUCCEEDED",
    "PM_EVT_PEER_DATA_UPDATE_FAILED",
    "PM_EVT_PEER_DELETE_SUCCEEDED",
    "PM_EVT_PEER_DELETE_FAILED",
    "PM_EVT_PEERS_DELETE_SUCCEEDED",
    "PM_EVT_PEERS_DELETE_FAILED",
    "PM_EVT_LOCAL_DB_CACHE_APPLIED",
    "PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED",
    "PM_EVT_SERVICE_CHANGED_IND_SENT",
    "PM_EVT_SERVICE_CHANGED_IND_CONFIRMED",
    "PM_EVT_SLAVE_SECURITY_REQ",
    "PM_EVT_FLASH_GARBAGE_COLLECTED",
    "PM_EVT_FLASH_GARBAGE_COLLECTION_FAILED",
};




// Static method for peer_manager events (Bounce it to the instance, which has access to member vars)
void BLELogService::static_pm_events(const pm_evt_t* p_event) {
  getInstance()->pm_events(p_event);
}


void BLELogService::pm_events(const pm_evt_t* p_event) {
  DEBUG("PM Event %s conn %d, peer %d\n",m_event_str[p_event->evt_id], p_event->conn_handle,  p_event->peer_id );
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

void BLELogService::setPassphrase(const char *newPassphrase) {
  if(newPassphrase && strlen(newPassphrase)>0) {
    strncpy(passphrase, newPassphrase, sizeof(passphrase)-1);
    passphrase[sizeof(passphrase)-1]=0; // Ensure null term.
    DEBUG("Passphrase is %s\n", passphrase);
  } else {
    passphrase[0] = 0; 
    DEBUG("Passphrase is NULL\n");
  }
}

static void fiberDone(void *data) {
  release_fiber();
}

void BLELogService::backgroundFiber(void *data) {
    BLELogService *instance = (BLELogService*)data;
    while(true) {
          instance->periodicUpdate();
          fiber_sleep(1000);
    }
}


/** 
 * Constructor.
 * Create a representation of the Bluetooth Data Logger Service
 */
BLELogService::BLELogService() 
{
  time = 0;
  usage = 0;
  dataLength = 0;
  erase = false; 
  DEBUG("BLELog Serv starting\n");
  memset(givenPass, 0, sizeof(givenPass));
  memset(eraseRequest, 0, sizeof(eraseRequest));

// Base: accb4ce4-8a4b-11ed-a1eb-0242ac120002
  uint8_t baseUUID[] = { 0xac, 0xcb, 0x4c, 0xe4,  0x8a, 0x4b  ,0x11, 0xed,  
                         0xa1, 0xeb,  0x02, 0x42, 0xac, 0x12, 0x00, 0x02}; 
  RegisterBaseUUID(baseUUID);

  CreateService( bleLogService );

  // Create the data structures that represent each of our characteristics in Soft Device.
  CreateCharacteristic( mbls_cIdxSecurity, charUUID[ mbls_cIdxSecurity ],
                      (uint8_t *)&authorized,
                      sizeof(authorized), sizeof(authorized),
                      microbit_propREAD  | microbit_propNOTIFY); 

  CreateCharacteristic( mbls_cIdxPassphrase, charUUID[ mbls_cIdxPassphrase ],
                      (uint8_t *)givenPass,
                      sizeof(givenPass), sizeof(givenPass),
                      microbit_propWRITE_WITHOUT); 

  CreateCharacteristic( mbls_cIdxDataLength, charUUID[ mbls_cIdxDataLength ],
                      (uint8_t *)&dataLength,
                      sizeof(dataLength), sizeof(dataLength),
                      microbit_propREAD | microbit_propREADAUTH | microbit_propNOTIFY); 

  CreateCharacteristic( mbls_cIdxData, charUUID[ mbls_cIdxData ],
                      (uint8_t *)&dummyData,
                      sizeof(dummyData), sizeof(dummyData),
                      microbit_propWRITE_WITHOUT | microbit_propNOTIFY); 

  CreateCharacteristic( mbls_cIdxDataRequest, charUUID[ mbls_cIdxDataRequest ],
                      (uint8_t *)&dummyData,
                      sizeof(dummyData), sizeof(dummyData),
                      microbit_propWRITE_WITHOUT); 

  CreateCharacteristic( mbls_cIdxErase, charUUID[ mbls_cIdxErase ],
                      (uint8_t *)eraseRequest,
                      sizeof(eraseRequest), sizeof(eraseRequest),
                      microbit_propWRITE_WITHOUT ); 

  CreateCharacteristic( mbls_cIdxUsage, charUUID[ mbls_cIdxUsage ],
                      (uint8_t *)&usage,
                      sizeof(usage), sizeof(usage),
                      microbit_propREAD | microbit_propREADAUTH | microbit_propNOTIFY); 

  CreateCharacteristic( mbls_cIdxTime, charUUID[ mbls_cIdxTime ],
                      (uint8_t *)&time,
                      sizeof(time), sizeof(time),
                      microbit_propREAD | microbit_propREADAUTH); 


  pm_register(static_pm_events); 
  setAuthorized(false);
  advertise();
  // Set up fun Fiber for periodic checks
  create_fiber(BLELogService::backgroundFiber, this, fiberDone);
}


void BLELogService::periodicUpdate() {
//  DEBUG("Update...\n");
  if(authorized) {
    // Update values and do notifies if values change
    uint16_t oldUsage = usage;
    updateUsage();
    if(usage!=oldUsage) {
      notifyChrValue(mbls_cIdxUsage, (uint8_t*)&usage, sizeof(usage));  
    }
    uint32_t oldLength = dataLength;
    updateLength();
    if(dataLength != oldLength) {
      notifyChrValue(mbls_cIdxDataLength, (uint8_t*) &dataLength, sizeof(dataLength));  
    }
  }
  if(erase) {
    DEBUG("Erasing now\n");
    uBit.log.clear(false);
    erase = false;
  }
}

/**
  * Invoked when BLE connects.
  */
void BLELogService::onConnect( const microbit_ble_evt_t *p_ble_evt)
{
  DEBUG("BLE Log onConnect\n");
  // Reload Peer data 
  setAuthorized(strlen(passphrase)==0);
}

/**
  * Invoked when BLE disconnects.
  */
void BLELogService::onDisconnect( const microbit_ble_evt_t *p_ble_evt)
{
    DEBUG("BLE Log onDisconnect\n");
    setAuthorized(false);
}

void BLELogService::updateLength() {
    dataLength = uBit.log.getDataLength(DataFormat::CSV);
    setChrValue( mbls_cIdxDataLength, (uint8_t *)&dataLength, sizeof(dataLength));
}

void BLELogService::updateUsage() {
//    uint32_t dataStart =  sizeof(MicroBitLog::header) + CONFIG_MICROBIT_LOG_METADATA_SIZE + CONFIG_MICROBIT_LOG_JOURNAL_SIZE;
    uint32_t dataStart =  2048 + CONFIG_MICROBIT_LOG_METADATA_SIZE + CONFIG_MICROBIT_LOG_JOURNAL_SIZE;
    uint32_t totalSize =  uBit.flash.getFlashEnd() - sizeof(uint32_t) - dataStart;
    uint32_t inUse = uBit.log.getDataLength(DataFormat::CSV);

    // DEBUG("Usage comp. dataStart=%d, totalSize=%d, inUse=%d\n", dataStart, totalSize, inUse);
    usage = min(1000,(1000*inUse/totalSize));
    setChrValue( mbls_cIdxUsage, (uint8_t *)&usage, sizeof(usage));
}

void BLELogService::onDataRead( microbit_onDataRead_t *params) {
    DEBUG("BLE Log onDataRead\n");
    debugAttribute(params->handle);
    microbit_charattr_t type;
    int index = charHandleToIdx(params->handle, &type);

    // Update params.allow, data, and len
    switch(index) {
        case mbls_cIdxDataLength: {
          if(authorized) {
            updateLength();
            params->allow = true;
            params->data = (uint8_t *)&dataLength;
            params->length = sizeof(dataLength);
          } else {
            params->allow = false;
          }
        }
        break;

        case mbls_cIdxUsage: {
          if(authorized) {
            updateUsage();
            params->allow = true;
            params->data = (uint8_t *)&usage;
            params->length = sizeof(usage);
          } else {
            params->allow = false;
          }
        }
        break;

        case mbls_cIdxTime: {
          if(authorized) {
            time = system_timer_current_time_us();
            setChrValue( mbls_cIdxTime, (uint8_t *)&time, sizeof(time));
            params->allow = true;
            params->data = (uint8_t *)&time;
            params->length = sizeof(time);
          } else {
            params->allow = false;
          }

        }
        break;

        default:
        // Nothing
        break;
    }


      // int index = charHandleToIdx(params->handle, &type);
      // int offset = params->offset;
      // if(index == mbbs_cIdxReportMap && type == microbit_charattrVALUE) {
      //   params->data = &(reportMap[offset]);
      //   params->length = max(reportMapUsed-offset,0);  // Remaining data
      // }
}

/**
  * Callback. Invoked when any of our attributes are written via BLE.
  */
void BLELogService::onDataWritten( const microbit_ble_evt_write_t *params)
{
    DEBUG("BLE Log onDataWritten\n");
    debugAttribute(params->handle);

    microbit_charattr_t type;
    int index = charHandleToIdx(params->handle, &type);

    switch(index) {
      case mbls_cIdxSecurity: {
          if(type == microbit_charattrCCCD) {
                  bool status = params->len>0 && params->data[0] ? true : false;
                  // Update the value to do the notify. 
                  setAuthorized(authorized);
          }        
      }
      break;

      case mbls_cIdxPassphrase: {
          if(type == microbit_charattrVALUE) {
            // If already authorized or no passphrase or this passphrase matches.
            uint8_t passphraseLen = strlen(passphrase);
            if(authorized ||
               strlen(passphrase)==0 || 
               ( passphraseLen == params->len && 
                 strncmp(passphrase, (char *)params->data,passphraseLen)==0)) {
              DEBUG("Authorized\n");
              setAuthorized(true);
            } else {
              DEBUG("Invalid Passphrase %s\n", params->data[0]);
              DEBUG("Authorized = %d\n", authorized);
              // Leave authorized at current value, but notify / update
              setAuthorized(false);
            }
          }
      }
      break;

      case mbls_cIdxErase: {
          DEBUG("Erase request %d %s\n", params->len, params->data);
          if(type == microbit_charattrVALUE) {
            if(authorized && params->len==5 && strncmp("ERASE", (char*)params->data, 5)==0) {
              DEBUG("Erasing...");
              // Do the erase in the periodic update loop
              erase = true;
            } else {
              DEBUG("ERASE SKIPPED\n");
            }
            memset(eraseRequest, 0, sizeof(eraseRequest));
            setChrValue(mbls_cIdxErase, (uint8_t*)eraseRequest, sizeof(eraseRequest));
          }
      }
      break;

      default:
        DEBUG("Unhandled write");

    }


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


// // BSIEVER: Can delete all these...
// void BLELogService::onAuthorizeRequest(    const microbit_ble_evt_t *p_ble_evt) {
//     DEBUG("BLE Log onAuthorizeRequest\n");
//     MicroBitBLEService::onAuthorizeRequest(p_ble_evt);
// }
// void BLELogService::onAuthorizeRead(       const microbit_ble_evt_t *p_ble_evt) {
//     DEBUG("BLE Log onAuthorizeRead\n");
//     MicroBitBLEService::onAuthorizeRead(p_ble_evt);

// }
// void BLELogService::onAuthorizeWrite(      const microbit_ble_evt_t *p_ble_evt) {
//     DEBUG("BLE Log onAuthorizeWrite\n");
//     MicroBitBLEService::onAuthorizeWrite(p_ble_evt);
// }

void BLELogService::onConfirmation( const microbit_ble_evt_hvc_t *params) {
  DEBUG("BLE Log onConfirmation\n");

}

// void BLELogService::onHVC(                 const microbit_ble_evt_t *p_ble_evt) {
//   DEBUG("BLE Log onHVC\n");

// } 


bool BLELogService::onBleEvent(const microbit_ble_evt_t *p_ble_evt) {
    DEBUG("onBleEvent id = %d\n", p_ble_evt->header.evt_id);
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
    //  "uBit [XXXXX]"   
    int len = sprintf(gapName, "uBit [%s]", microbit_friendly_name());
    ble_gap_conn_sec_mode_t permissions;
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS( &permissions);
    MICROBIT_BLE_ECHK( sd_ble_gap_device_name_set( &permissions, (uint8_t *)gapName, len) );
}


void BLELogService::setAuthorized(bool nowAuthorized) {
  DEBUG("setAuthorized(%d)\n", nowAuthorized);
  authorized = nowAuthorized ? 0xFF : 0x00;
  setChrValue( mbls_cIdxSecurity, &authorized, sizeof(authorized));
  notifyChrValue(mbls_cIdxSecurity, &authorized, sizeof(authorized));  
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
 
        // uuid.type = BLE_UUID_TYPE_VENDOR_BEGIN ;
        // uuid.uuid = 0x182; // 1812 is HID 
        // m_advdata.uuids_complete.uuid_cnt = 1;
        // m_advdata.uuids_complete.p_uuids = &uuid;
        // m_advdata.include_appearance = true;
        // Name needed to be identified by Android
        m_advdata.name_type = BLE_ADVDATA_FULL_NAME;
        
        // Appearance isn't strictly needed for detection 
        // sd_ble_gap_appearance_set(BLE_APPEARANCE_GENERIC_HID );

        // The flags below ensure "pairing mode" so it shows up in Android
        // m_advdata.flags = BLE_GAP_ADV_FLAG_BR_EDR_NOT_SUPPORTED | BLE_GAP_ADV_FLAG_LE_GENERAL_DISC_MODE;

        ble_gap_adv_params_t    gap_adv_params;
        memset( &gap_adv_params, 0, sizeof( gap_adv_params));
        gap_adv_params.properties.type  = BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED;
        gap_adv_params.interval         = ( 1000 * MICROBIT_BLE_ADVERTISING_INTERVAL/* interval_ms */) / 625;  // 625 us units
        if ( gap_adv_params.interval < BLE_GAP_ADV_INTERVAL_MIN) gap_adv_params.interval = BLE_GAP_ADV_INTERVAL_MIN;
        if ( gap_adv_params.interval > BLE_GAP_ADV_INTERVAL_MAX) gap_adv_params.interval = BLE_GAP_ADV_INTERVAL_MAX;
        gap_adv_params.duration         = MICROBIT_BLE_ADVERTISING_TIMEOUT /* timeout_seconds */ * 100;              //10 ms units
        gap_adv_params.filter_policy    = BLE_GAP_ADV_FP_ANY;
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
      if(index<0 || index>=mbbs_cIdxCOUNT) index = mbbs_cIdxCOUNT;

      char const *charNames[] = {"Security", "Passphrase", "Data Length", "Data", "Erase", "Usage", "Time"};
      if(index<mbbs_cIdxCOUNT) {
        DEBUG("     %s %s\n", charNames[index], typeName);
      }
#endif
}
#endif