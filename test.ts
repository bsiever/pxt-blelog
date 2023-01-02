serial.writeLine("starting...")
//bluetooth.startUartService()
blelog.startBLELogService()
//blelog.startBLELogService("Pass")
let x = 0
input.onButtonPressed(Button.A, function () {
    serial.writeLine("Logging X,Y,Z")
    x = x + 1
    if(x%5==0) {
        serial.writeLine("Doing a Z...")
        datalogger.log(
            datalogger.createCV("x", input.acceleration(Dimension.X)),
            datalogger.createCV("y", input.acceleration(Dimension.Y)),
            datalogger.createCV("z", input.acceleration(Dimension.Z))
            )
        
    } else {
        datalogger.log(
            datalogger.createCV("x", input.acceleration(Dimension.X)),
            datalogger.createCV("y", input.acceleration(Dimension.Y))
            )
        
    }
})
input.onButtonPressed(Button.AB, function () {
    serial.writeLine("Clearing Log Full")

//    datalogger.deleteLog(datalogger.DeleteType.Full)
    datalogger.deleteLog()
})
datalogger.includeTimestamp(FlashLogTimeStampFormat.Hours)

datalogger.setColumnTitles(
"x",
"y"//,
//"z"
)

input.onButtonPressed(Button.B, function() {
    blelog.dumpBLELog()
})