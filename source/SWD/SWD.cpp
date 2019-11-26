// SWD.cpp 2014/7/5
#include "SWD.h"
#include <algorithm>

SWD::SWD(PinName swdio, PinName swclk, PinName reset)
    : _swdio(swdio), _swclk(swclk), _nreset(reset)
{
    conf.turnaround = 1;
    conf.data_phase = 0;
    idle_cycles = 0;
    retry_count = 100;
    
    _cpu_delay_count = 2;
    TransferAbort = false;
}

void SWD::Setup()
{
    _swclk = 1;
    _swdio.output();
    _swdio = 1;
    _nreset.input();
    _nreset.mode(PullUp);
}

void SWD::reset()
{
    SWJPins(0x00, 0x80);
    SWJPins(0x80, 0x80);
}

void SWD::SWJSequence(int count, const uint8_t* data)
{
    for(int n = 0; n < count; n++) {
        uint8_t val = data[n/8];
        write_bit(val>>(n%8));
    }
}

uint8_t SWD::SWJPins(uint32_t value, uint32_t select ,int waittime_us)
{
    if (select & 0x01) { // swclk
        _swclk = (value & 0x01) ? 1 : 0; 
    }
    if (select & 0x02) { // swdio
        _swdio = (value & 0x02) ? 1 : 0;
    }
    if (select & 0x80) { // nReset
        if (value & 0x80) {
            _nreset.input();
        } else {    
            _nreset.output();
            _nreset = 0;
        }
    }
    if (waittime_us) {
        waittime_us = std::min(waittime_us, 3000000);
        Timer t;
        t.reset();
        t.start();
        while(t.read_us() < waittime_us) {
            if (select & 0x01) { // swclk
                if (_swclk ^ ((value & 0x01) ? 1 : 0)) {
                    continue;
                } 
            }
            if (select & 0x02) { // swdio
                if (_swdio ^ ((value & 0x02) ? 1 : 0)) {
                    continue;
                }
            }
            if (select & 0x80) { // nReset
                if (_nreset ^ ((value & 0x80) ? 1 : 0)) {
                    continue;
                }
            }
            break;
        }
    }
    return (_swclk ? 0x01 : 0x00) | (_swdio ? 0x02 : 0x00) | (_nreset ? 0x80 : 0x00);
}

void SWD::SWJClock(uint32_t clock_hz)
{
    if (clock_hz) {
        uint32_t scc = SystemCoreClock;
        _cpu_delay_count = scc / 2 / clock_hz / 3;
    } 
}

void SWD::TransferConfigure(int _idle_cycles, int _retry_count)
{
    idle_cycles = _idle_cycles;
    retry_count = _retry_count; 
}

void SWD::Configure(int turnaround, int data_phase)
{
    conf.turnaround = turnaround;
    conf.data_phase = data_phase;
}

uint8_t SWD::Transfer(uint8_t request, uint32_t *data)
{
    for(int retry = retry_count; retry >= 0; retry--) {
        uint8_t ack = rawTransfer(request, data);
        if (ack != SWD_WAIT || TransferAbort) {
            return ack;
        }
    }
    return SWD_WAIT;
}

#pragma Otime

static uint32_t calc_parity(uint32_t data, int n)
{
    uint32_t parity = 0;
    for(int i = 0; i < n; i++) {
        parity += data>>i;
    }
    return parity & 1;
}

uint8_t SWD::rawTransfer(uint8_t request, uint32_t *data)
{
    write_bit(1);          // start bit
    write_bit(request, 4); // APnDP,RnW,A2,A3
    write_bit(calc_parity(request, 4)); // parity bit
    write_bit(0);       // stop bit
    write_bit(1);       // park bit
    
    _swdio.input();
    clock_cycle(conf.turnaround);
    
    uint8_t ack = read_bit(3);
    if (ack == SWD_OK) {
        if (request & SWD_RnW) { // read
            uint32_t val = read_bit(32);
            uint32_t parity = read_bit(1);
            if (parity ^ calc_parity(val, 32)) {
                ack = SWD_ERROR;
            }
            if (data) {
                *data = val;
            }
            clock_cycle(conf.turnaround);
            _swdio.output();
        } else {                        // write
            clock_cycle(conf.turnaround);
            _swdio.output();
            uint32_t val = *data;
            write_bit(val, 32);
            write_bit(calc_parity(val, 32));
        }
        if (idle_cycles) {
            _swdio = 0;
            clock_cycle(idle_cycles);
        }
        _swdio = 1;
        return ack;
    }

    if (ack == SWD_WAIT || ack == SWD_FAULT) {
        if (conf.data_phase && (request & SWD_RnW)) {
            clock_cycle(32+1);
        }
        clock_cycle(conf.turnaround);
        _swdio.output();
        if (conf.data_phase && ((request & SWD_RnW) == 0)) {
            _swdio = 0;
            clock_cycle(32+1);
        }
        _swdio = 1;
        return ack;
    }
    clock_cycle(32 + 1 + conf.turnaround);
    _swdio.output();
    return ack;
}

void SWD::pin_delay()
{
    __IO int n = _cpu_delay_count;
    while(n-- > 0)
        ;
}

void SWD::clock_cycle(int n)
{
    while(n-- > 0) {
        _swclk = 0;
        pin_delay();
        _swclk = 1;
        pin_delay();
    }    
}

void SWD::write_bit(uint32_t data, int n)
{
    for(int i = 0; i < n; i++) {
        _swdio = (data>>i) & 1;
        clock_cycle();
    }
}

uint32_t SWD::read_bit(int n)
{
    uint32_t data = 0;
    for(int i = 0; i < n; i++) {
        _swclk = 0;
        pin_delay();
        uint32_t val = _swdio & 1;
        data |= val<<i;
        _swclk = 1;
        pin_delay();
    }    
    return data;
}
