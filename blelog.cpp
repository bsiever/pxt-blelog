/**
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
#include "debug.h"
using namespace pxt;

namespace blelog { 

    //%
    void startBLELogService() {
#if MICROBIT_CODAL
        // V2
        DEBUG("Running Service\n");
        BLELogService::getInstance();
#endif
    }
}