/*
 * rc522.h - Interface pública do driver MFRC522 (leitor RFID via SPI).
 *
 * Define os pinos de controlo (CS e RST), os códigos de retorno,
 * os comandos MIFARE e os protótipos das funções do driver.
 */

#ifndef RC522_H
#define RC522_H

#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/system/system.h" 

/* Controlo dos pinos CS e RST - ajustar aos nomes definidos no MCC */
#define RC522_CS_LOW()   cs_SetLow()
#define RC522_CS_HIGH()  cs_SetHigh()
#define RC522_RST_HIGH() rst_SetHigh()
#define RC522_RST_LOW()  rst_SetLow()

/* Códigos de retorno das funções do driver */
#define MI_OK        0  // operação bem-sucedida 
#define MI_NOTAGERR  1  // nenhuma tag detetada 
#define MI_ERR       2  // erro de comunicação/CRC 

/* Comandos PICC (tag) usados nas operações MIFARE */
#define PICC_REQIDL     0x26    // Request em modo IDLE  
#define PICC_REQALL     0x52    // Request para todas as tags 
#define PICC_AUTHENT1A  0x60    // Autenticação com chave A 
#define PICC_AUTHENT1B  0x61    // Autenticação com chave B 
#define PICC_READ       0x30    // Leitura de bloco 
#define PICC_WRITE      0xA0    // Escrita de bloco 
#define PICC_HALT       0x50    // Coloca a tag em estado HALT

/* API pública */
void    MFRC522_Init(void);
void    MFRC522_Reset(void);
void    MFRC522_Halt(void);
char    MFRC522_Request(char reqMode, char *TagType);
char    MFRC522_AntiColl(char *serNum);
char    MFRC522_SelectTag(char *serNum);
char    MFRC522_Auth(char authMode, char blockAddr, char *Sectorkey, char *serNum);
char    MFRC522_Read(char blockAddr, char *recvData);
char    MFRC522_Write(char blockAddr, char *writeData);
void    MFRC522_SoftPowerDown(void);
void    MFRC522_SoftPowerUp(void);

/* Acesso direto a registos (uso interno / debug) */
void    MFRC522_WriteReg(uint8_t reg, uint8_t value);
uint8_t MFRC522_ReadReg(uint8_t reg);

#endif