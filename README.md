# BLE Log Service

```package
pins=github:bsiever/microbit-pxt-blelog
```

Extension for Bluetooth retrieval and modification of the micro:bit v2 data logger's log.

# Enabling the service

```sig
blelog.startBLELogService(passphrase?: string) : void
```

Start the BLE Log service (with optional passphrase).  This should be included in the start blocks.

If a passphrase is empty, no passphrase is required for other devices to retrieve data.  If a passphrase is provided, devices must provide the passphrase to retrieve the data over Bluetooth. (The passphrase does not impact access to the data log file on the micro:bit).  If a passphrase is longer than 20 characters, only the first 20 characters are used.

# Example

```block
blelog.startBLELogService()
```


# Bluetooth Service Overview 


## Service UUID

accb4ce4-8a4b-11ed-a1eb-0242ac120002
## Characteristics

| UUID | Name |  Properties |
|:----:|------|-------------|
|accb4f64-8a4b-11ed-a1eb-0242ac120002 | Security  | Read, Notify|
|accb50a4-8a4b-11ed-a1eb-0242ac120002 | Passphrase |Write |
|accb520c-8a4b-11ed-a1eb-0242ac120002 | Data Length       | Read, Notify |
|accb53ba-8a4b-11ed-a1eb-0242ac120002 | Data       | Notify |
|accb552c-8a4b-11ed-a1eb-0242ac120002 | Data Request       |Write|
|accb5946-8a4b-11ed-a1eb-0242ac120002 | Erase        | Write |
|accb5be4-8a4b-11ed-a1eb-0242ac120002 | Usage        | Read, Notify |
|accb5dd8-8a4b-11ed-a1eb-0242ac120002 | Time        | Read |

## Security

Read/notify will indicate a true (0x00) if the interactions are authorized and false (0x00) otherwise. (Automatically sends value when notifies are enabled)

If false, a correct passphrase is required to enable access to other data. 

## Passphrase

Characteristic to write passphrase (if needed).  If a passphrase is required and the correct one is provided, other characteristics will be available.  Access will be granted until disconnection.

The passphrase can be provided in the on-service block.  It is not required (no passphrase needed) by default.

Passphrases are no more than 20 characters.
## Data Length

The length of the data in bytes (`uint32_t`).

## Data

Notifiations will be 0-20 bytes.  An empty (length 0) indicates the end of transmitted data.  If non-empty, the first 4 bytes are the index into the message and the remaining bytes are the data starting at that index (up to 16 bytes of data).

## Data Request

A `uint32_t` indicating the index into the log to retrieve.  A `uint32_t` indicating the length (in bytes) to retrieve.  Data is sent via the Data characteristics.
## Erase

Write the full word "ERASE" to erase the log file. (Only erases if authorized)

## Usage

Percentage of log currently in use (`uint16_t` from [0-1000], where 1000=100.0). 

## Time

The current micro:bit clock (milliseconds) as a `uint64_t`.

<script src="https://makecode.com/gh-pages-embed.js"></script>
<script>makeCodeRender("{{ site.makecode.home_url }}", "{{ site.github.owner_name }}/{{ site.github.repository_name }}");</script>
