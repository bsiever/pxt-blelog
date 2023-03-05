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
  0x5dd8,   // mbls_cIdxTime          // Read
  0x5f72    // mbls_cIdxDataRead      // Read & Write
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
accb5f72-8a4b-11ed-a1eb-0242ac120002

Spares:
accb613e-8a4b-11ed-a1eb-0242ac120002
accb6332-8a4b-11ed-a1eb-0242ac120002

*/


BLELogService *BLELogService::service = NULL; // Singleton reference to the service

/**
 */
BLELogService *BLELogService::getInstance()
{
    if (service == NULL) {
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
          fiber_sleep(100);
    }
}

void BLELogService::resetConnection() {
  DEBUG("Resetting Connection\n");
  erase = false; 
  memset(readDataBuffer, 0, sizeof(readDataBuffer));
  readDataStart = 0;

  memset(readRequest, 0, sizeof(readRequest));
  dataLength = 0;
  readStart = 0;
  readLength = 0;
  if(readInProgress)
    readUpdate = true;  // Stop the read process
  readInProgress = false;
  memset(givenPass, 0, sizeof(givenPass));
  memset(eraseRequest, 0, sizeof(eraseRequest));
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
  resetConnection();

  DEBUG("BLELog Serv starting\n");

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
                      (uint8_t *)&dataBuffer,
                      0, sizeof(dataBuffer),
                      microbit_propNOTIFY); 

  CreateCharacteristic( mbls_cIdxDataRequest, charUUID[ mbls_cIdxDataRequest ],
                      (uint8_t *)&readRequest,
                      sizeof(readRequest), sizeof(readRequest),
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

  CreateCharacteristic( mbls_cIdxDataRead, charUUID[ mbls_cIdxDataRead ],
                      (uint8_t *)&readDataBuffer,
                      0, sizeof(readDataBuffer),
                      microbit_propREAD | microbit_propREADAUTH | microbit_propWRITE_WITHOUT); 

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

    // Check "TODOs"
    if(erase) {
      uBit.log.clear(false);
      erase = false;
    }

    if(readInProgress) {
      // Sanity check
      if(readStart<dataLength) {
        uint32_t endIndex = min(dataLength, readStart+readLength);
        // Send index & <= 16 bytes of data
        while(readStart < endIndex) {
          if(readUpdate) {
            // Exit the loop and start process over with new request
            readUpdate = false;
            return;
          }

          uint32_t amount = min(16, endIndex-readStart);
          memcpy(dataBuffer, &readStart, sizeof(readStart));
          DEBUG("read %d\n", readStart);
          uBit.log.readData(dataBuffer+4, readStart, amount, DataFormat::CSV, dataLength);
          // Notify / Update
          DEBUG("N\n");
          notifyChrValue(mbls_cIdxData, dataBuffer, amount+4);  
          // Delay to ensure no overrun.
          DEBUG("S\n");
          fiber_sleep(25);  // Connection event is 10-20ms
          readStart+=amount;
        }
      }
      // Send final message / Sentinel (0 length message)
      memset(dataBuffer, 0, sizeof(dataBuffer));
      notifyChrValue(mbls_cIdxData, dataBuffer, 0);  
      readInProgress = false;
      readStart = 0;
      readLength = 0;
    }
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
  resetConnection();
}

/**
  * Invoked when BLE disconnects.
  */
void BLELogService::onDisconnect( const microbit_ble_evt_t *p_ble_evt)
{
    DEBUG("BLE Log onDisconnect\n");
    setAuthorized(false);
    resetConnection();
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

        case mbls_cIdxDataRead: {
          if(authorized) {
            // Get data 
            int offset = params->offset;
            int len = params->length;
            DEBUG("Reading Data... offset %d len %d\n",offset, len);
//            setChrValue(mbls_cIdxDataRead, (uint8_t *)&readDataBuffer, readDataBufferLength);
            params->allow = true;
            params->data = &(readDataBuffer[offset]);
            params->length = max(min(sizeof(readDataBuffer)-offset, dataLength + 4 - offset - readDataStart),0);

            // TODO:  If this is the last allowable read of this 200-byte unit, auto-advance (if there's more)



          } else {
            params->allow = false;
          }

        }
        break;


        default:
        // Nothing
        break;
    }
}


void BLELogService::logRetrieve(void *data) {
  BLELogService *instance = (BLELogService*)data;
  DEBUG("Reading log\n");
//  instance->updateLength();
  uint32_t start = min(instance->readDataStart, instance->dataLength-1);
  uint32_t len = min(sizeof(instance->readDataBuffer)-4, instance->dataLength-start);
  DEBUG("Reading from %d (%d)\n", start, len);
  uBit.log.readData(instance->readDataBuffer+4, start, len, DataFormat::CSV, instance->dataLength);
  DEBUG("Read log complete\n");
  memcpy(instance->readDataBuffer, &instance->readDataStart, sizeof(instance->readDataStart));
  DEBUG("Size updated too\n");
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

      case mbls_cIdxUsage: {
          if(type == microbit_charattrCCCD) {
                  bool status = params->len>0 && params->data[0] ? true : false;
                  // Provoke a notify
                  usage = -1;
          }        
      }
      break;

     case mbls_cIdxDataLength: {
          if(type == microbit_charattrCCCD) {
                  bool status = params->len>0 && params->data[0] ? true : false;
                  // Provoke a notify
                  dataLength = -1;
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

      case mbls_cIdxDataRequest: {
          if(authorized && type == microbit_charattrVALUE && params->len==8) {
            if(readInProgress) {
              DEBUG("Abandoning read in progress\n");
              readUpdate = true;
            } else {
              readUpdate = false;
            }
            memcpy(&readStart, params->data, 4);
            memcpy(&readLength, params->data+4, 4);
            DEBUG("Reading at index %d of len %d\n", readStart, readLength);
            // If there's already a read in progress, abandon it.
            readInProgress = true;
            DEBUG("Read in progress..%d\n", readInProgress);
          }
      }
      break;
 
      case mbls_cIdxDataRead: {
          if(type == microbit_charattrVALUE) {
            // If already authorized or no passphrase or this passphrase matches.
            if(authorized && params->len==4) {
              memcpy(&readDataStart, params->data, 4);
              DEBUG("Read request at index %d", start);
              // Do fiber thing...
              create_fiber(BLELogService::logRetrieve, this, fiberDone);
            }
          }
      }
      break;


      default:
        DEBUG("Unhandled write");

    }

}

bool BLELogService::onBleEvent(const microbit_ble_evt_t *p_ble_evt) {
    DEBUG("onBleEvent id = %d\n", p_ble_evt->header.evt_id);
    // Let usual process handle it. 
    return MicroBitBLEService::onBleEvent(p_ble_evt);
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
 
        // Name needed to be identified by Android
        m_advdata.name_type = BLE_ADVDATA_FULL_NAME;
        
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

      char const *charNames[] = {"Security", "Passphrase", "Data Length", "Data", "Data Req", "Erase", "Usage", "Time"};
      if(index<mbbs_cIdxCOUNT) {
        DEBUG("     %s %s\n", charNames[index], typeName);
      }
#endif
}
#endif