// TransferCore.cpp 2013/9/14
#include "TransferCore.h"

void transData::init(uint8_t* base, int pos)
{
    _base = base;
    _pos = pos;
}

void transData::append(uint32_t data)
{
    ST<uint32_t>(_base+_pos, data);
    _pos += sizeof(uint32_t);
}

uint8_t* transData::data()
{
    return _base;
}

int transData::length()
{
    return _pos;
}

int TransferCore::Transfer(SWD* swd, uint8_t* request, uint8_t* response) // 0x05
{
    _swd = swd;
    reqData.init(request, 2);
    resData.init(response, 3);
    post_read = check_write = false;
    response_count = 0;
    uint8_t ack = 0;
    for(int count = reqData.get<uint8_t>(); count > 0; count--) {
        uint8_t cmd = reqData.get<uint8_t>();
        if (cmd & SWD_RnW) { // read register
            ack = read(cmd);
            check_write = false;
        } else { // write register
            ack = write(cmd);
        }
        if (ack != SWD_OK) {
            break;
        }
        response_count++;
    }
    if (ack == SWD_OK) {
        if (post_read) { // read previous data
            uint32_t data;
            ack = _swd->Transfer(DP_RDBUFF, &data);
            resData.append(data);
        } else if (check_write) { // check last write
            ack = _swd->Transfer(DP_RDBUFF, NULL);
        }
    }
    resData.data()[0] = 0x05; // transfer
    resData.data()[1] = response_count;
    resData.data()[2] = ack;
    return resData.length();
}

uint8_t TransferCore::read(uint8_t cmd)
{
    uint8_t ack = 0;
    uint32_t data;
    if (post_read) {
        if ((cmd & SWD_APnDP) && !(cmd & DAP_TRANSFER_MATCH_VALUE)) {
            ack = _swd->Transfer(cmd, &data); // next post_read
        } else {
            ack = _swd->Transfer(DP_RDBUFF, &data);
            post_read = false;
        }
        if (ack != SWD_OK) {
            return ack;
        }
        resData.append(data);
    }
    if (cmd & DAP_TRANSFER_MATCH_VALUE) {
        uint32_t match_value = reqData.get<uint32_t>();
        if (cmd & SWD_APnDP) {
            ack = _swd->Transfer(cmd, NULL);
            if (ack != SWD_OK) {
                return ack;
            }
        }
        for(int retry = match_retry; retry >= 0; retry--) {
            ack = _swd->Transfer(cmd, &data);
            if (ack == SWD_OK && (data&match_mask) == match_value) {
                return ack;
            }
        }
        return ack | DAP_TRANSFER_MISMATCH; 
    } else {
        if (cmd & SWD_APnDP) {
            if (post_read == false) {
                ack = _swd->Transfer(cmd, NULL);
                post_read = true;
            }
        } else {
            ack = _swd->Transfer(cmd, &data);
            resData.append(data);
        }    
    }
    return ack;
}

uint8_t TransferCore::write(uint8_t cmd)
{
    if (post_read) { // read previous data
        uint32_t data;
        uint8_t ack = _swd->Transfer(DP_RDBUFF, &data);
        if (ack != SWD_OK) {
            return ack;
        }
        resData.append(data);
        post_read = false;
    }    
    if (cmd & DAP_TRANSFER_MATCH_MASK) {
        match_mask = reqData.get<uint32_t>();
        return SWD_OK;
    }
    check_write = true;
    uint32_t data = reqData.get<uint32_t>();
    return _swd->Transfer(cmd, &data);
}

int TransferCore::TransferBlock(SWD* swd, uint8_t* request, uint8_t* response)
{
    _swd = swd;
    reqData.init(request, 2);
    resData.init(response, 4);
    uint8_t ack = 0;
    response_count = 0;
    int count = reqData.get<uint16_t>();
    if (count > 0) {
        uint8_t cmd = reqData.get<uint8_t>();
        if (cmd & SWD_RnW) { // read register block
            ack = read_block(cmd, count);
        } else { // write register block
            ack = write_block(cmd, count);
        }
    }
    resData.data()[0] = 0x06; // transfer block
    ST<uint16_t>(resData.data()+1, response_count);
    resData.data()[3] = ack;    
    return resData.length(); 
}

uint8_t TransferCore::read_block(uint8_t cmd, int count)
{
    if (cmd & SWD_APnDP) { // post AP read
        uint8_t ack = _swd->Transfer(cmd, NULL);
        if (ack != SWD_OK) {
            return ack;
        }
    }
    uint8_t ack = 0;
    while(count-- > 0) { // read DP/AP register
        if (count == 0 && (cmd & SWD_APnDP)) { // last AP read
            cmd = DP_RDBUFF;
        }
        uint32_t data;
        ack = _swd->Transfer(cmd, &data);
        if (ack != SWD_OK) {
            break;
        }
        resData.append(data);
        response_count++;
    }
    return ack;
}

uint8_t TransferCore::write_block(uint8_t cmd, int count)
{
    while(count-- > 0) {
        uint32_t data = reqData.get<uint32_t>();
        uint8_t ack = _swd->Transfer(cmd, &data);
        if (ack != SWD_OK) {
            return ack;
        }
        response_count++;
    }
    return _swd->Transfer(DP_RDBUFF, NULL);
}
