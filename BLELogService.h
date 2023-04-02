#ifndef BLELOG_SERVICE_H
#define BLELOG_SERVICE_H

#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "MicroBitBLEManager.h"
#include "MicroBitBLEService.h"
#include "EventModel.h"
#include "debug.h"


/**
  * Class definition for a MicroBit BLE HID Service.
  */
class BLELogService : public MicroBitBLEService
{
  public:
    static BLELogService *getInstance();
    void setPassphrase(const char* newPassphrase);

  private:
    static BLELogService *service; // Singleton

    /**
     * Constructor.
     * Create a representation of the Bluetooth SIG HID Service
     * @param _ble The instance of a BLE device that we're running on.
     */
    BLELogService();

    /**
      * Invoked when BLE connects.
      */
    void onConnect(const microbit_ble_evt_t *p_ble_evt);

    /**
      * Invoked when BLE disconnects.
      */
    void onDisconnect(const microbit_ble_evt_t *p_ble_evt);

    /**
      * Callback. Invoked when any of our attributes are written via BLE.
      */
    void onDataWritten(const microbit_ble_evt_write_t *params);

    /**
     * Callback. Invoked when any of our attributes are read via BLE.
     */
    void onDataRead(microbit_onDataRead_t *params);

    // BLE Events...Let's monitor 'em all. 
    bool onBleEvent(const microbit_ble_evt_t *p_ble_evt);

    // Index for each characteristic in arrays of handles and UUIDs
    typedef enum mbbs_cIdx
    {  
      mbls_cIdxSecurity,     // Read/Write/Notify 
      mbls_cIdxPassphrase,   // Write
      mbls_cIdxDataLength,   // Read/Notify 
      mbls_cIdxData,         // Notify 
      mbls_cIdxDataRequest,  // Write
      mbls_cIdxErase,        // Write (request)
      mbls_cIdxUsage,        // Read/Notify
      mbls_cIdxTime,         // Read 
      mbls_cIdxDataRead,     // Data Read Request
      // ?? FULL???
      mbbs_cIdxCOUNT
    } mbbs_cIdx;

    // Service UUID
    static const uint16_t bleLogService;

    // UUIDs for our service and characteristics
    static const uint16_t charUUID[mbbs_cIdxCOUNT];
    
    // Data for each characteristic when they are held by Soft Device.
    MicroBitBLEChar      chars[mbbs_cIdxCOUNT];

    int              characteristicCount()          { return mbbs_cIdxCOUNT; };
    MicroBitBLEChar *characteristicPtr(int idx)     { return &chars[idx]; };

    char gapName[14];
    char passphrase[21];
    uint8_t authorized;
    char givenPass[20]; // Buffer that represents the tried password / value
    uint8_t readRequest[8]; // 32-bits for index, 32-bits for size

    uint8_t readDataBuffer[204];  // Buffer for reading directly
    uint32_t readDataStart; 
 
    uint32_t readStart;
    uint32_t readLength;
    bool readInProgress;
    bool readUpdate;

    uint8_t dataBuffer[20];
    char eraseRequest[6];

    uint32_t dataLength;
    uint64_t time;
    uint16_t usage;
    bool erase = false;

    void setName();
    void advertise();
    void setAuthorized(bool nowAuthorized);
    void periodicUpdate();
    void resetConnection();
    void updateUsage();
    void updateLength();

    // Debugging: Print the attribute / info.
    void debugAttribute(int index); 

    static void backgroundFiber(void *data);
    static void logRetrieve(void *data);

};
#endif
#endif