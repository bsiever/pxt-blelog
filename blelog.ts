



// Same icon as datalogger block? (f0ce), but blue?  
//  line-chart: f201? paper-plane: f1d8? share-square: f045

/**
 * Bluetooth Access to Data Logger's Data
 */
//% color=#0000FF 
//% icon="\uf045"       
//% block="Log Bluetooth"
//% group="micro:bit (V2)"
namespace blelog {
    /**
     * Start the Bluetooth Access to the Data Logger's Data
     * @param {string} passphrase optional pass phrase to limit access to the data
     */
    //% block="bluetooth data logger service || with passphrase $passphrase"
    //% passphrase.defl=""
    //% shim=blelog::startBLELogService
    //% expandableArgumentMode="toggle"
    //% group="micro:bit (V2)"
    export function startBLELogService(passphrase?: string) : void {
        // Per https://github.com/microsoft/pxt-microbit/issues/4292
        0;
    }

    // //% blelog="dumpBLELog" block="dump ble log"
    // //% shim=blelog::dumpBLELog
    // //% group="micro:bit (V2)"
    // export function dumpBLELog() : void {
    //     // Per https://github.com/microsoft/pxt-microbit/issues/4292
    //     0;
    // }


}
