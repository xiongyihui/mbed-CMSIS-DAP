// BaseDAP.cpp 2013/9/14
#include "BaseDAP.h"

BaseDAP::BaseDAP(SWD* swd) : _swd(swd)
{
}

BaseDAP::~BaseDAP()
{
}

int BaseDAP::Command(uint8_t* request, uint8_t* response)
{
    switch(*request) {
        case 0x00: return Info(request, response);
        case 0x01: return LED(request, response); 
        case 0x02: return Connect(request, response); 
        case 0x03: return Disconnect(request, response);
        case 0x04: return TransferConfigure(request, response);
        case 0x05: return Transfer(request, response);
        case 0x06: return TransferBlock(request, response);

        case 0x08: return WriteABORT(request, response);
    
        case 0x10: return SWJ_Pins(request, response);
        case 0x11: return SWJ_Clock(request, response);
        case 0x12: return SWJ_Sequence(request, response);
        case 0x13: return SWD_Configure(request, response);
        
        case 0x80 ... 0x9f: return Vendor(request, response);
    };
    return Invalid(request, response);
}

/* virtual */ const char* BaseDAP::getInfo(int id)
{
    const char* info_str[] = {
        NULL,     // 1 VENDOR
        NULL,     // 2 PRODUCT
        NULL,     // 3 SER_NUM
        "1.0",    // 4 FW_VER
        NULL,     // 5 DEVICE_VENDOR
        NULL,     // 6 DEVICE_NAME
    };
    if (id >= 1 && id <= 6) {
        return info_str[id-1];
    }
    return NULL;
}

/* virtual */ int BaseDAP::Info(uint8_t* request, uint8_t* response) // 0x00
{
    const char* s;
    int slen;
    response[0] = 0x00; // Info
    int length = 2;
    int id = request[1];
    switch(id) {
        case 1 ... 6: // VENDOR PRODUCT SER_NUM FW_VER DEVICE_VENDOR DEVICE_NAME
            slen = 0;
            s = getInfo(id);
            if (s) {     
                slen = strlen(s) + 1;
                memcpy(response+2, s, slen);
            }
            response[1] = slen;    
            length += slen;
            break;
        case 0xf0:          // CAPABILITIES
            response[1] = sizeof(uint8_t);
            response[2] = 0x01; // SWD
            length += sizeof(uint8_t);
            break;
        case 0xfe:          // PACKET_COUNT
            response[1] = sizeof(uint8_t);
            response[2] = 1;
            length += sizeof(uint8_t);
            break;
        case 0xff:          // PACKET_SIZE
            response[1] = sizeof(uint16_t);
            ST<uint16_t>(response+2, 64);
            length += sizeof(uint16_t);
            break;
        default:
            response[1] = DAP_ERROR;
            break;
    }
    return length;
}

/* virtual */ void BaseDAP::infoLED(int select, int value)
{
}

int BaseDAP::LED(uint8_t* request, uint8_t* response) // 0x01
{
    infoLED(request[1], request[2]&1);
    response[0] = 0x01; // LED
    response[1] = DAP_OK;
    return 2;
}
   
int BaseDAP::Connect(uint8_t* request, uint8_t* response) // 0x02
{
    response[0] = 0x02; // Connect
    response[1] = DAP_PORT_DISABLED;
    if (_swd) {
        if (request[1] == DAP_PORT_AUTODETECT || request[1] == DAP_PORT_SWD) {
            response[1] = DAP_PORT_SWD;
            _swd->Setup();
        }
    }
    return 2;
}

int BaseDAP::Disconnect(uint8_t* request, uint8_t* response) // 0x03
{
    response[0] = 0x03; // disconnect
    response[1] = DAP_OK;
    return 2;
}

int BaseDAP::TransferConfigure(uint8_t* request, uint8_t* response) // 0x04
{
    uint8_t idle_cycles = request[1];
    uint16_t retry_count = LD<uint16_t>(request+2);
    transfer.match_retry = LD<uint16_t>(request+4);
    _swd->TransferConfigure(idle_cycles, retry_count);
    response[0] = 0x04; //tansfer configure
    response[1] = DAP_OK;
    return 2;
}

int BaseDAP::Transfer(uint8_t* request, uint8_t* response) // 0x05
{
    return transfer.Transfer(_swd, request, response);
}

int BaseDAP::TransferBlock(uint8_t* request, uint8_t* response) // 0x06
{
    return transfer.TransferBlock(_swd, request, response);
}

int BaseDAP::WriteABORT(uint8_t* request, uint8_t* response) // 0x08
{
    uint32_t data = LD<uint32_t>(request+2);
    uint8_t ack = _swd->Transfer(DP_ABORT, &data);
    response[0] = 0x08; // write abort
    response[1] = ack == SWD_OK ? DAP_OK : DAP_ERROR;
    return 2;
}

int BaseDAP::Delay(uint8_t* request, uint8_t* response) // 0x09
{
    int waittime_ms = LD<uint16_t>(request+1);
    wait_ms(waittime_ms);
    response[0] = 0x09;
    response[1] = DAP_OK;
    return 2;
}

int BaseDAP::ResetTarget(uint8_t* request, uint8_t* response) // 0x0A
{
    response[0] = 0x0a;
    response[1] = 0;
    response[2] = DAP_OK;
    return 3;
}

int BaseDAP::SWJ_Pins(uint8_t* request, uint8_t* response) // 0x10
{
    uint32_t value = request[1];
    uint32_t select = request[2];
    uint32_t waittime_us = LD<uint32_t>(request+3);
    response[0] = 0x10; // swj pins
    response[1] = _swd->SWJPins(value, select, waittime_us);
    return 2;    
}

int BaseDAP::SWJ_Clock(uint8_t* request, uint8_t* response) // 0x11
{
    uint32_t clock = LD<uint32_t>(request+1);
    _swd->SWJClock(clock);
    response[0] = 0x11; // swj clock
    response[1] = DAP_OK;
    return 2;
}

int BaseDAP::SWJ_Sequence(uint8_t* request, uint8_t* response) // 0x12
{
    int count = request[1];
    if (count == 0) {
        count = 256;
    }
    _swd->SWJSequence(count, request+2);
    response[0] = 0x12; // swj sequence
    response[1] = DAP_OK;
    return 2;
}

int BaseDAP::SWD_Configure(uint8_t* request, uint8_t* response) // 0x13
{
    uint8_t cfg = request[1];
    _swd->Configure((cfg&0x03)+1, cfg&0x04 ? 1: 0); 
    response[0] = 0x13; // swd configure
    response[1] = DAP_OK;
    return 2;
}

/* virtual */ int BaseDAP::Vendor(uint8_t* request, uint8_t* response) // 0x80 ... 0x9f
{
    switch(request[0]) {
        case 0x80: return Vendor0(request, response);
    }
    response[0] = 0xff; // invalid
    return 1;    
}

int BaseDAP::Vendor0(uint8_t* request, uint8_t* response) // 0x80
{
    const char* board_id = "1040123456789"; // lpc11u24
    int len = strlen(board_id);
    response[0] = 0x80;
    response[1] = len;
    memcpy(response+2, board_id, len);  
    return len + 1;
}

/* virtual */ int BaseDAP::Invalid(uint8_t* request, uint8_t* response)
{
    response[0] = 0xff; // invalid
    return 1;    
}
