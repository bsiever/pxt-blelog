/**
 * Bluetooth Access to Data Logger's Data
 */
//% color=#0000FF 
//% icon="\uf14D"       
//% block="Log Bluetooth"
namespace blelog {
    /**
     * Start the Bluetooth Access to the Data Logger's Data
     * @param passphrase optional pass phrase to limit access to the data
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
}
