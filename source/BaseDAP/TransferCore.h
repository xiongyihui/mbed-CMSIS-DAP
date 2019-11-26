// TransferCore.h 2013/9/14
#pragma once
#include "SWD.h"

// DAP Transfer Request
#define DAP_TRANSFER_MATCH_VALUE        (1<<4)
#define DAP_TRANSFER_MATCH_MASK         (1<<5)

// DAP Transfer Response
#define DAP_TRANSFER_MISMATCH           (1<<4)

template<typename T>
T LD(uint8_t* buf) {
    T data = 0;
    for(int i = sizeof(T)-1; i >= 0; i--) {
        data <<= 8;
        data |= buf[i];
    }
    return data;
}

template<typename T>
void ST(uint8_t* buf, T data) {
    for(int i = 0; i < sizeof(T); i++) {
        buf[i] = data>>(8*i);
    }
}

class transData {
public:
    void init(uint8_t* base, int pos);
    void append(uint32_t data);
    uint8_t* data();
    int length();

    template<typename T>
    T get() {
        T data = LD<T>(_base+_pos);
        _pos += sizeof(T);
        return data;
    }
protected:
    uint8_t* _base;
    int _pos;
};

class TransferCore {
public:
    int Transfer(SWD* swd, uint8_t* request, uint8_t* response);
    int TransferBlock(SWD* swd, uint8_t* request, uint8_t* response);
    uint16_t  match_retry; // Number of retries if read value does not match
    uint32_t  match_mask;  // Match Mask
private:
    uint8_t read(uint8_t cmd);
    uint8_t write(uint8_t cmd);
    uint8_t read_block(uint8_t cmd, int count);
    uint8_t write_block(uint8_t cmd, int count);

    bool post_read;
    bool check_write;
    transData reqData;  
    transData resData;
    int response_count;
protected:
    SWD* _swd;
};
