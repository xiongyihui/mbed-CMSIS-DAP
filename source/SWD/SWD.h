// SWD.h 2013/9/13
#pragma once
#include "mbed.h"

#define SWD_OK     0x01
#define SWD_WAIT   0x02
#define SWD_FAULT  0x04
#define SWD_ERROR  0x08

#define SWD_APnDP  0x01
#define SWD_RnW    0x02

#define DP_ABORT       0<<0|0<<1|0x00
#define DP_IDCODE      0<<0|1<<1|0x00
#define DP_CTRL_STAT   0<<0|0<<1|0x04
#define DP_CTRL_STAT_R 0<<0|1<<1|0x04
#define DP_SELECT      0<<0|0<<1|0x08
#define DP_RDBUFF      0<<0|1<<1|0x0c

#define AP_CSW         1<<0|0<<1|0x00
#define AP_CSW_R       1<<0|1<<1|0x00
#define AP_TAR         1<<0|0<<1|0x04
#define AP_TAR_R       1<<0|1<<1|0x04
#define AP_DRW_W       1<<0|0<<1|0x0c
#define AP_DRW_R       1<<0|1<<1|0x0c

#define CSYSPWRUPACK 0x80000000
#define CDBGPWRUPACK 0x20000000
#define CSYSPWRUPREQ 0x40000000
#define CDBGPWRUPREQ 0x10000000

#define TRNNORMAL    0x00000000
#define MASKLANE     0x00000f00

// AP Control and Status Word definitions
#define CSW_SIZE     0x00000007
#define CSW_SIZE8    0x00000000
#define CSW_SIZE16   0x00000001
#define CSW_SIZE32   0x00000002
#define CSW_ADDRINC  0x00000030
#define CSW_NADDRINC 0x00000000
#define CSW_SADDRINC 0x00000010
#define CSW_PADDRINC 0x00000020
#define CSW_DBGSTAT  0x00000040
#define CSW_TINPROG  0x00000080
#define CSW_HPROT    0x02000000
#define CSW_MSTRTYPE 0x20000000
#define CSW_MSTRCORE 0x00000000
#define CSW_MSTRDBG  0x20000000
#define CSW_RESERVED 0x01000000

#define CSW_VALUE  (CSW_RESERVED | CSW_MSTRDBG | CSW_HPROT | CSW_DBGSTAT | CSW_SADDRINC)

/** SWD(Serial Wire Debug) interface
 * 
 * Example:
 * @code
 * #include "SWD.h"
 * #include "BaseDAP.h"
 * #include "USBDAP.h"
 * 
 * USBDAP hid(64, 64, 0x0d28, 0x0204);
 * 
 * SWD swd(PTB8,PTB9,PTB10); // SWDIO(dp12),SWCLK(dp3),nReset(dp23)
 * DigitalOut connected(LED_GREEN);
 * DigitalOut running(LED_RED);
 * class myDAP : public BaseDAP {
 * public:
 *     myDAP(SWD* swd):BaseDAP(swd){};
 *     virtual void infoLED(int select, int value) {
 *         switch(select) {
 *             case 0: connected = value^1; running = 1; break;
 *             case 1: running = value^1; connected = 1; break;
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
class SWD {
public:
    /** Create SWD(Serial Wire Debug) interface
     * @param swdio SWD(swdio) pin
     * @param swclk SWD(swclk) pin
     * @param reset reset pin
     */
    SWD(PinName swdio, PinName swclk, PinName reset);
    virtual ~SWD() {}
    void Setup();
    void SWJSequence(int count, const uint8_t* data);
    uint8_t SWJPins(uint32_t value, uint32_t select ,int waittime_us = 0);
    void SWJClock(uint32_t clock_hz);
    void TransferConfigure(int idle_cycles, int retry_count);
    void Configure(int turnaround, int data_phase);
    uint8_t Transfer(uint8_t request, uint32_t *data);
    __IO bool TransferAbort;
    void reset();
private:
    uint8_t rawTransfer(uint8_t request, uint32_t *data);
    void pin_delay();
    void clock_cycle(int n = 1);
    void write_bit(uint32_t data, int n = 1);
    uint32_t read_bit(int n = 1);
protected:
    struct {
        int turnaround;
        int data_phase;
    } conf;
    int idle_cycles;
    int retry_count;
    int _cpu_delay_count;
    DigitalInOut _swdio;
    DigitalOut _swclk;
    DigitalInOut _nreset;
};
