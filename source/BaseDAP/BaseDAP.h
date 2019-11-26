// BaseDAP.h 2013/9/14
#pragma once
#include "SWD.h"
#include "TransferCore.h"

// DAP Status Code
#define DAP_OK                          0
#define DAP_ERROR                       0xFF

// DAP Port
#define DAP_PORT_AUTODETECT             0       // Autodetect Port
#define DAP_PORT_DISABLED               0       // Port Disabled (I/O pins in High-Z)
#define DAP_PORT_SWD                    1       // SWD Port (SWCLK, SWDIO) + nRESET

/** DAP(Debug Access Port) interface
 * 
 * Example:
 * @code
 * #include "BaseDAP.h"
 * #include "USBDAP.h"
 * 
 * USBDAP hid(64, 64, 0x0d28, 0x0204);
 * 
 * SWD swd(p21,p22,p17); // SWDIO(dp12),SWCLK(dp3),nReset(dp23)
 * DigitalOut connected(LED1);
 * DigitalOut running(LED2);
 * class myDAP : public BaseDAP {
 * public:
 *     myDAP(SWD* swd):BaseDAP(swd){};
 *     virtual void infoLED(int select, int value) {
 *         switch(select) {
 *             case 0: connected = value; break;
 *             case 1: running = value; break;
 *         }
 *     } 
 * };
 * 
 * int main()
 * {
 *     HID_REPORT recv_report;
 *     HID_REPORT send_report;
 * 
 *     myDAP* dap = new myDAP(&swd);
 *     while(1) {
 *         if(hid.readNB(&recv_report)) {
 *             dap->Command(recv_report.data, send_report.data);
 *             send_report.length = 64;
 *             hid.send(&send_report);
 *         }
 *     }    
 * }
 * @endcode
 */
class BaseDAP {
public:
    /** Create a DAP(Debug Access Port) interface
     * @param swd assign SWD interface
     */
    BaseDAP(SWD* swd);
    virtual ~BaseDAP();
    /** Process command
     * @param request request data(64 bytes)
     * @param response response data(64 bytes)
     * @returns response length 
     */
    int Command(uint8_t* request, uint8_t* response);
protected:
    virtual const char* getInfo(int id);
    /** LED indicator
     * @param selct 0...connected LED, 1...running LED
     * @param value 0...OFF, 1...ON
     */
    virtual void infoLED(int select, int value);

    virtual int Info(uint8_t* request, uint8_t* response);  // 0x00
    int LED(uint8_t* request, uint8_t* response);           // 0x01
    int Connect(uint8_t* request, uint8_t* response);       // 0x02
    int Disconnect(uint8_t* request, uint8_t* response);    // 0x03
    int TransferConfigure(uint8_t* request, uint8_t* response);// 0x04
    int Transfer(uint8_t* request, uint8_t* response);      // 0x05
    int TransferBlock(uint8_t* request, uint8_t* response); // 0x06

    int WriteABORT(uint8_t* request, uint8_t* response);    // 0x08
    int Delay(uint8_t* request, uint8_t* response);         // 0x09
    int ResetTarget(uint8_t* request, uint8_t* response);   // 0x0A
    int SWJ_Pins(uint8_t* request, uint8_t* response);      // 0x10
    int SWJ_Clock(uint8_t* request, uint8_t* response);     // 0x11
    int SWJ_Sequence(uint8_t* request, uint8_t* response);  // 0x12
    int SWD_Configure(uint8_t* request, uint8_t* response); // 0x13
    int Vendor0(uint8_t* request, uint8_t* response);       // 0x80
    /** vendor command
     */
    virtual int Vendor(uint8_t* request, uint8_t* response);// 0x80-0x9f
    virtual int Invalid(uint8_t* request, uint8_t* response);

    TransferCore transfer;
    SWD* _swd;
};
