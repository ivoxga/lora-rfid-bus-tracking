/*
 * rc522.c - Driver para o módulo leitor RFID MFRC522 via SPI.
 *
 * Implementa a comunicação de baixo nível com o MFRC522:
 * leitura/escrita de registos, cálculo de CRC, troca de tramas
 * com a tag (Request, AntiColisão, Seleção, Autenticação, Leitura, Escrita).
 */

#include "rc522.h"

/* Comandos internos do MFRC522 */
#define PCD_IDLE        0x00    // para qualquer operação em curso
#define PCD_AUTHENT     0x0E    // autenticação MIFARE
#define PCD_RECEIVE     0x08    // ativa o recetor
#define PCD_TRANSMIT    0x04    // transmite dados do FIFO
#define PCD_TRANSCEIVE  0x0C    // transmite e recebe
#define PCD_RESETPHASE  0x0F    // reset do módulo
#define PCD_CALCCRC     0x03    // calcula CRC sobre dados no FIFO

#define PICC_ANTICOLL   0x93
#define PICC_SElECTTAG  0x93

/* Endereços dos registos do MFRC522 */
#define COMMANDREG      0x01
#define COMMIENREG      0x02
#define DIVLENREG       0x03
#define COMMIRQREG      0x04
#define DIVIRQREG       0x05
#define ERRORREG        0x06
#define STATUS1REG      0x07
#define STATUS2REG      0x08
#define FIFODATAREG     0x09
#define FIFOLEVELREG    0x0A
#define CONTROLREG      0x0C
#define BITFRAMINGREG   0x0D
#define MODEREG         0x11
#define TXMODEREG       0x12
#define RXMODEREG       0x13
#define TXCONTROLREG    0x14
#define TXAUTOREG       0x15
#define TMODEREG        0x2A
#define TPRESCALERREG   0x2B
#define TRELOADREGH     0x2C
#define TRELOADREGL     0x2D
#define CRCRESULTREGM   0x21
#define CRCRESULTREGL   0x22


/* Transfere 1 byte via SPI e retorna o byte recebido */
static inline uint8_t spi_xfer(uint8_t v)
{
    return SPI1_ByteExchange(v);
}

/* Escreve um valor num registo do MFRC522 via SPI */
void MFRC522_WriteReg(uint8_t reg, uint8_t value)
{
    RC522_CS_LOW();
    spi_xfer((reg << 1) & 0x7E);   // write
    spi_xfer(value);
    RC522_CS_HIGH();
}

/* Lê o valor de um registo do MFRC522 via SPI */
uint8_t MFRC522_ReadReg(uint8_t reg)
{
    uint8_t val;
    RC522_CS_LOW();
    spi_xfer(((reg << 1) & 0x7E) | 0x80);  // read
    val = spi_xfer(0x00);
    RC522_CS_HIGH();
    return val;
}

/* Ativa bits específicos num registo */
static void MFRC522_SetBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteReg(reg, MFRC522_ReadReg(reg) | mask);
}

/* Limpa bits específicos num registo */
static void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
    MFRC522_WriteReg(reg, MFRC522_ReadReg(reg) & (uint8_t)(~mask));
}

/* Reset por hardware via pino RST, seguido de reset por software */
void MFRC522_Reset(void)
{
    RC522_RST_LOW();   __delay_ms(1);
    RC522_RST_HIGH();  __delay_ms(50);

    MFRC522_WriteReg(COMMANDREG, PCD_RESETPHASE);
    __delay_ms(10);
}

/* Liga a antena RF (ativa os dois pinos TX) */
static void MFRC522_AntennaOn(void)
{
    uint8_t v = MFRC522_ReadReg(TXCONTROLREG);
    if ((v & 0x03) != 0x03)
        MFRC522_SetBitMask(TXCONTROLREG, 0x03);
}

/* Desliga a antena RF */
static void MFRC522_AntennaOff(void)
{
    MFRC522_ClearBitMask(TXCONTROLREG, 0x03);
}

/*
 * Inicializa o MFRC522: configura pinos, temporizador interno,
 * modo de operação e liga a antena.
 */
void MFRC522_Init(void)
{
    cs_SetDigitalMode();
    cs_SetDigitalOutput();
    RC522_CS_HIGH();

    rst_SetDigitalMode();
    rst_SetDigitalOutput();
    RC522_RST_HIGH();
    
    // Temporizador interno para timeout de comunicação
    MFRC522_WriteReg(TMODEREG, 0x8D);
    MFRC522_WriteReg(TPRESCALERREG, 0x3E);
    MFRC522_WriteReg(TRELOADREGL, 30);
    MFRC522_WriteReg(TRELOADREGH, 0);
    
    MFRC522_WriteReg(TXAUTOREG, 0x40);  // ativa CRC automático na transmissão
    MFRC522_WriteReg(MODEREG, 0x3D);    // CRC preset 0x6363 (ISO 14443)

    MFRC522_AntennaOff();
    __delay_ms(1);
    MFRC522_AntennaOn();
}

/*
 * Envia dados para a tag e recebe a resposta (operação de baixo nível).
 * Suporta os comandos PCD_AUTHENT e PCD_TRANSCEIVE.
 * Retorna MI_OK, MI_NOTAGERR ou MI_ERR.
 */
static char MFRC522_ToCard(char command, char *sendData, char sendLen, char *backData, unsigned *backLen)
{
    char _status = MI_ERR;
    char irqEn = 0x00;
    char waitIRq = 0x00;
    char lastBits;
    char n;
    unsigned i;
    
    // Configura máscaras de IRQ conforme o comando
    switch (command)
    {
        case PCD_AUTHENT:    irqEn = 0x12; waitIRq = 0x10; break;
        case PCD_TRANSCEIVE: irqEn = 0x77; waitIRq = 0x30; break;
        default: break;
    }

    MFRC522_WriteReg(COMMIENREG, irqEn | 0x80);
    MFRC522_ClearBitMask(COMMIRQREG, 0x80);
    MFRC522_SetBitMask(FIFOLEVELREG, 0x80); // flush FIFO 
    MFRC522_WriteReg(COMMANDREG, PCD_IDLE);
    
    // Carrega os dados no FIFO
    for (i = 0; i < (unsigned)sendLen; i++)
        MFRC522_WriteReg(FIFODATAREG, sendData[i]);

    MFRC522_WriteReg(COMMANDREG, command);
    if (command == PCD_TRANSCEIVE)
        MFRC522_SetBitMask(BITFRAMINGREG, 0x80);    // inicia a transmissão
    
    // Aguarda IRQ de fim de operação ou timeout (~4000 iterações)
    i = 4000;
    do {
        n = (char)MFRC522_ReadReg(COMMIRQREG);
        i--;
    } while (i && !(n & 0x01) && !(n & waitIRq));

    MFRC522_ClearBitMask(BITFRAMINGREG, 0x80);

    if (i != 0)
    {
        if (!(MFRC522_ReadReg(ERRORREG) & 0x1B))    // sem erros de bit/paridade/CRC
        {
            _status = MI_OK;
            if (n & irqEn & 0x01) _status = MI_NOTAGERR;

            if (command == PCD_TRANSCEIVE)
            {
                n = (char)MFRC522_ReadReg(FIFOLEVELREG);
                lastBits = (char)(MFRC522_ReadReg(CONTROLREG) & 0x07);
                
                // Calcula número de bits recebidos
                if (lastBits) *backLen = (unsigned)((n - 1) * 8 + lastBits);
                else          *backLen = (unsigned)(n * 8);

                if (n == 0) n = 1;
                if (n > 16) n = 16;

                for (i = 0; i < (unsigned)n; i++)
                    backData[i] = (char)MFRC522_ReadReg(FIFODATAREG);
            }
        }
    }
    return _status;
}

/* Calcula CRC sobre 'length' bytes de dataIn e guarda os 2 bytes resultado em dataOut */
static void MFRC522_CRC(char *dataIn, char length, char *dataOut)
{
    char i, n;

    MFRC522_ClearBitMask(DIVIRQREG, 0x04);
    MFRC522_SetBitMask(FIFOLEVELREG, 0x80);

    for (i = 0; i < length; i++)
        MFRC522_WriteReg(FIFODATAREG, (uint8_t)*dataIn++);

    MFRC522_WriteReg(COMMANDREG, PCD_CALCCRC);
    
    // Aguarda conclusão do cálculo 
    i = (char)0xFF;
    do {
        n = (char)MFRC522_ReadReg(DIVIRQREG);
        i--;
    } while (i && !(n & 0x04));

    dataOut[0] = (char)MFRC522_ReadReg(CRCRESULTREGL);
    dataOut[1] = (char)MFRC522_ReadReg(CRCRESULTREGM);
}

/* Envia um pedido REQA/WUPA e retorna o tipo de tag em TagType */
char MFRC522_Request(char reqMode, char *TagType)
{
    char _status;
    unsigned backBits;

    MFRC522_WriteReg(BITFRAMINGREG, 0x07);
    TagType[0] = reqMode;

    _status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
    if ((_status != MI_OK) || (backBits != 0x10)) _status = MI_ERR;

    return _status;
}

/* Executa anti-colisão e lê o UID de 4 bytes da tag para serNum */
char MFRC522_AntiColl(char *serNum)
{
    char _status;
    char i;
    char serNumCheck = 0;
    unsigned unLen;

    MFRC522_WriteReg(BITFRAMINGREG, 0x00);
    serNum[0] = PICC_ANTICOLL;
    serNum[1] = 0x20;

    MFRC522_ClearBitMask(STATUS2REG, 0x08);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);

    if (_status == MI_OK)
    {
        // Verifica BCC: XOR dos 4 bytes do UID deve igualar o 5º byte
        for (i = 0; i < 4; i++) serNumCheck ^= serNum[i];
        if (serNumCheck != serNum[4]) _status = MI_ERR;
    }
    return _status;
}

/* Seleciona a tag pelo UID e retorna o tamanho de memória (SAK) ou 0 em caso de erro */
char MFRC522_SelectTag(char *serNum)
{
    char i, size;
    char _status;
    unsigned recvBits;
    char buffer[9];

    buffer[0] = PICC_SElECTTAG;
    buffer[1] = 0x70;

    for (i = 2; i < 7; i++) buffer[i] = *serNum++;

    MFRC522_CRC(buffer, 7, &buffer[7]);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);

    size = ((_status == MI_OK) && (recvBits == 0x18)) ? buffer[0] : 0;
    return size;
}

/* Autentica o setor indicado usando a chave e o UID fornecidos */
char MFRC522_Auth(char authMode, char blockAddr, char *Sectorkey, char *serNum)
{
    char _status;
    unsigned recvBits;
    char i;
    char buff[12];

    buff[0] = authMode;
    buff[1] = blockAddr;

    for (i = 2; i < 8; i++)  buff[i] = Sectorkey[i - 2];    // 6 bytes de chave
    for (i = 8; i < 12; i++) buff[i] = serNum[i - 8];       // 4 bytes do UID

    _status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
    
    // Verifica também o bit MFCrypto1On no STATUS2REG
    if ((_status != MI_OK) || !(MFRC522_ReadReg(STATUS2REG) & 0x08)) _status = MI_ERR;
    return _status;
}

/* Escreve 16 bytes no bloco indicado (requer autenticação prévia) */
char MFRC522_Write(char blockAddr, char *writeData)
{
    char _status;
    unsigned recvBits;
    char i;
    char buff[18];
    
    // Fase 1: envia comando de escrita e endereço do bloco
    buff[0] = PICC_WRITE;
    buff[1] = blockAddr;

    MFRC522_CRC(buff, 2, &buff[2]);
    _status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);

    if ((_status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) _status = MI_ERR;
    
    // Fase 2: envia os 16 bytes de dados com CRC
    if (_status == MI_OK)
    {
        for (i = 0; i < 16; i++) buff[i] = writeData[i];
        MFRC522_CRC(buff, 16, &buff[16]);

        _status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
        if ((_status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A)) _status = MI_ERR;
    }
    return _status;
}

/* Lê 16 bytes do bloco indicado para recvData (requer autenticação prévia) */
char MFRC522_Read(char blockAddr, char *recvData)
{
    char _status;
    unsigned unLen;

    recvData[0] = PICC_READ;
    recvData[1] = blockAddr;
    MFRC522_CRC(recvData, 2, &recvData[2]);

    _status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
    if ((_status != MI_OK) || (unLen != 0x90)) _status = MI_ERR;

    return _status;
}

/* Envia o comando HALT à tag, colocando-a em estado inativo */
void MFRC522_Halt(void)
{
    unsigned unLen;
    char buff[4];

    buff[0] = PICC_HALT;
    buff[1] = 0;
    MFRC522_CRC(buff, 2, &buff[2]);

    MFRC522_ClearBitMask(STATUS2REG, 0x08);
    MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
    MFRC522_ClearBitMask(STATUS2REG, 0x08);
}

/* Coloca o módulo em modo de baixo consumo (bit PowerDown no COMMANDREG) */
void MFRC522_SoftPowerDown(void)
{
    MFRC522_SetBitMask(COMMANDREG, 0x10); // PowerDown
}

/* Acorda o módulo do modo de baixo consumo e aguarda estabilização */
void MFRC522_SoftPowerUp(void)
{
    MFRC522_ClearBitMask(COMMANDREG, 0x10);
    __delay_ms(5);
}
