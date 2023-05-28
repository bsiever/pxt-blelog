/*
* Bill Siever
* 2022-10-16 Initial Version
*
* Development environment specifics:
* Written in Microsoft PXT
*
* This code is released under the [MIT License](http://opensource.org/licenses/MIT).
* Please review the LICENSE.md file included with this example. If you have any questions 
* or concerns with licensing, please contact techsupport@sparkfun.com.
* Distributed as-is; no warranty is given.
*/

#include "pxt.h"
#include "MicroBit.h"
#include "BLELogService.h"
#include "MicroBitLog.h"  // TimeStampFormat

//#include "debug.h"
using namespace pxt;

namespace blelog { 

    //% group="micro:bit (V2)"
    //%
    void startBLELogService(void* passphrase) {
#if MICROBIT_CODAL
        // Only start service once
        static bool started = false;
        if(started) 
            return;
        started = true;

        // V2
        //DEBUG("Pass is %X\n", passphrase);
        const char *cpPassphrase = NULL;
        if(passphrase) {
            cpPassphrase = ((String)passphrase)->getUTF8Data();
            //DEBUG("cpPassphrase is %s\n", cpPassphrase);
        } else {
            //DEBUG("No Passphrase\n");
        }
        //DEBUG("Running Service\n");

        // Set timestamp format to seconds
        uBit.log.setTimeStamp(TimeStampFormat::Seconds);

        // On reboot if the log isn't empty, add a timestamp-only with 0. 
        if(uBit.log.getDataLength(DataFormat::CSV) > 0) {
            uBit.log.logString("0\n");
        }
        BLELogService::getInstance()->setPassphrase(cpPassphrase);
#endif
    }

// //    // %
//     void dumpBLELog() {
// #if MICROBIT_CODAL
//         const int BUF_SZ = 20;
//         char bigBuff[5000];
//         // V2
//         //DEBUG("Dumping Log (C++)...\n");
//         // First read is header
//         // Index is by 1
//         // index = 0;
//         // bool done = false;
//         // do {
//         //     uBit.log.readData(buffer, index, BUF_SZ, DataFormat::CSV, BUF_SZ);
//         //     if(buffer[0] < 10)
//         //         //DEBUG("%c (0%x)\n",buffer[0], buffer[0]);
//         //     else
//         //         //DEBUG("%c (%x)\n",buffer[0], buffer[0]);
//         //     done = buffer[0] == 0 || buffer[1] == 0xFF;
//         //     index++;
//         // } while(index<100 && !done);
//         // //DEBUG("\n"); 

//         uint32_t totalLen = uBit.log.getDataLength(DataFormat::CSV);
//         uint32_t remaining = totalLen;
//         //DEBUG("Log Length 2 = %d\n",remaining);
//         uint32_t index = 0;

//         bigBuff[0] = 0;
//         while(remaining>0) {
//             uint32_t readSz = min(BUF_SZ, remaining);
//             uBit.log.readData(bigBuff+index, index, readSz, DataFormat::CSV, totalLen);
//             //DEBUG("Data = ");
//             for(uint32_t i=0;i<readSz;i++) {
//                 char c = bigBuff[index+i];
//                 if(c<10) {
//                     //DEBUG("0%x", c);
//                 } else {
//                     //DEBUG("%x", c);
//                 }
//             }
//             //DEBUG("\n");
//             //DEBUG("Data Char = %s\n",bigBuff+index);
//             index += readSz;
//             //DEBUG("Index = %d\n",index);
//             remaining -= readSz;
//         }
//         bigBuff[index] = 0; 
//         //DEBUG("bigBuf = %s\n",bigBuff);

// #endif
//     }


}