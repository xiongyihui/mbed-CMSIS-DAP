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

## For nRF52840 MDK USB Dongle
1.  Download pre-built firmware [CMSIS-DAP for nRF52840 MDK USB Dongle](cmsis-dap-for-nrf52840-mdk-usb-dongle-0x1000.hex)
2.  Use nRF Connect Desktop or nRF Util to upgrade the firmware via its bootloader

    ```
    pip3 install nrfutil
    nrfutil pkg generate --hw-version 52 --application-version 1 --sd-req 0x00 --application cmsis-dap-for-nrf52840-mdk-usb-dongle-0x1000.hex cmsis-dap.zip
    nrfutil dfu usb-serial -pkg cmsis-dap.zip -p /dev/ttyACM0
    ```

3. Connect the USB Dongle with a ARM microcontroller through SWD interface

    + nRESET - p0.6
    + SWCLK - p0.7
    + SWDIO - p0.8

