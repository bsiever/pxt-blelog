

//% color=#0000FF 
//% icon="\uf074"
//% block="Datalogger Bluetooth"
//% group="micro:bit (V2)"
namespace blelog {
    //% blelog="setI2CPins" block="set i2c data to %sdaPin and clock to %sclPin|"
    //% shim=blelog::setI2CPins
    //% sdaPin.defl=DigitalPin.P1 sclPin.defl=DigitalPin.P2
    //% group="micro:bit (V2)"
    export function setI2CPins(sdaPin : DigitalPin,  sclPin : DigitalPin) : void {
        // Per https://github.com/microsoft/pxt-microbit/issues/4292
        0;
    }
}
