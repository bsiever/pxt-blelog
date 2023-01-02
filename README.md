# BLE Log Service

```package
pins=github:bsiever/microbit-pxt-blelog
```

Extension for Bluetooth Retrieval of the micro:bit v2 data logger's log. 

Service overview

Password : Write:  Must be provided to authorize service
Clock : Read:  Current micro:bit clock (used to time-date stamp items...Assumes no rollover)
Reset: write/control point:  Reset the log?
Data:  Read the data??? Stream as long reads? Or as simple data via notifivations?  Include index into records ?()
Data 2:  Custom read/write to correct any omissions?
Data done : Indicate?  to indicate done with data stream???






# Setting the pins

<!-- ```sig
pins.setI2CPins(sda : DigitalPin,  scl : DigitalPin) : void
``` -->

Set the Data and Clock pins.

# Acknowledgements 

<script src="https://makecode.com/gh-pages-embed.js"></script>
<script>makeCodeRender("{{ site.makecode.home_url }}", "{{ site.github.owner_name }}/{{ site.github.repository_name }}");</script>
