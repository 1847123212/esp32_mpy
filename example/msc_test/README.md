# USB Flash Disk

**The demo is only used for function preview , only tested on Windows 10 now, don't be surprised if you find bug**



> Internal CommitID 43bc969ab9f41190b

## Hardware

- Board：ESP32-S2-Saola-1_v1.2
- MCU：ESP32-S2 
- Flash：4MB NOR Flash
- Hardware Connection： 

GPIO19/20 to D-/D+ respectively for an ESP32-S2 board

![](./_static/usb-board-connection.png)

### Build The Example

1. for general uart download
   1. input `idf.py build` to build the example
   2. enter or autoswitch to downloader mode
   3. input `idf.py flash` to download the bin to esp32s2
2. only have usb port?
   1. input `idf.py dfu` to build the example
   2. pulldown `boot` io to enter `USB DFU` downloader mode
   3. input `idf.py dfu-flash` to download the bin to esp32s2


## ESP32S2 USB Overview

### ESP32-S2 USB Hardware Description :

[ESP32-S2 family](https://www.espressif.com/sites/default/files/documentation/esp32-s2_datasheet_en.pdf) features a full-speed USB OTG interface which is compliant with the USB 1.1 specification. It has the following features:

* software-configurable endpoint settings and suspend/resume
* support for dynamic FIFO sizing
* support for session request protocol (SRP) and host negotiation protocol (HNP)
* a full-speed USB PHY integrated in the chip

### ESP32-S2 USB Software Description :

[TinyUSB](https://github.com/hathach/tinyusb) is integrating with [ESP-IDF](https://github.com/espressif/esp-idf) to provide USB features of the framework. TinyUSB is an open-source cross-platform USB Host/Device stack for embedded system, designed to be memory-safe with no dynamic allocation and thread-safe with all interrupt events are deferred then handled in the non-ISR task function.

#### Device Stack(Supported)

Supports multiple device configurations by dynamically changing usb descriptors. Low power functions such like suspend, resume, and remote wakeup. Following device classes are supported:

- Communication Class (CDC)
- Human Interface Device (HID): Generic (In & Out), Keyboard, Mouse, Gamepad etc ...
- Mass Storage Class (MSC): with multiple LUNs
- Musical Instrument Digital Interface (MIDI)
- Network with RNDIS, CDC-ECM (work in progress)
- USB Test and Measurement Class (USBTMC)
- Vendor-specific class support with generic In & Out endpoints. Can be used with MS OS 2.0 compatible descriptor to load winUSB driver without INF file.
- [WebUSB](https://github.com/WICG/webusb) with vendor-specific class

#### Host Stack([will support soon](https://esp32.com/viewtopic.php?f=10&t=14532))

**Most active development is on the Device stack. The Host stack is under rework**

- Human Interface Device (HID): Keyboard, Mouse, Generic
- Mass Storage Class (MSC)
- Hub currently only supports 1 level of hub (due to my laziness)
