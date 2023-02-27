

//% color=#0000FF 
//% icon="\uf074"
//% block="Datalogger Bluetooth Service"
//% group="micro:bit (V2)"
namespace blelog {
    //% block="start Bluetooth Log Service || with passphase $passphrase"
    //% passphrase.defl=""
    //% shim=blelog::startBLELogService
    //% expandableArgumentMode="toggle"
    //% group="micro:bit (V2)"
    export function startBLELogService(passphrase: string = null) : void {
        // Per https://github.com/microsoft/pxt-microbit/issues/4292
        0;
    }

    //% blelog="dumpBLELog" block="dump ble log"
    //% shim=blelog::dumpBLELog
    //% group="micro:bit (V2)"
    export function dumpBLELog() : void {
        // Per https://github.com/microsoft/pxt-microbit/issues/4292
        0;
    }


}
