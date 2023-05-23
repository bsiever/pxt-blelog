
## Misc. Data

| Fiber sleep | Throughput (bytes/sec) |
| 20 | 909 | 
| 10 | 1449 |
| 7 | 1824 |
| 5 | 1977 |
| 4 | 775 |
| 2 | 749 |
| 1 | 850 |

Initially I was using 20.  I've updated it to 5, which nearly doubles performance of the initial read.

Redesign using ~500 byte long reads:  Throughput was about 600 bytes / second.  Streaming approach was about 3x better.


# Approval TODOs

## TODO

The category has no icon.

There is a commit after the last tag.

Does pxt.json need to configure open security? -> Yes.  That's the intended approach.  

The javascript tooltip on namespace "blelog" doesn't seem appropriate, and startBLELogService doesn't have a tooltip. -> Done, but need to test / confirm

Please see https://makecode.com/extensions/getting-started#:~:text=It%20is%20recommended%20that%20for%20an%20extension%20called%20banana%20the%20repository%20should%20be%20called%20pxt%2Dbanana.


## DONE

​
The category name "Data Logger Bluetooth" is very long, and makes the whole panel wider.  -> "Log Bluetooth"

The block has a misspelling: "passphase". -> Fixed

Does pxt.json need to enable a large DMESG buffer to serial? -> Removed

How did you choose the service and characteristic UUIDs? -> Explained

I think "pins=github:bsiever/microbit-pxt-blelog" should be removed from the README.
https://github.com/microsoft/pxt-microbit/issues/4832#issuecomment-1554747300 -> Removed

The README MIsc. Data section doesn't seem helpful. -> Refactored
 






<!-- ```package
pins=github:bsiever/microbit-pxt-blelog
``` -->