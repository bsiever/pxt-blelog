#ifndef BLELOG_SERVICE_H
#define BLELOG_SERVICE_H

#include "MicroBitConfig.h"

#if CONFIG_ENABLED(DEVICE_BLE)

#include "MicroBitBLEManager.h"
#include "MicroBitBLEService.h"
#include "EventModel.h"
#include "debug.h"

#include"peer_manager.h"




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

    // Override notification process to enforce minimum time between events. 
    bool notifyChrValue( int idx, const uint8_t *data, uint16_t length);


    // void onAuthorizeRequest(    const microbit_ble_evt_t *p_ble_evt);
    // void onAuthorizeRead(       const microbit_ble_evt_t *p_ble_evt);
    // void onAuthorizeWrite(      const microbit_ble_evt_t *p_ble_evt);
    void onConfirmation( const microbit_ble_evt_hvc_t *params);
    // void onHVC(                 const microbit_ble_evt_t *p_ble_evt);


    // Peer Manager Events (re-enable CCCDs)
    void pm_events( const pm_evt_t* p_event);

    // Static instance variables were created to facilitate multiple HID Services
    // (Now a singleton is used and they could be converted to instance variables)

    // Peer Manager Events (re-enable CCCDs)
    static void static_pm_events( const pm_evt_t* p_event);

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
    int dummyData;

    uint32_t dataLength;
    uint64_t time;
    uint16_t usage;

    void setName();
    void advertise();
    void setAuthorized(bool nowAuthorized);
    void periodicUpdate();

    void updateUsage();
    void updateLength();

    // Debugging: Print the attribute / info.
    void debugAttribute(int index); 

    static void backgroundFiber(void *data);

};
#endif
#endif