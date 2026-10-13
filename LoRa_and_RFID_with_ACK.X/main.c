/*
 * Sistema de paragem de autocarro com RFID, rede LoRa mesh e confirmação ACK.
 *
 * Cada LoRa representa uma paragem física. O ID da paragem corresponde
 * ao MAC do módulo LoRa, guardado na EEPROM interna.
 *
 * Funcionamento:
 *   - Quando um autocarro passa na paragem, o leitor RFID lê o seu ID.
 *   - A paragem constrói uma trama [ID_BUS | STOP_H | STOP_L | DIR] e
 *     envia-a para os nós vizinhos (MAC+1 frente, MAC-1 trás) via LoRa.
 *   - Cada nó recebe a trama, valida o autocarro, envia ACK ao remetente
 *     e reencaminha a trama na direção indicada pelo campo DIR.
 *   - O nó transmissor aguarda o ACK; em caso de timeout, reenvia até
 *     MAX_RETRIES tentativas.
 *
 * Modos de configuração (pinos físicos):
 *   lora_config  - ponte UART para configurar o módulo LoRa com comandos AT
 *   work_mode    - gerir a lista de autocarros autorizados nesta paragem
 *   test_lora    - envia uma trama de teste para o MAC 0x0003
 *   dev          - escreve valores padrão na EEPROM (modo desenvolvimento)
 *
 * A lógica principal é implementada como máquina de estados com goto.
 */

#include "mcc_generated_files/system/system.h"
#include <string.h>
#include "rc522.h"

/* Valores padrão para testes - descomentar o DEFAULT_STOP_ID pretendido */
//#define DEFAULT_STOP_ID   0x0001
//#define DEFAULT_STOP_ID   0x0002
//#define DEFAULT_STOP_ID   0x0003
#define DEFAULT_STOP_ID   0x0004
static const uint8_t DEFAULT_BUSES[] = { 0x01, 0x02, 0x03, 0x04 };
#define DEFAULT_BUS_COUNT (sizeof(DEFAULT_BUSES)/sizeof(DEFAULT_BUSES[0]))

/* Macros para drenar os buffers RX de cada UART */
#define PC_DRAIN_RX()        do{ while(UART2_IsRxReady()) (void)UART2_Read(); }while(0)
#define LORA_DRAIN_RX()      do{ while(UART1_IsRxReady()) (void)UART1_Read(); }while(0)

/* Macros de escrita bloqueante nas duas UARTs */
#define PC_WRITE_TEXT(s)     do{ const char *_p=(s); while(*_p){ while(!UART2_IsTxReady()){} UART2_Write(*_p++); } }while(0)
#define LORA_WRITE_TEXT(s)   do{ const char *_p=(s); while(*_p){ while(!UART1_IsTxReady()){} UART1_Write(*_p++); } }while(0)

/* Endereços EEPROM */
#define EE_ID_STOP_H    0x00    // byte alto do MAC/ID da paragem
#define EE_ID_STOP_L    0x01    // byte baixo do MAC/ID da paragem
#define EE_BUS_COUNT    0x02    // número de autocarros autorizados
#define EE_BUS_LIST     0x03    // início da lista (0x03..0x16)
#define EE_MAGIC        0x30    // byte de controlo de inicialização
#define EE_MAGIC_VALUE  0xA5    // valor esperado se EEPROM inicializada

#define MAX_BUS         20      // máximo de autocarros autorizados por paragem

/* RFID */
#define BLOCK_ADDR          4   // bloco MIFARE usado para guardar o ID do autocarro
#define RFID_POLL_NO_TAG_MS 30  // pausa entre tentativas quando não há tag
#define RFID_WAKEUP_MS      10  // tempo de arranque do módulo RC522 após power-up

/* LoRa */
#define LORA_CH         24      // canal LoRa fixo usado em todas as transmissões
#define LORA_FRAME_LEN  4       // tamanho fixo da trama: [BUS | STOP_H | STOP_L | DIR]

/* ACK */
#define ACK_TIMEOUT_MS  4000    // tempo máximo de espera por ACK (ms)
#define MAX_RETRIES     5       // número máximo de retransmissões antes de desistir

/* flag_irs: sinaliza (via ISR) que chegou uma trama LoRa completa */
volatile bool    flag_irs       = false;
volatile bool    lora_rx_enable = false;        // habilita processamento de bytes LoRa na ISR
volatile uint8_t lora_rx_buf[LORA_FRAME_LEN];   // buffer de receção da trama LoRa
volatile uint8_t lora_rx_idx    = 0;            // índice de escrita no buffer

/* Variáveis de estado do mecanismo ACK */
uint8_t  tx_payload[LORA_FRAME_LEN];    // trama a transmitir (guardada para retransmissão)
uint8_t  tx_expected_ack  = 0;          // ACK esperado: 0xF0 (frente) ou 0xF1 (trás)
uint8_t  tx_retry_count   = 0;          // contador de tentativas de retransmissão
uint8_t  tx_phase         = 0;          // fase de envio RFID: 0 = frente, 1 = trás
uint16_t tx_dest_mac      = 0;          // MAC de destino da transmissão atual
volatile uint8_t  ack_byte          = 0;        // byte ACK recebido pela ISR
volatile uint8_t  ack_pending_value = 0;        // valor do ACK a enviar (preparado na ISR)
volatile uint16_t ack_pending_dest  = 0;        // destino do ACK a enviar 
volatile uint16_t g_stop_id         = 0;        // ID desta paragem (acessível na ISR)
volatile bool flag_ack    = false;      // sinaliza receção de ACK pela ISR 
volatile bool waiting_ack = false;      // indica que o loop principal aguarda ACK 
volatile bool ack_pending = false;      // indica que há um ACK por enviar (preparado na ISR)

/* Tabela RAM: guarda a última paragem conhecida de cada autocarro (índice = ID autocarro) */
static uint16_t last_stop_by_bus[256];   // 512 bytes de RAM

/* Chave A padrão de fábrica MIFARE Classic */
static uint8_t keyA[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

/* Indica a origem do evento que desencadeou o ciclo de validação */
typedef enum {SRC_NONE=0, SRC_LORA, SRC_RFID} SRC;


/*
 * Escreve os valores padrão de teste na EEPROM.
 * Chamada quando o pino dev está ativo - útil para repor um nó rapidamente
 * sem precisar de configuração manual via UART.
 */
static void Config_WriteDefaultsToEEPROM(void)
{
    // ID da paragem
    EEPROM_Write(EEPROM_START_ADDRESS + EE_ID_STOP_H, (uint8_t)(DEFAULT_STOP_ID >> 8));
    while (NVM_IsBusy());

    EEPROM_Write(EEPROM_START_ADDRESS + EE_ID_STOP_L, (uint8_t)(DEFAULT_STOP_ID & 0xFF));
    while (NVM_IsBusy());

    // Lista de autocarros autorizados
    EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_COUNT, (uint8_t)DEFAULT_BUS_COUNT);
    while (NVM_IsBusy());
    for (uint8_t i = 0; i < DEFAULT_BUS_COUNT; i++)
    {
        EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_LIST + i, DEFAULT_BUSES[i]);
        while (NVM_IsBusy());
    }

    // Marca EEPROM como inicializada
    EEPROM_Write(EEPROM_START_ADDRESS + EE_MAGIC, EE_MAGIC_VALUE);
    while (NVM_IsBusy());
}

/* Limpa erro de overrun na UART1 (LoRa) */
void UART1_ClearErrors()
{
    uart1_status_t st;
    st.status = UART1_ErrorGet();
    if(st.oerr) 
    { 
        UART1_ReceiveDisable(); 
        UART1_ReceiveEnable(); 
    }
}

/* Limpa erro de overrun na UART2 (PC) */
void UART2_ClearErrors()
{
    uart2_status_t st;
    st.status = UART2_ErrorGet();
    if(st.oerr) 
    { 
        UART2_ReceiveDisable(); 
        UART2_ReceiveEnable(); 
    }
}

/* Converte o nibble baixo de v num caráter hex ('0'-'9' ou 'A'-'F') */
static char hex_digit(uint8_t v)
{
    v &= 0x0F;
    return (v < 10) ? ('0' + v) : ('A' + (v - 10));
}

/* Envia um uint16_t como 4 dígitos hex para o PC */
void UART_PrintHex16(uint16_t v)
{
    char s[5];
    s[0] = hex_digit((uint8_t)(v >> 12));
    s[1] = hex_digit((uint8_t)(v >> 8));
    s[2] = hex_digit((uint8_t)(v >> 4));
    s[3] = hex_digit((uint8_t)(v >> 0));
    s[4] = '\0';
    PC_WRITE_TEXT(s);
}

/* Envia 1 byte como 2 dígitos hex para o PC */
static void UART_WriteHex8(uint8_t v)
{
    char s[3];
    s[0] = hex_digit(v >> 4);
    s[1] = hex_digit(v);
    s[2] = '\0';
    PC_WRITE_TEXT(s);
}

/* Verifica se há uma tag RFID no campo do leitor.
 * Retorna true se o Request for bem-sucedido. */
static bool RC522_TagPresent(void)
{
    uint8_t tagType[2];
    return (MFRC522_Request(PICC_REQIDL, (char*)tagType) == MI_OK);
}

/*
 * Lê 16 bytes do bloco indicado para out16 (requer tag já detetada).
 * Não inclui o Request - deve ser precedida por RC522_TagPresent().
 */
static bool RC522_ReadBlock(uint8_t blockAddr, uint8_t *out16)
{
    uint8_t uid[5];
    char buf[16];   // buffer temporário para os dados brutos do RC522

    if (MFRC522_AntiColl((char*)uid) != MI_OK)  return false;
    if (MFRC522_SelectTag((char*)uid) == 0)     return false;
    if (MFRC522_Auth(PICC_AUTHENT1A, blockAddr, (char*)keyA, (char*)uid) != MI_OK) return false;
    if (MFRC522_Read(blockAddr, buf) != MI_OK)  return false;

    for (uint8_t i=0; i<16; i++) out16[i] = buf[i];
    MFRC522_Halt();
    return true;
}

/*
 * Lê a resposta do LoRa (UART1) e ecoa byte a byte para o PC (UART2).
 * Termina ao detetar: "Entry AT", "Exit AT", "OK", "ERROR=",
 * linha com '+' (resposta de query), ou ao fechar o bloco AT+HELP.
 * Abandona após 2 segundos sem resposta (timeout).
 */
void LoRa_ReadResponseToPC()
{
    char c;
    char line[60];
    uint8_t idx = 0;
    bool in_help = false;   // flag: estamos dentro do bloco AT+HELP?
    uint16_t timeout;

    UART1_ClearErrors();    // limpa LoRa RX
    
    while (1)
    {
        // Aguarda byte com timeout de 2s
        timeout = 0;
        while (!UART1_IsRxReady())
        {
            __delay_ms(1);
            if (++timeout > 2000)    // 2s sem resposta
            {
                PC_WRITE_TEXT("[Timeout - sem resposta do LoRa]\r\n");
                return;
            }
        }

        c = UART1_Read();

        while (!UART2_IsTxReady());
        UART2_Write(c);     // eco para o PC

        if (c == '\r' || c == '\n' || c == '\0')
        {
            if (idx == 0) continue;

            line[idx] = '\0';
            idx = 0;

            // separadores do HELP: 1º entra, 2º sai
            if (strcmp(line, "===================================") == 0)
            {
                if (!in_help) in_help = true;
                else break;
                continue;
            }

            // Dentro do HELP, continua a reencaminhar sem terminar
            if (in_help) continue;
            
            // Terminações fora do bloco HELP
            if (strcmp(line, "Entry AT")    == 0) break;
            if (strcmp(line, "Exit AT")     == 0) break;
            if (strcmp(line, "OK")          == 0) break;
            if (strncmp(line, "EEROR=", 6)  == 0) break;
            if (strncmp(line, "ERROR=", 6) == 0) break;
            if (line[0] == '+')                   break; // Respostas de query em 1 linha (ex: +MODE=1, +MAC=..)

            continue;
        }

        if (idx < (sizeof(line) - 1)) line[idx++] = c;
        else idx = 0;   // linha demasiado longa - descarta e recomeça
    }
}

/* Converte um caráter hex no seu valor numérico. 
 * Retorna -1 se inválido. */
static int8_t hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return (int8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (int8_t)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (int8_t)(c - 'A' + 10);
    return -1;
}

/* Converte 2 caracteres hex em 1 byte (ex: "1A" > 0x1A). 
 * Retorna false se inválido. */
static bool parse_hex_byte_2(const char *p, uint8_t *out)
{
    int8_t hi = hex_nibble(p[0]);
    int8_t lo = hex_nibble(p[1]);
    
    if (hi < 0 || lo < 0) return false;
    
    *out = (uint8_t)((hi << 4) | lo);
    return true;
}

/*
 * Extrai o ID de 16 bits de um comando "AT+MACXX,YY" para id_out (= 0xXXYY).
 * Retorna false se o formato não for reconhecido.
 */
static bool Parse_ID_FromSetCmd(const char *cmd, uint16_t *id_out)
{
    if (strncmp(cmd, "AT+MAC", 6) != 0) return false;

    const char *p = cmd + 6;   // aponta para os parâmetros "XX,YY"
    uint8_t b0 = 0, b1 = 0;

    if (p[2] != ',')                    return false;   // formato com vírgula: XX,YY
    if (!parse_hex_byte_2(p, &b0))      return false;   // byte alto  XX
    if (!parse_hex_byte_2(p + 3, &b1))  return false;   // byte baixo YY
    
    *id_out = (uint16_t)((uint16_t)b0 << 8) | b1;
    return true;
}

/* Retorna o número de autocarros autorizados guardados na EEPROM */
static uint8_t BusList_Count()
{
    uint8_t n = EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_COUNT);
    return (n > MAX_BUS) ? 0 : n;
}

/* Retorna true se o autocarro id existir na lista da EEPROM */
static bool BusList_Contains(uint8_t id)
{
    uint8_t n = BusList_Count();
    for (uint8_t i = 0; i < n; i++)
    {
        if (EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + i) == id)
            return true;
    }
    
    return false;
}

/* Adiciona um autocarro à lista. 
 * Retorna false se já existir ou a lista estiver cheia. */
static bool BusList_Add(uint8_t id)
{
    // já existe
    if (BusList_Contains(id)) 
    {
        PC_WRITE_TEXT("Autocarro ja registado\r\n");
        return false;
    }
    
    uint8_t n = BusList_Count();
    
    // lista cheia
    if (n >= MAX_BUS) 
    {
        PC_WRITE_TEXT("Lista cheia (max 20)\r\n");
        return false;
    }

    EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_LIST + n, id);
    
    while(NVM_IsBusy());
    n++;
    EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_COUNT, n);
    while(NVM_IsBusy());
    
    return true;
}

/*
 * Remove um autocarro da lista. 
 * Preenche o espaço vazio com o último elemento(compactação sem preservar ordem). 
 * Retorna false se não encontrado.
 */
static bool BusList_Remove(uint8_t id)
{
    uint8_t n = BusList_Count();
    for (uint8_t i = 0; i < n; i++)
    {
        if (EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + i) == id)
        {
            // move o último elemento para a posição removida
            uint8_t last = EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + n - 1);
            
            EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_LIST + i, last);
            while (NVM_IsBusy()) {}
            n--;
            EEPROM_Write(EEPROM_START_ADDRESS + EE_BUS_COUNT, n);
            while (NVM_IsBusy()) {}
            
            return true;
        }
    }
    return false;
}

static bool Bus_StopChanged(uint8_t id_bus, uint16_t new_stop)
{
    return (last_stop_by_bus[id_bus] != new_stop);
}

static void Bus_UpdateStop(uint8_t id_bus, uint16_t new_stop)
{
    last_stop_by_bus[id_bus] = new_stop;
}

static void BusTable_InitAuthorizedOnly()
{
    uint8_t n = BusList_Count();

    for (uint8_t i = 0; i < n; i++)
    {
        uint8_t id = EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + i);
        last_stop_by_bus[id] = 0xFFFF;
    }
}

/* ISR do TMR0: descarta uma trama incompleta se o timer expirar sem receber todos os bytes */
void Lora_RxTimeout_ISR()
{
    if (lora_rx_idx > 0 && lora_rx_idx < LORA_FRAME_LEN)
        lora_rx_idx = 0;    // trama incompleta - descarta

    TMR0_Stop();
    TMR0_CounterSet(0);
}

/*
 * ISR de receção LoRa (UART1): acumula bytes até completar uma trama de 4 bytes.
 * - Se estiver à espera de ACK e chegar 0xF0 ou 0xF1, processa como ACK.
 * - Caso contrário, acumula no buffer e usa o TMR0 para detetar fim de trama.
 * - Quando a trama fica completa, prepara o ACK a devolver e ativa flag_irs.
 */
static void Lora_Recebeu()
{
    uint8_t b;

    if (!lora_rx_enable) return;
    UART1_ClearErrors();
    b = UART1_Read();
    
    // byte de ACK - processa imediatamente sem acumular no buffer de trama
    if (waiting_ack && (b == 0xF0 || b == 0xF1))
    {
        ack_byte = b;
        flag_ack = true;
        return;
    }       

    if (flag_irs) return;   // trama anterior ainda não processada - descarta

    // índice fora dos limites - reinicia
    if (lora_rx_idx >= LORA_FRAME_LEN)
    {
        lora_rx_idx = 0;
        TMR0_Stop();
        TMR0_CounterSet(0);
        return;
    }

    lora_rx_buf[lora_rx_idx++] = b;

    // reinicia o timer de silêncio a cada byte recebido
    TMR0_Stop();
    TMR0_CounterSet(0);
    TMR0_Start();

    if (lora_rx_idx == LORA_FRAME_LEN)
    {
        uint8_t rx_dir = lora_rx_buf[3];

        /* Prepara ACK conforme a direção da trama recebida:
         * DIR=0x00 (vem da frente) > ACK para MAC-1
         * DIR=0x01 (vem de trás)   > ACK para MAC+1 */
        if (rx_dir == 0x00 && g_stop_id > 0)
        {
            ack_pending_dest  = g_stop_id - 1;
            ack_pending_value = 0xF0;
            ack_pending = true;
            flag_irs = true;
        }
        else if (rx_dir == 0x01)
        {
            ack_pending_dest  = g_stop_id + 1;
            ack_pending_value = 0xF1;
            ack_pending = true;
            flag_irs = true;
        }
        else
        {
            // direção inválida - descarta trama
            lora_rx_idx = 0;
            TMR0_Stop();
            TMR0_CounterSet(0);
            return;
        }

        TMR0_Stop();
        TMR0_CounterSet(0);
        lora_rx_idx = 0;
    }
}

/* Envia uma trama LoRa em fixed-point: DEST_H + DEST_L + CH + DATA[len] */
static void Lora_Send(uint16_t dest_mac, uint8_t ch, const uint8_t *data, uint8_t len)
{
    // DEST (2 bytes)
    while (!UART1_IsTxReady());
    UART1_Write((dest_mac >> 8));   // endereço alto
    while (!UART1_IsTxReady());     
    UART1_Write((dest_mac & 0xFF)); // endereço baixo

    // Canal (1 byte)
    while (!UART1_IsTxReady());
    UART1_Write(ch);

    // DATA (len bytes)
    for (uint8_t i = 0; i < len; i++)
    {
        while (!UART1_IsTxReady());
        UART1_Write(data[i]);
    }
}

/* Imprime no PC o conteúdo de uma trama LoRa antes de a enviar */
static void LoRa_PrintTxFrame(const char *label, uint16_t dest_mac, uint8_t ch, const uint8_t *data, uint8_t len)
{
    PC_WRITE_TEXT(label);

    PC_WRITE_TEXT(" | DEST=0x");
    UART_PrintHex16(dest_mac);

    PC_WRITE_TEXT(" | CH=0x");
    UART_WriteHex8(ch);

    PC_WRITE_TEXT(" | DATA=");
    for (uint8_t i = 0; i < len; i++)
    {
        UART_WriteHex8(data[i]);
        PC_WRITE_TEXT(" ");
    }

    PC_WRITE_TEXT("\r\n");
}

/* Imprime no PC o conteúdo de uma trama LoRa recebida */
static void LoRa_PrintRxFrame(const char *label, const uint8_t *data, uint8_t len)
{
    PC_WRITE_TEXT(label);
    PC_WRITE_TEXT(" | DATA=");

    for (uint8_t i = 0; i < len; i++)
    {
        UART_WriteHex8(data[i]);
        PC_WRITE_TEXT(" ");
    }

    PC_WRITE_TEXT("\r\n");
}

/* Imprime no PC um evento de envio de ACK: destino e valor */
static void LoRa_PrintAckEvent(const char *label, uint16_t dest_mac, uint8_t ack)
{
    PC_WRITE_TEXT(label);

    PC_WRITE_TEXT(" | DEST=0x");
    UART_PrintHex16(dest_mac);

    PC_WRITE_TEXT(" | ACK=0x");
    UART_WriteHex8(ack);

    PC_WRITE_TEXT("\r\n");
}

/* Imprime no PC um ACK recebido */
static void LoRa_PrintAckRx(const char *label, uint8_t ack)
{
    PC_WRITE_TEXT(label);

    PC_WRITE_TEXT(" | ACK=0x");
    UART_WriteHex8(ack);

    PC_WRITE_TEXT("\r\n");
}

/* Envia uma trama de teste fixa para o MAC 0x0003 */
static void LoRa_Send_TestCompat()
{
    uint8_t data[4] = {0x01, 0x00, 0x01, 0x00};
//                  BUS   STOP_H STOP_L  DIR
//                  bus1  paragem 0x0001  frente

    while(aux_GetValue() == 1);
    Lora_Send(0x0003, 24, data, 4);
    while (!UART1_IsTxDone());
    __delay_ms(10);
    while(aux_GetValue() == 1);
    LoRa_PrintTxFrame("TESTE COMPAT", 0x0003, 24, data, 4);
}

/*
 * Envia o ACK preparado pela ISR, se existir.
 * Chamada periodicamente durante ST_WAIT_ACK para não bloquear o envio
 * de ACKs a outros nós enquanto se aguarda o próprio ACK.
 */
static void Send_PendingAck_IfAny(void)
{
    uint16_t dest;
    uint8_t ack;

    if (!ack_pending) return;
    
    // Copia e limpa as variáveis voláteis com interrupções desligadas
    INTERRUPT_GlobalInterruptDisable();
    dest = ack_pending_dest;
    ack  = ack_pending_value;
    ack_pending = false;
    INTERRUPT_GlobalInterruptEnable();

    while (aux_GetValue() == 1);
    Lora_Send(dest, LORA_CH, &ack, 1);
    while (!UART1_IsTxDone());
    __delay_ms(10);
    while (aux_GetValue() == 1);

    LoRa_PrintAckEvent("* ACK enviado", dest, ack);
}

/*
 * Atualiza os LEDs de indicação de posição dos autocarros.
 * Apaga todos e acende o LED correspondente à paragem atual de cada autocarro autorizado.
 * LED1=paragem 1, LED2=paragem 2, LED3=paragem 3, LED4=paragem 4.
 */ 
static void LED_RefreshAll(void)
{
    led1_SetLow();
    led2_SetLow();
    led3_SetLow();
    led4_SetLow();

    // Percorre a lista dos autocarros autorizados
    uint8_t n = BusList_Count();
    for (uint8_t i = 0; i < n; i++)
    {
        uint8_t bus = EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + i);
        uint16_t stop = last_stop_by_bus[bus];

        switch (stop)
        {
            case 0x0001: led1_SetHigh(); break;
            case 0x0002: led2_SetHigh(); break;
            case 0x0003: led3_SetHigh(); break;
            case 0x0004: led4_SetHigh(); break;
            default: break;
        }
    }
}


int main()
{
    SYSTEM_Initialize();
    NVM_Initialize();
    INTERRUPT_Initialize();
    
    SPI1_Open(HOST_CONFIG);
    __delay_ms(10);
    MFRC522_Init();
    __delay_ms(10);
    
    NVM_UnlockKeySet(UNLOCK_KEY);   // UNLOCK_KEY = 0xAA55 
    
    // Regista ISR do TMR0 para detetar tramas LoRa incompletas por timeout
    TMR0_OverflowCallbackRegister(Lora_RxTimeout_ISR);
    TMR0_Stop();
    TMR0_CounterSet(0);
    
    UART1_RxCompleteCallbackRegister(Lora_Recebeu); // regista ISR de receção LoRa
    INTERRUPT_GlobalInterruptEnable();

    MFRC522_SoftPowerDown();    // módulo RFID começa em modo de baixo consumo
    
    // Se o pino dev estiver ativo, repõe os valores padrão na EEPROM
    if(dev_GetValue()==1) Config_WriteDefaultsToEEPROM();
    
    // Inicializa tabela RAM de paragens com 0xFFFF
    BusTable_InitAuthorizedOnly();
    
    // Lê o ID/MAC desta paragem guardado na EEPROM
    g_stop_id = ((uint16_t)EEPROM_Read(EEPROM_START_ADDRESS + EE_ID_STOP_H) << 8) |
            (uint16_t)EEPROM_Read(EEPROM_START_ADDRESS + EE_ID_STOP_L);
    uint16_t stop_id = g_stop_id;
    
    // Variáveis de estado partilhadas entre os estados da máquina
    SRC src = SRC_NONE;
    uint8_t id_bus = 0;
    uint16_t id_stop = 0;
    uint8_t dir = 0;        // direção da trama: 0x00 = frente, 0x01 = trás

    uint8_t rfid_data[16];  // recebe os 16 bytes lidos do bloco RFID
    
    // Variáveis do estado ST_CONFIG_LORA
    uint16_t id_cfg_lora;
    char comando_cfg_lora[30];
    uint8_t idx_cfg_lora;
    
    // Variáveis do estado ST_CONFIG_STOP 
    char cmd_cfg_stop[10];
    uint8_t idx_cfg_stop;
    uint8_t val_cfg_stop;
    
    // Trama LoRa e endereços dos nós vizinhos
    uint8_t lora_frame[LORA_FRAME_LEN];
    uint16_t mac_front = 0;     // MAC da paragem seguinte (stop_id + 1)
    uint16_t mac_back  = 0;     // MAC da paragem anterior (stop_id - 1)
    
    // Variáveis locais do mecanismo ACK
    uint16_t dest;
    uint8_t ack;
       
    __delay_ms(500);
    LORA_DRAIN_RX();
    
    /* =====================================================================
     * MÁQUINA DE ESTADOS
     * ===================================================================== */
    ST_IDLE:
        // ACK e LoRa têm prioridade sobre RFID e modos de configuração
        if (ack_pending) goto ST_ACK_SEND_PENDING;
        if (flag_irs)    goto ST_LORA_COPY;

        if (lora_config_GetValue() != 0) goto ST_CONFIG_LORA;
        if (stop_config_GetValue() != 0) goto ST_CONFIG_STOP;
        if (test_lora_mode_GetValue() != 0)
        {
            LoRa_Send_TestCompat();
            goto ST_IDLE;
        }

        lora_rx_enable = true;
        goto ST_RFID_CHECK;
        
    /* == Configuração do LoRa com comandos AT ============================= */ 
    ST_CONFIG_LORA:
        lora_rx_enable = false;
        flag_irs = false;
        lora_rx_idx = 0;
        TMR0_Stop();    
        TMR0_CounterSet(0);
        
        PC_WRITE_TEXT("* Modo config LoRa *\r\n");
        PC_DRAIN_RX();
        __delay_ms(500);
        LORA_DRAIN_RX();
      
        // Entra no modo AT com a sequência "+++"
        PC_WRITE_TEXT("A entrar no modo AT...\r\n");
        __delay_ms(250);
        LORA_WRITE_TEXT("+++\r\n");
        __delay_ms(250);
        LoRa_ReadResponseToPC();
        
        PC_WRITE_TEXT("Modo AT ativado\r\n");
        PC_WRITE_TEXT("A testar modulo...\r\n");
        
        LORA_WRITE_TEXT("AT\r\n");
        LoRa_ReadResponseToPC();
        PC_WRITE_TEXT("\r\n\r\n");
        
        PC_WRITE_TEXT(">> Escrever comandos AT\r\n");
        PC_DRAIN_RX();
        
        INICIO:
        while (1)
        {
            // Lê comando do PC (UART2)
            UART2_ClearErrors();
            idx_cfg_lora = 0;
            memset(comando_cfg_lora, 0, sizeof(comando_cfg_lora));

            while (1)
            {
                if (UART2_IsRxReady())
                {
                    char c = (char)UART2_Read();

                    if (c == '\r' || c == '\n')
                    {
                        if (idx_cfg_lora > 0) break;     // linha completa
                    }
                    else if (idx_cfg_lora < (sizeof(comando_cfg_lora) - 1))
                    {
                        comando_cfg_lora[idx_cfg_lora++] = c;
                    }
                }
            }
            __delay_ms(10);

            // Acrescenta "\r\n" ao comando antes de enviar para o LoRa (UART1)
            LORA_DRAIN_RX();
            if (idx_cfg_lora <= (sizeof(comando_cfg_lora) - 3))
            {
                comando_cfg_lora[idx_cfg_lora++] = '\r';
                comando_cfg_lora[idx_cfg_lora++] = '\n';
                comando_cfg_lora[idx_cfg_lora]   = '\0';
            }
            else
            {
                // Buffer quase cheio - coloca terminador no fim de forma segura
                comando_cfg_lora[sizeof(comando_cfg_lora) - 3] = '\r';
                comando_cfg_lora[sizeof(comando_cfg_lora) - 2] = '\n';
                comando_cfg_lora[sizeof(comando_cfg_lora) - 1] = '\0';
            }

            // Evia comando para Lora e faz eco no PC
            LORA_WRITE_TEXT(comando_cfg_lora);
            PC_WRITE_TEXT(comando_cfg_lora);

            // Lê resposta completa do LoRa e reencaminha para o PC
            LoRa_ReadResponseToPC();
            PC_WRITE_TEXT("\r\n\r\n");
            PC_DRAIN_RX();
            LORA_DRAIN_RX();

            // AT+RESET: aguarda boot do módulo e reinicia a máquina de estados
            if (strcmp(comando_cfg_lora, "AT+RESET\r\n") == 0)
            {
                for (uint8_t i = 0; i < 50; i++) __delay_ms(100);   // 5s

                flag_irs = false;
                lora_rx_idx = 0;
                TMR0_Stop();
                TMR0_CounterSet(0);
                LORA_DRAIN_RX();
                
                goto ST_IDLE;
            }

            // AT+MACXX,YY: guarda o ID na EEPROM e atualiza stop_id em RAM
            if(Parse_ID_FromSetCmd(comando_cfg_lora, &id_cfg_lora))
            {
                NVM_StatusClear();

                // high byte
                EEPROM_Write((eeprom_address_t)(EEPROM_START_ADDRESS + EE_ID_STOP_H), (eeprom_data_t)(id_cfg_lora >> 8));
                while (NVM_IsBusy());
                if (NVM_StatusGet() != NVM_OK) PC_WRITE_TEXT("Erro a escrever EEPROM (high)\r\n");

                // low byte
                EEPROM_Write((eeprom_address_t)(EEPROM_START_ADDRESS + EE_ID_STOP_L), (eeprom_data_t)(id_cfg_lora & 0xFF));
                while (NVM_IsBusy());
                if (NVM_StatusGet() != NVM_OK) PC_WRITE_TEXT("Erro a escrever EEPROM (low)\r\n");

                PC_WRITE_TEXT("ID guardado na EEPROM: 0x");
                UART_PrintHex16(id_cfg_lora);
                PC_WRITE_TEXT("\r\n\r\n");
                
                stop_id = id_cfg_lora;
                g_stop_id = id_cfg_lora;

                goto INICIO;
            }
        }
        
    /* == Gestão da lista de autocarros autorizados ======================== */    
    ST_CONFIG_STOP:
        lora_rx_enable = false;
        flag_irs = false;
        lora_rx_idx = 0;
        TMR0_Stop();
        TMR0_CounterSet(0);
        
        PC_WRITE_TEXT("* Modo config Paragem *\r\n");
        PC_WRITE_TEXT("Comandos: BUS+XX | BUS-XX | BUS? | EXIT || ");
        PC_WRITE_TEXT("(XX - em hex)\r\n");
        PC_WRITE_TEXT("ID_Stop: 0x");
        UART_PrintHex16(stop_id);
        PC_WRITE_TEXT("\r\n\r\n");
        PC_DRAIN_RX();
        
        while (1)
        {
            UART2_ClearErrors();
            idx_cfg_stop = 0;
            memset(cmd_cfg_stop, 0, sizeof(cmd_cfg_stop));

            while (1)
            {
                // Lê comando
                if (UART2_IsRxReady())
                {
                    char c = UART2_Read();
                    if (c == '\r' || c == '\n')
                    {
                        if (idx_cfg_stop > 0) 
                            break;
                    }
                    
                    else if(idx_cfg_stop < sizeof(cmd_cfg_stop) - 1) 
                        cmd_cfg_stop[idx_cfg_stop++] = c;
                }
            }
            
            // eco do comando
            PC_WRITE_TEXT(cmd_cfg_stop);
            PC_WRITE_TEXT("\r\n");
                        
            // BUS+XX -> adiciona autocarro XX (hex)
            if (strncmp(cmd_cfg_stop, "BUS+", 4) == 0 && idx_cfg_stop >= 6)
            {
                if (parse_hex_byte_2(cmd_cfg_stop + 4, &val_cfg_stop))
                {
                    if (BusList_Add(val_cfg_stop)) 
                    {
                        PC_WRITE_TEXT("Adicionado: 0x");
                        UART_WriteHex8(val_cfg_stop);
                        PC_WRITE_TEXT("\r\n");
                        last_stop_by_bus[val_cfg_stop] = 0xFFFF;
                    }
                }
                else PC_WRITE_TEXT("ID invalido\r\n");
            }

            // BUS-XX -> remove autocarro XX (hex)
            else if (strncmp(cmd_cfg_stop, "BUS-", 4) == 0 && idx_cfg_stop >= 6)
            {
                if (parse_hex_byte_2(cmd_cfg_stop + 4, &val_cfg_stop))
                {
                    if (BusList_Remove(val_cfg_stop)) 
                    {
                        PC_WRITE_TEXT("Removido: 0x");
                        UART_WriteHex8(val_cfg_stop);
                        PC_WRITE_TEXT("\r\n");
                        last_stop_by_bus[val_cfg_stop] = 0xFFFF;
                    } 
                    else PC_WRITE_TEXT("Autocarro nao encontrado\r\n");
                }
                else PC_WRITE_TEXT("ID invalido\r\n");
            }

            // BUS? -> lista todos os autocarros autorizados
            else if (strcmp(cmd_cfg_stop, "BUS?") == 0)
            {
                uint8_t n = BusList_Count();
                PC_WRITE_TEXT("================\r\n");
                PC_WRITE_TEXT("Autorizados (");
                UART_WriteHex8(n);
                PC_WRITE_TEXT("): \r\n");
                for (uint8_t i = 0; i < n; i++) 
                {
                    PC_WRITE_TEXT("0x");
                    UART_WriteHex8(EEPROM_Read(EEPROM_START_ADDRESS + EE_BUS_LIST + i));
                    PC_WRITE_TEXT("\r\n");
                }
                
                PC_WRITE_TEXT("================\r\n");
                PC_WRITE_TEXT("\r\n");
            }

            // EXIT -> aguarda 5s e volta ao loop principal
            else if (strcmp(cmd_cfg_stop, "EXIT") == 0)
            {
                PC_WRITE_TEXT("A sair...\r\n\r\n");
                
                flag_irs = false;
                lora_rx_idx = 0;
                TMR0_Stop();
                TMR0_CounterSet(0);
                
                for (uint8_t i = 0; i < 50; i++) __delay_ms(100);  // 5 s
                
                goto ST_IDLE;
            }

            else PC_WRITE_TEXT("Comando desconhecido\r\n");
            
            PC_WRITE_TEXT("\r\n");
        }

    /* == Envia o ACK que a ISR preparou, depois processa a trama recebida = */   
    ST_ACK_SEND_PENDING:
        // Copia e limpa as variáveis voláteis com interrupções desligadas
        INTERRUPT_GlobalInterruptDisable();
        dest = ack_pending_dest;
        ack  = ack_pending_value;
        ack_pending = false;
        INTERRUPT_GlobalInterruptEnable();
                    
        PC_WRITE_TEXT("* A enviar ACK...\r\n");
        while (aux_GetValue() == 1);
        Lora_Send(dest, LORA_CH, &ack, 1);
        while (!UART1_IsTxDone());
        __delay_ms(15);
        while(aux_GetValue() == 1);
        
        LoRa_PrintAckEvent("* ACK enviado", dest, ack);
        goto ST_LORA_COPY;
    
    /* == Copia a trama LoRa do buffer volátil para variáveis locais ======= */
    ST_LORA_COPY:
        INTERRUPT_GlobalInterruptDisable();
        
        // copia trama do buffer
        for (uint8_t i = 0; i < LORA_FRAME_LEN; i++)
            lora_frame[i] = lora_rx_buf[i];

        flag_irs = false;
        lora_rx_idx = 0;
        TMR0_Stop();
        TMR0_CounterSet(0);

        INTERRUPT_GlobalInterruptEnable();

        src = SRC_LORA;

        id_bus  = lora_frame[0];
        id_stop = ((uint16_t)lora_frame[1] << 8) | lora_frame[2];
        dir     = lora_frame[3];
                
        goto ST_VALIDATE;

    /* == Verifica presença de tag e lê o ID do autocarro ================== */   
    ST_RFID_CHECK:
        tx_phase = 0;   // reinicia a fase de envio para o caso RFID
    
        MFRC522_SoftPowerUp();
        __delay_ms(RFID_WAKEUP_MS);
        
        // Primeiro verifica se há tag
        if (!RC522_TagPresent())
        {
            MFRC522_SoftPowerDown();
            __delay_ms(RFID_POLL_NO_TAG_MS);
            goto ST_IDLE;
        }
        
        // Se houver tag é que faz anticollision + auth + read
        if (!RC522_ReadBlock(BLOCK_ADDR, rfid_data))
        {
            MFRC522_SoftPowerDown();
            __delay_ms(RFID_POLL_NO_TAG_MS);
            goto ST_IDLE;
        }

        id_bus = rfid_data[0];  // apenas o primeiro byte interessa - o ID do autocarro
        
        MFRC522_SoftPowerDown();
        __delay_ms(RFID_POLL_NO_TAG_MS);

        // Autocarro não autorizado
        if (!BusList_Contains(id_bus)) goto ST_IDLE;

        id_stop = stop_id;
        src = SRC_RFID;
        goto ST_VALIDATE;
        
    /* == Valida se o evento deve gerar uma transmissão ==================== */
    ST_VALIDATE:
        if (src == SRC_LORA)
        {
            PC_WRITE_TEXT("* VALIDATE LORA src | dir=0x");
            UART_WriteHex8(dir);
            PC_WRITE_TEXT(" stop=0x");
            UART_PrintHex16(id_stop);
            PC_WRITE_TEXT(" bus=0x");
            UART_WriteHex8(id_bus);
            PC_WRITE_TEXT("\r\n");

            if (dir != 0x00 && dir != 0x01) goto ST_CLEANUP;    // direção inválida
            if (id_stop == 0)               goto ST_CLEANUP;    // ID de paragem inválido
            if (!BusList_Contains(id_bus))  goto ST_CLEANUP;    // autocarro não autorizado

            LoRa_PrintRxFrame("* DATA RX", lora_frame, LORA_FRAME_LEN);
        }
    
        if (!Bus_StopChanged(id_bus, id_stop))
        {
            PC_WRITE_TEXT("* Stop nao mudou - ignora\r\n");
            goto ST_CLEANUP; // paragem já registada - ignora
        } 

        goto ST_UPDATE;

    /* == Atualiza a tabela RAM e os LEDs de indicação ===================== */
    ST_UPDATE:
        Bus_UpdateStop(id_bus, id_stop);
        LED_RefreshAll();       // atualiza todos os LEDs com as posições atuais
        goto ST_TX_PREP;

    /* == Prepara a trama e o destino para transmissão ===================== */
    ST_TX_PREP:
        tx_retry_count = 0;
    
        mac_front = stop_id + 1;
        if (stop_id > 1) mac_back = stop_id - 1;

        if (src == SRC_RFID)
        {
            // 1º envio: tras
            if (tx_phase == 0)
            {
                // paragem 1 não tem vizinho atrás
                if (stop_id == 1){
                    tx_phase = 1;
                    goto ST_TX_PREP;  
                }

                tx_payload[0] = id_bus;
                tx_payload[1] = (uint8_t)(stop_id >> 8);
                tx_payload[2] = (uint8_t)(stop_id & 0xFF);
                tx_payload[3] = 0x01;       // direção: trás

                tx_dest_mac = mac_back;
                tx_expected_ack = 0xF1;
                __delay_ms(10);
                
                goto ST_TX_SEND;
            }
            
            // 2º envio: frente
            if (tx_phase == 1)
            {
                tx_payload[0] = id_bus;
                tx_payload[1] = (uint8_t)(stop_id >> 8);
                tx_payload[2] = (uint8_t)(stop_id & 0xFF);
                tx_payload[3] = 0x00;       // direção: frente

                tx_dest_mac = mac_front;
                tx_expected_ack = 0xF0;
                __delay_ms(10);
                    
                goto ST_TX_SEND;
            }

            goto ST_CLEANUP;
        }

        if (src == SRC_LORA)
        {
            // Reencaminha na mesma direção da trama recebida
            tx_payload[0] = id_bus;
            tx_payload[1] = (uint8_t)(id_stop >> 8);
            tx_payload[2] = (uint8_t)(id_stop & 0xFF);

            if (dir == 0x00)
            {
                tx_payload[3] = 0x00;
                tx_dest_mac = mac_front;
                tx_expected_ack = 0xF0;
                __delay_ms(10);
                
                goto ST_TX_SEND;
            }

            if (dir == 0x01)
            {
                if (stop_id == 1) goto ST_CLEANUP;

                tx_payload[3] = 0x01;
                tx_dest_mac = mac_back;
                tx_expected_ack = 0xF1;
                __delay_ms(10);
                
                goto ST_TX_SEND;
            }
        }
        goto ST_CLEANUP;

    /* == Envia a trama e ativa a espera por ACK =========================== */
    ST_TX_SEND:
        flag_ack = false;
        ack_byte = 0;
        waiting_ack = true; // sinaliza à ISR que o próximo 0xF0/0xF1 é um ACK
        
        while(aux_GetValue() == 1);
        Lora_Send(tx_dest_mac, LORA_CH, tx_payload, LORA_FRAME_LEN);
        while (!UART1_IsTxDone());
        __delay_ms(20);
        while(aux_GetValue() == 1);
        
        LoRa_PrintTxFrame("* DATA TX", tx_dest_mac, LORA_CH, tx_payload, LORA_FRAME_LEN);
        goto ST_WAIT_ACK;

    /* == Aguarda ACK com timeout de ACK_TIMEOUT_MS ms ==================== */
    ST_WAIT_ACK:
        for (uint16_t t = 0; t < ACK_TIMEOUT_MS; t++)
        {
            if (flag_ack)
            {
                if (ack_byte == tx_expected_ack)
                {
                    LoRa_PrintAckRx("* ACK recebido", ack_byte);
                    PC_WRITE_TEXT("******************************************************\r\n\r\n");

                    waiting_ack = false;
                    flag_ack = false;
                    ack_byte = 0;
                    goto ST_TX_NEXT;
                }

                // ACK recebido mas não é o esperado - continua à espera
                flag_ack = false;
                ack_byte = 0;
            }
            Send_PendingAck_IfAny();    // responde a ACKs de outros nós durante a espera
            __delay_ms(1);
        }

        // timeout atingido sem receber ACK
        waiting_ack = false;
        PC_WRITE_TEXT("Timeout ACK\r\n");
        PC_WRITE_TEXT("\r\n");
        
        goto ST_TX_RETRY;

    /* == Retransmite até MAX_RETRIES tentativas =========================== */
    ST_TX_RETRY:
        waiting_ack = false;
        tx_retry_count++;
        
        PC_WRITE_TEXT("Retry #");
        UART_WriteHex8(tx_retry_count);
        PC_WRITE_TEXT("\r\n\r\n");

        if (tx_retry_count < MAX_RETRIES)
        {
            __delay_ms(10);
            goto ST_TX_SEND;
        }

        PC_WRITE_TEXT("Falha apos max retries\r\n");
        PC_WRITE_TEXT("******************************************************\r\n\r\n");
        
        // Se veio do RFID, mesmo falhando um sentido, tenta o outro
        if (src == SRC_RFID) goto ST_TX_NEXT;
        
        // Se veio do LoRa, só havia uma direção
        goto ST_CLEANUP;

    /* == Avança para o próximo envio ou termina o ciclo ================== */
    ST_TX_NEXT:
        waiting_ack = false;

        if (src == SRC_RFID)
        {
            if (tx_phase == 0)
            {
                tx_phase = 1;   // passa ao envio para a frente
                goto ST_TX_PREP;
            }

            tx_phase = 0;
            goto ST_CLEANUP;
        }
        goto ST_CLEANUP;

    /* == Limpa a variável src e volta ao início do loop =================== */
    ST_CLEANUP:
        src = SRC_NONE;
        goto ST_IDLE;
}