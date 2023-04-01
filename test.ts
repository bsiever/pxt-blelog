serial.writeLine("starting...")

let k = 0
input.onButtonPressed(Button.A, function () {
    basic.showIcon(IconNames.Heart)

    for(let i = 0; i < 10; i++) {
        k = k + 1
        datalogger.log(
            datalogger.createCV("x", 100+k),
            datalogger.createCV("y", 200+k),
            datalogger.createCV("z", 300+k))
    }
    basic.clearScreen()
})


input.onButtonPressed(Button.B, function () {
    game.addScore(1)
    datalogger.log(
    datalogger.createCV("x", input.acceleration(Dimension.X)),
    datalogger.createCV("y", input.acceleration(Dimension.Y))
    )
})


input.onButtonPressed(Button.AB, function () {
    basic.showIcon(IconNames.No)
    datalogger.deleteLog(datalogger.DeleteType.Fast)
    basic.clearScreen()
})

basic.showIcon(IconNames.Happy)
blelog.startBLELogService()
datalogger.includeTimestamp(FlashLogTimeStampFormat.Seconds)

// basic.forever(function () {
//     game.addScore(1)

//     // for(let i = 0; i < 10; i++) {

//     // datalogger.log(
//     //     datalogger.createCV("x", input.acceleration(Dimension.X)),
//     //     datalogger.createCV("y", input.acceleration(Dimension.Y)),
//     //     datalogger.createCV("abc", 12.34),
//     //     datalogger.createCV("def", 45.67),
//     //     datalogger.createCV("g", 123.456),
//     //     datalogger.createCV("h", 12312.12312123),
//     //     datalogger.createCV("i", 123.65434)
//     //     )
//     //     datalogger.log(
//     //     datalogger.createCV("x", input.acceleration(Dimension.X)),
//     //     datalogger.createCV("y", input.acceleration(Dimension.Y))
//     //     )
//     //     basic.pause(10)
//     // }
//     basic.pause(500)
// })





// blelog.startBLELogService()
// datalogger.setColumnTitles(
//     "x",
//     "y"//,
//     //"z"
//     )
    
// let x = 0

// input.onButtonPressed(Button.A, function () {
//     serial.writeLine("Logging X,Y,Z")
//     x = x + 1
//     if(x%5==0) {
//         serial.writeLine("Doing a Z...")
//         datalogger.log(
//             datalogger.createCV("x", input.acceleration(Dimension.X)),
//             datalogger.createCV("y", input.acceleration(Dimension.Y)),
//             datalogger.createCV("z", input.acceleration(Dimension.Z))
//             )
        
//     } else {
//         datalogger.log(
//             datalogger.createCV("x", input.acceleration(Dimension.X)),
//             datalogger.createCV("y", input.acceleration(Dimension.Y))
//             )
        
//     }
// })
// input.onButtonPressed(Button.AB, function () {
//     serial.writeLine("Clearing Log Full")
//     datalogger.deleteLog()
// })

// input.onButtonPressed(Button.B, function() {
//     blelog.dumpBLELog()
// })