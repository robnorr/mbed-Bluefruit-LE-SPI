#include "Bluefruit-LE-SPI.h"
#include <algorithm>
#include <iostream>
#include <ostream>

BluefruitLE::BluefruitLE(std::shared_ptr<SPI> _spi, DigitalOut _cs, DigitalIn _irq)
    : spi(_spi)
    , cs(_cs)
    , irq(_irq)
{
    init();
}

BluefruitLE::BluefruitLE(std::shared_ptr<SPI> _spi, std::shared_ptr<BusOut> _csBus, uint8_t _csAddr, DigitalIn _irq)
    : spi(_spi)
    , cs(NC)
    , csBus(_csBus)
    , csAddr(_csAddr)
    , irq(_irq)
{
    init();
}

void BluefruitLE::setupAndConnect(std::string name, std::string baud)
{
    echo(false);
    requestInfo();

    setName(name);
    wait_us(1000000);
    while(!isConnected())
    {
        //unconnected
        wait_us(1000000);
    }
    wait_us(5000000);

    setBaud(baud);
}


void BluefruitLE::init()
{
    //init
    printf("BLE: init\r\n");
    
    setTXheader(_MSG_COMMAND, _SDEP_INITIALIZE, 0);
    activateCS();
    spi->write(txbuf, 4, rxbuf, 0);
    deactivateCS();

    // //factory reset
    // printf("BLE: factory reset\r\n");
    // setTXheader(_MSG_COMMAND, _SDEP_ATCOMMAND, 0x0F);
    // txbuf[4] = 'A';
    // txbuf[5] = 'T';
    // txbuf[6] = '+';
    // txbuf[7] = 'F';
    // txbuf[8] = 'A';
    // txbuf[9] = 'C';
    // txbuf[10] = 'T';
    // txbuf[11] = 'O';
    // txbuf[12] = 'R';
    // txbuf[13] = 'Y';
    // txbuf[14] = 'R';
    // txbuf[15] = 'E';
    // txbuf[16] = 'S';
    // txbuf[17] = 'E';
    // txbuf[18] = 'T';
    // activateCS();
    // spi->write(txbuf, 19, rxbuf, 0);
    // deactivateCS();

    // wait_us(1000000);

    // fillTX255(0, 8);
    // activateCS();
    // spi->write(txbuf, 8, rxbuf, 8);
    // deactivateCS();

    // for(int i = 0; i < 8; ++i)
    // {
    //     printf("%i ", rxbuf[i]);
    // }
    // printf("\r\n");

    // if(!(rxbuf[4] == 'O' && rxbuf[5] == 'K'))
    // {
    //     printf("BLE factory reset failed\r\n");
    //     while(1);
    // }

}

void BluefruitLE::setTXheader(const char type, const uint16_t command, char plen)
{
    txbuf[0] = type;
    txbuf[1] = command & 0xFF;
    txbuf[2] = (command & 0xFF00) >> 8;
    txbuf[3] = plen;
}

void BluefruitLE::fillTX255(size_t offset, size_t len)
{
    for(int i = 0; i < len; ++i)
    {
        txbuf[offset + i] = 0xFF;
    }
}

void BluefruitLE::clearRxBuf()
{
    for(int i = 0; i < 128; ++i)
    {
        rxbuf[i] = 0x00;
    }
}

void BluefruitLE::echo(bool enable)
{
    wait_us(1000000);
    printf("BLE: echo %i\r\n", enable);
    setTXheader(_MSG_COMMAND, _SDEP_ATCOMMAND, 5);
    txbuf[4] = 'A';
    txbuf[5] = 'T';
    txbuf[6] = 'E';
    txbuf[7] = '=';
    txbuf[8] = '0';
    activateCS();
    spi->write(txbuf, 9, rxbuf, 9);
    deactivateCS();

    wait_us(10000);

    fillTX255(0, 8);
    activateCS();
    spi->write(txbuf, 8, rxbuf, 8);
    deactivateCS();
    for(int i = 0; i < 8; ++i)
    {
        printf("%i ", rxbuf[i]);
    }
    printf("\r\n");

    if(!(rxbuf[4] == 'O' && rxbuf[5] == 'K'))
    {
        printf("BLE echo failed\r\n");
        while(1);
    }

}

void BluefruitLE::requestInfo()
{
    wait_us(100000);
    printf("BLE: request info\r\n");
    setTXheader(_MSG_COMMAND, _SDEP_ATCOMMAND, 3);
    txbuf[4] = 'A';
    txbuf[5] = 'T';
    txbuf[6] = 'I';
    fillTX255(7, 20);
    activateCS();
    spi->write(txbuf, 20, rxbuf, 20);
    deactivateCS();

    // better than strstr because strstr fails if there are 0 ('\0') chars in rx
    char ok[] = "OK";
    char err[] = "ERROR";
    while((std::search(rxbuf, rxbuf + 20, ok, ok + 1) == rxbuf + 20) && (std::search(rxbuf, rxbuf + 20, err, err + 4) == rxbuf + 20))
    {
        wait_us(10000);
        for (int i = 0; i < 20; ++i)
        {
            txbuf[i] = 0xFF;
        }
        activateCS();
        spi->write(txbuf, 20, rxbuf, 20);
        deactivateCS();
        for(int i = 0; i < 20; ++i)
        {
            printf("%i ", rxbuf[i]);
        }
        printf("\r\n");
    }
}

bool BluefruitLE::isConnected()
{
    printf("BLE: check connection state\r\n");
    setTXheader(_MSG_COMMAND, _SDEP_ATCOMMAND, 13);
    txbuf[4] = 'A';
    txbuf[5] = 'T';
    txbuf[6] = '+';
    txbuf[7] = 'G';
    txbuf[8] = 'A';
    txbuf[9] = 'P';
    txbuf[10] = 'G';
    txbuf[11] = 'E';
    txbuf[12] = 'T';
    txbuf[13] = 'C';
    txbuf[14] = 'O';
    txbuf[15] = 'N';
    txbuf[16] = 'N';
    activateCS();
    spi->write(txbuf, 17, rxbuf, 17);
    deactivateCS();
    for(int i = 0; i < 17; ++i)
    {
        printf("%i ", rxbuf[i]);
    }
    printf("\r\n");

    fillTX255(0, 11);
    activateCS();
    spi->write(txbuf, 11, rxbuf, 11);
    deactivateCS();
    for(int i = 0; i < 11; ++i)
    {
        printf("%i ", rxbuf[i]);
    }
    printf("\r\n");
    return rxbuf[4] == '1';
}

bool BluefruitLE::isAtLeastVersion(char * version)
{
    // wait_us(1000);
    printf("BLE: check version at least %s\r\n", version);
    char tx[16] = { 0 };
    char rx[16] = { 0 };
    tx[0] = 0x10;
    tx[1] = 0x00;
    tx[2] = 0x0A;
    tx[3] = 0x05;
    tx[4] = 'A';
    tx[5] = 'T';
    tx[6] = 'I';
    tx[7] = '=';
    tx[8] = '4';
    activateCS();
    spi->write(tx, 9, rx, 9);
    deactivateCS();
    for(int i = 0; i < 9; ++i)
    {
        printf("%i ", rx[i]);
    }
    printf("\r\n");
    wait_us(10000);

    for (int i = 0; i < 15; ++i)
    {
        tx[i] = 0xFF;
    }
    activateCS();
    spi->write(tx, 15, rx, 15);
    deactivateCS();
    for(int i = 0; i < 15; ++i)
    {
        printf("%i ", rx[i]);
    }
    printf("\r\n");
    return std::strncmp(rx + 4, version, 5) >= 0;
}

void BluefruitLE::setLEDMode(char * mode)
{
    wait_us(1000000);
    printf("BLE: set LED mode %s\r\n", mode);
    char tx[35] = { 0 };
    char rx[35] = { 0 };
    tx[0] = 0x10;
    tx[1] = 0x00;
    tx[2] = 0x0A;
    tx[3] = 0x90;
    tx[4] = 'A';
    tx[5] = 'T';
    tx[6] = '+';
    tx[7] = 'H';
    tx[8] = 'W';
    tx[9] = 'M';
    tx[10] = 'o';
    tx[11] = 'd';
    tx[12] = 'e';
    tx[13] = 'L';
    tx[14] = 'E';
    tx[15] = 'D';
    tx[16] = '=';
    tx[17] = 'M';
    tx[18] = 'O';
    tx[19] = 'D';
    tx[20] = 0x10;
    tx[21] = 0x10;
    tx[22] = 0x00;
    tx[23] = 0x0A;
    tx[24] = 0x01;
    tx[25] = 'E';
    tx[26] = 0xFF;
    tx[27] = 0xFF;
    tx[28] = 0xFF;
    tx[29] = 0xFF;
    tx[30] = 0xFF;
    tx[31] = 0xFF;
    tx[32] = 0xFF;
    tx[33] = 0xFF;
    tx[34] = 0xFF;
    activateCS();
    spi->write(tx, 35, rx, 35);
    deactivateCS();
    for(int i = 0; i < 35; ++i)
    {
        printf("%i ", rx[i]);
    }
    printf("\r\n"); 
    //not working yet (not returning "OK")
}

size_t BluefruitLE::rx(char * rx)
{
    wait_us(5000);
    // printf("BLE: rx\r\n");
    char tx[32] = { 0 };
    // char rx[32] = { 0 };
    tx[0] = 0x10;
    tx[1] = 0x00;
    tx[2] = 0x0A;
    tx[3] = 0x0C;
    tx[4] = 'A';
    tx[5] = 'T';
    tx[6] = '+';
    tx[7] = 'B';
    tx[8] = 'L';
    tx[9] = 'E';
    tx[10] = 'U';
    tx[11] = 'A';
    tx[12] = 'R';
    tx[13] = 'T';
    tx[14] = 'R';
    tx[15] = 'X';
    tx[16] = 0xFF;
    tx[17] = 0xFF;
    tx[18] = 0xFF;
    tx[19] = 0xFF;
    tx[20] = 0xFF;
    tx[21] = 0xFF;
    tx[22] = 0xFF;
    tx[23] = 0xFF;
    activateCS();
    spi->write(tx, 24, rx, 24);
    deactivateCS();
    // for(int i = 0; i < 24; ++i)
    // {
    //     printf("%i ", rx[i]);
    // }
    char ok[] = "OK";
    char err[] = "ERROR";
    if((std::search(rx, rx + 24, ok, ok + 1) == rx + 24) && (std::search(rx, rx + 24, err, err + 4) == rx + 24))
    {
        for(int i = 0; i < 32; ++i)
        {
            tx[i] = 0xFF;
        }
        wait_us(5000);
        activateCS();
        spi->write(tx, 32, rx, 32);
        deactivateCS();
        // for(int i = 0; i < 32; ++i)
        // {
        //     printf("%i ", rx[i]);
        // }
        // printf("\r\n");
        //26-49
        if((std::search(rx, rx + 32, ok, ok + 1) == rx + 32) && (std::search(rx, rx + 32, err, err + 4) == rx + 32))
        {
            for(int i = 0; i < 32; ++i)
            {
                tx[i] = 0xFF;
            }
            wait_us(5000);
            activateCS();
            spi->write(tx, 32, rx+32, 32);
            deactivateCS();
            // for(int i = 0; i < 32; ++i)
            // {
            //     printf("%i ", rx[i]);
            // }
            // printf("\r\n");
            return 64;
        }
        return 32;
    }
    return 0;
}

size_t BluefruitLE::tx(char * txbuf, size_t txlen, char * rxbuf)
{
    char tx[20] = { 0 };
    char rx[20] = { 0 };
    bool more = true;
    size_t pos = 0;
    // printf("a\r\n");
    if (txlen > 256)
    {
        txlen = 256;
    }
    // printf("b\r\n");

    while((txlen - pos) > 0)
    {
        // printf("c\r\n");
        if(txlen - pos <= 16)
        {
            more = false;
        }
        // printf("d\r\n");
        size_t plen = txlen - pos;
        plen = plen < 16 ? plen : 16;
        tx[0] = 0x10;
        tx[1] = 0x00;
        tx[2] = 0x0A;
        tx[3] = (more ? 0x80 : 0x00) | plen;
        // printf("e\r\n");
        for(int i = 0; i < plen; ++i)
        {
            tx[4 + i] = txbuf[i + pos];
        }
        wait_us(5000);
        pos += plen;
        activateCS();
        spi->write(tx, 20, rx, 20);
        deactivateCS();
        // for (int i = 0; i < 20; ++i)
        // {
        //     printf("%i ", rx[i]);
        // }
        // printf("\r\n");
    }

    size_t timeout = 20; //200ms
    // printf("timeout %i\r\n", timeout);
    while(!irq.read() && timeout--)
    {
        // printf("no irq %i\r\n", timeout);
        wait_us(10000);
    }
    // printf("g\r\n");

    size_t rsppos = 0;
    while(irq.read())
    {
        // printf("h\r\n");
        wait_us(5000);

        for(int i = 0; i < 20; ++i)
        {
            tx[i] = 0xFF;
        }
        activateCS();
        spi->write(tx, 20, rx, 20);
        deactivateCS();
        size_t rsplen = (rx[3] >= 16 ? 16 : rx[3]);
        for(int i = 0; i < rsplen; ++i)
        {
            rxbuf[i + rsppos] = rx[i + 4];
        }
        rsppos += rsplen;
    }
    // printf("ret\r\n");
    return rsppos;
}

void BluefruitLE::setName(std::string name)
{
    std::string command = "AT+GAPDEVNAME=" + name;
    tx(const_cast<char*>(command.c_str()), command.length(), rxbuf);
}

void BluefruitLE::setBaud(std::string baud)
{
    std::string command = "AT+BAUDRATE=" + baud;
    tx(const_cast<char*>(command.c_str()), command.length(), rxbuf);
}

void BluefruitLE::send(std::string message)
{
    while(message.length() > 128)
    {
        send(message.substr(0, 128));
        message = message.substr(128, message.length() - 128);
    }
    std::string command = "AT+BLEUARTTX=" + message;
    tx(const_cast<char*>(command.c_str()), command.length(), rxbuf);
}

std::string BluefruitLE::receive()
{
    auto len = rx(rxbuf);
    if(len == 0)
    {
        // printf("len = 0");
        return {};
    }
    if(rxbuf[4] == 79 && rxbuf[5] == 75)  // OK
    {
        // printf("OK no message");
        return {};
    }

    char ok[] = "OK";
    auto end = std::search(rxbuf, rxbuf + 128, ok, ok + 2);
    // printf("4: %i, 5: %i, end: %i", rxbuf[4], rxbuf[5], *end);
    std::string msg{rxbuf + 4, end};
    // clearRxBuf();
    return msg;
    
}