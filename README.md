mbed CMSIS-DAP
==============

Turn a mbed-enabled development board with USB into a CMSIS-DAP debug probe.

mbed CMSIS-DAP is based on mbed os 5. If you prefer to using mbed os 2, you can try [Simple-CMSIS-DAP](https://os.mbed.com/users/va009039/code/Simple-CMSIS-DAP/)


## Build
```
git clone https://github.com/xiongyihui/mbed-CMSIS-DAP.git
cd mbed-CMSIS-DAP
pip install mbed-cli
mbed deploy
mbed compile -t gcc_arm -m nrf52840_dk
```

## Hardware
Devices supported by mbed os 5 usb stack should be supported, such as nRF52840, LPC17xx and STM32.

Tested devices:

+   nRF52840

    + [nRF52840-MDK USB Dongle](https://wiki.makerdiary.com/nrf52840-mdk-usb-dongle/)
    + [nRF52840-MDK ](https://wiki.makerdiary.com/nrf52840-mdk/)
    + [Pitaya Go](https://wiki.makerdiary.com/pitaya-go/)

