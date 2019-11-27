#include <events/mbed_events.h>

#include <mbed.h>

#include "USBHID.h"
#include "BaseDAP.h"

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);



SWD swd(P0_25, P0_26, P0_27); // SWDIO,SWCLK,nRESET
DigitalOut connected(P0_22);
DigitalOut running(P0_23);


class HIDDAP: public USBHID {
public:
    HIDDAP(): USBHID() {}

    const uint8_t *string_iproduct_desc()
    {
        static const uint8_t stringIproductDescriptor[] = {
            30,                                                       //bLength
            STRING_DESCRIPTOR,                                          //bDescriptorType 0x03
            'm', 0, 'b', 0, 'e', 0, 'd', 0, ' ', 0, 'C', 0, 'M', 0, 'S', 0, 'I', 0, 'S', 0, '-', 0, 'D', 0, 'A', 0, 'P', 0 //bString iProduct - HID device
        };
        return stringIproductDescriptor;
    }

    const uint8_t *string_imanufacturer_desc()
    {
        static const uint8_t string_imanufacturer_descriptor[] = {
            0x12,                                            /*bLength*/
            STRING_DESCRIPTOR,                               /*bDescriptorType 0x03*/
            'm', 0, 'b', 0, 'e', 0, 'd', 0, '.', 0, 'o', 0, 'r', 0, 'g', 0, /*bString iManufacturer */
        };
        return string_imanufacturer_descriptor;
    }
};

class myDAP : public BaseDAP {
public:
    myDAP(SWD* swd):BaseDAP(swd){};
    virtual void infoLED(int select, int value) {
        switch(select) {
            case 0:
                connected = value^1; 
                running = 1;
                break;
            case 1: 
                running = value^1; 
                connected = 1;
                break;
        }
    } 
};

int main()
{
   HIDDAP* hid = new HIDDAP();
   myDAP* dap = new myDAP(&swd);
   while(1) {
        HID_REPORT recv_report;
        if(hid->read(&recv_report)) {
            HID_REPORT send_report;
            dap->Command(recv_report.data, send_report.data);
            send_report.length = 64;
            hid->send(&send_report);
        }
    }

    return 0;
}

