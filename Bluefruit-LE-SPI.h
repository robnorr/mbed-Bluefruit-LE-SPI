#pragma once

#include "mbed.h"
#include <memory>

    
#define _MSG_COMMAND 0x10
#define _MSG_RESPONSE 0x20
#define _MSG_ALERT 0x40
#define _MSG_ERROR 0x80

#define _SDEP_INITIALIZE 0xBEEF
#define _SDEP_ATCOMMAND 0x0A00

class BluefruitLE
{
public:
    BluefruitLE(std::shared_ptr<SPI> _spi, DigitalOut _cs, DigitalIn _irq);
    BluefruitLE(std::shared_ptr<SPI> _spi, std::shared_ptr<BusOut> _csBus, uint8_t _csAddr, DigitalIn _irq);
    void init();
    void echo(bool enable);
    void requestInfo();
    bool isConnected();
    bool isAtLeastVersion(char*);
    void setLEDMode(char*); //"DISABLE" "MODE" "BLEUART" "HWUART" "SPI" "MANUAL"
    size_t rx(char * buf); 
    size_t tx(char * txbuf, size_t txlen, char * rxbuf);

private:
    void activateCS();
    void deactivateCS();

    void setTXheader(const char type, const uint16_t command, char plen);
    void fillTX255(size_t offset, size_t len);

    std::shared_ptr<SPI> spi;
    DigitalOut cs;
    DigitalIn irq;
    std::shared_ptr<BusOut> csBus;
    uint8_t csAddr;

    char txbuf[20];
    char rxbuf[20];
};



inline void BluefruitLE::activateCS()
{
    if(cs.is_connected())
    {
        cs = 0;
    }
    else
    {
        csBus->write(csAddr);
    }
}

inline void BluefruitLE::deactivateCS()
{
    if(cs.is_connected())
    {
        cs = 1;
    }
    else
    {
        csBus->write(0x00);
    }
}
