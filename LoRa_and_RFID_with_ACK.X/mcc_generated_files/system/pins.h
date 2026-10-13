/**
 * Generated Pins header File
 * 
 * @file pins.h
 * 
 * @defgroup  pinsdriver Pins Driver
 * 
 * @brief This is generated driver header for pins. 
 *        This header file provides APIs for all pins selected in the GUI.
 *
 * @version Driver Version  3.1.1
*/

/*
© [2026] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#ifndef PINS_H
#define PINS_H

#include <xc.h>

#define INPUT   1
#define OUTPUT  0

#define HIGH    1
#define LOW     0

#define ANALOG      1
#define DIGITAL     0

#define PULL_UP_ENABLED      1
#define PULL_UP_DISABLED     0

// get/set RA0 aliases
#define lora_config_TRIS                 TRISAbits.TRISA0
#define lora_config_LAT                  LATAbits.LATA0
#define lora_config_PORT                 PORTAbits.RA0
#define lora_config_WPU                  WPUAbits.WPUA0
#define lora_config_OD                   ODCONAbits.ODCA0
#define lora_config_ANS                  ANSELAbits.ANSELA0
#define lora_config_SetHigh()            do { LATAbits.LATA0 = 1; } while(0)
#define lora_config_SetLow()             do { LATAbits.LATA0 = 0; } while(0)
#define lora_config_Toggle()             do { LATAbits.LATA0 = ~LATAbits.LATA0; } while(0)
#define lora_config_GetValue()           PORTAbits.RA0
#define lora_config_SetDigitalInput()    do { TRISAbits.TRISA0 = 1; } while(0)
#define lora_config_SetDigitalOutput()   do { TRISAbits.TRISA0 = 0; } while(0)
#define lora_config_SetPullup()          do { WPUAbits.WPUA0 = 1; } while(0)
#define lora_config_ResetPullup()        do { WPUAbits.WPUA0 = 0; } while(0)
#define lora_config_SetPushPull()        do { ODCONAbits.ODCA0 = 0; } while(0)
#define lora_config_SetOpenDrain()       do { ODCONAbits.ODCA0 = 1; } while(0)
#define lora_config_SetAnalogMode()      do { ANSELAbits.ANSELA0 = 1; } while(0)
#define lora_config_SetDigitalMode()     do { ANSELAbits.ANSELA0 = 0; } while(0)

// get/set RA1 aliases
#define stop_config_TRIS                 TRISAbits.TRISA1
#define stop_config_LAT                  LATAbits.LATA1
#define stop_config_PORT                 PORTAbits.RA1
#define stop_config_WPU                  WPUAbits.WPUA1
#define stop_config_OD                   ODCONAbits.ODCA1
#define stop_config_ANS                  ANSELAbits.ANSELA1
#define stop_config_SetHigh()            do { LATAbits.LATA1 = 1; } while(0)
#define stop_config_SetLow()             do { LATAbits.LATA1 = 0; } while(0)
#define stop_config_Toggle()             do { LATAbits.LATA1 = ~LATAbits.LATA1; } while(0)
#define stop_config_GetValue()           PORTAbits.RA1
#define stop_config_SetDigitalInput()    do { TRISAbits.TRISA1 = 1; } while(0)
#define stop_config_SetDigitalOutput()   do { TRISAbits.TRISA1 = 0; } while(0)
#define stop_config_SetPullup()          do { WPUAbits.WPUA1 = 1; } while(0)
#define stop_config_ResetPullup()        do { WPUAbits.WPUA1 = 0; } while(0)
#define stop_config_SetPushPull()        do { ODCONAbits.ODCA1 = 0; } while(0)
#define stop_config_SetOpenDrain()       do { ODCONAbits.ODCA1 = 1; } while(0)
#define stop_config_SetAnalogMode()      do { ANSELAbits.ANSELA1 = 1; } while(0)
#define stop_config_SetDigitalMode()     do { ANSELAbits.ANSELA1 = 0; } while(0)

// get/set RA2 aliases
#define test_lora_mode_TRIS                 TRISAbits.TRISA2
#define test_lora_mode_LAT                  LATAbits.LATA2
#define test_lora_mode_PORT                 PORTAbits.RA2
#define test_lora_mode_WPU                  WPUAbits.WPUA2
#define test_lora_mode_OD                   ODCONAbits.ODCA2
#define test_lora_mode_ANS                  ANSELAbits.ANSELA2
#define test_lora_mode_SetHigh()            do { LATAbits.LATA2 = 1; } while(0)
#define test_lora_mode_SetLow()             do { LATAbits.LATA2 = 0; } while(0)
#define test_lora_mode_Toggle()             do { LATAbits.LATA2 = ~LATAbits.LATA2; } while(0)
#define test_lora_mode_GetValue()           PORTAbits.RA2
#define test_lora_mode_SetDigitalInput()    do { TRISAbits.TRISA2 = 1; } while(0)
#define test_lora_mode_SetDigitalOutput()   do { TRISAbits.TRISA2 = 0; } while(0)
#define test_lora_mode_SetPullup()          do { WPUAbits.WPUA2 = 1; } while(0)
#define test_lora_mode_ResetPullup()        do { WPUAbits.WPUA2 = 0; } while(0)
#define test_lora_mode_SetPushPull()        do { ODCONAbits.ODCA2 = 0; } while(0)
#define test_lora_mode_SetOpenDrain()       do { ODCONAbits.ODCA2 = 1; } while(0)
#define test_lora_mode_SetAnalogMode()      do { ANSELAbits.ANSELA2 = 1; } while(0)
#define test_lora_mode_SetDigitalMode()     do { ANSELAbits.ANSELA2 = 0; } while(0)

// get/set RA3 aliases
#define dev_TRIS                 TRISAbits.TRISA3
#define dev_LAT                  LATAbits.LATA3
#define dev_PORT                 PORTAbits.RA3
#define dev_WPU                  WPUAbits.WPUA3
#define dev_OD                   ODCONAbits.ODCA3
#define dev_ANS                  ANSELAbits.ANSELA3
#define dev_SetHigh()            do { LATAbits.LATA3 = 1; } while(0)
#define dev_SetLow()             do { LATAbits.LATA3 = 0; } while(0)
#define dev_Toggle()             do { LATAbits.LATA3 = ~LATAbits.LATA3; } while(0)
#define dev_GetValue()           PORTAbits.RA3
#define dev_SetDigitalInput()    do { TRISAbits.TRISA3 = 1; } while(0)
#define dev_SetDigitalOutput()   do { TRISAbits.TRISA3 = 0; } while(0)
#define dev_SetPullup()          do { WPUAbits.WPUA3 = 1; } while(0)
#define dev_ResetPullup()        do { WPUAbits.WPUA3 = 0; } while(0)
#define dev_SetPushPull()        do { ODCONAbits.ODCA3 = 0; } while(0)
#define dev_SetOpenDrain()       do { ODCONAbits.ODCA3 = 1; } while(0)
#define dev_SetAnalogMode()      do { ANSELAbits.ANSELA3 = 1; } while(0)
#define dev_SetDigitalMode()     do { ANSELAbits.ANSELA3 = 0; } while(0)

// get/set RA6 aliases
#define led3_TRIS                 TRISAbits.TRISA6
#define led3_LAT                  LATAbits.LATA6
#define led3_PORT                 PORTAbits.RA6
#define led3_WPU                  WPUAbits.WPUA6
#define led3_OD                   ODCONAbits.ODCA6
#define led3_ANS                  ANSELAbits.ANSELA6
#define led3_SetHigh()            do { LATAbits.LATA6 = 1; } while(0)
#define led3_SetLow()             do { LATAbits.LATA6 = 0; } while(0)
#define led3_Toggle()             do { LATAbits.LATA6 = ~LATAbits.LATA6; } while(0)
#define led3_GetValue()           PORTAbits.RA6
#define led3_SetDigitalInput()    do { TRISAbits.TRISA6 = 1; } while(0)
#define led3_SetDigitalOutput()   do { TRISAbits.TRISA6 = 0; } while(0)
#define led3_SetPullup()          do { WPUAbits.WPUA6 = 1; } while(0)
#define led3_ResetPullup()        do { WPUAbits.WPUA6 = 0; } while(0)
#define led3_SetPushPull()        do { ODCONAbits.ODCA6 = 0; } while(0)
#define led3_SetOpenDrain()       do { ODCONAbits.ODCA6 = 1; } while(0)
#define led3_SetAnalogMode()      do { ANSELAbits.ANSELA6 = 1; } while(0)
#define led3_SetDigitalMode()     do { ANSELAbits.ANSELA6 = 0; } while(0)

// get/set RA7 aliases
#define led4_TRIS                 TRISAbits.TRISA7
#define led4_LAT                  LATAbits.LATA7
#define led4_PORT                 PORTAbits.RA7
#define led4_WPU                  WPUAbits.WPUA7
#define led4_OD                   ODCONAbits.ODCA7
#define led4_ANS                  ANSELAbits.ANSELA7
#define led4_SetHigh()            do { LATAbits.LATA7 = 1; } while(0)
#define led4_SetLow()             do { LATAbits.LATA7 = 0; } while(0)
#define led4_Toggle()             do { LATAbits.LATA7 = ~LATAbits.LATA7; } while(0)
#define led4_GetValue()           PORTAbits.RA7
#define led4_SetDigitalInput()    do { TRISAbits.TRISA7 = 1; } while(0)
#define led4_SetDigitalOutput()   do { TRISAbits.TRISA7 = 0; } while(0)
#define led4_SetPullup()          do { WPUAbits.WPUA7 = 1; } while(0)
#define led4_ResetPullup()        do { WPUAbits.WPUA7 = 0; } while(0)
#define led4_SetPushPull()        do { ODCONAbits.ODCA7 = 0; } while(0)
#define led4_SetOpenDrain()       do { ODCONAbits.ODCA7 = 1; } while(0)
#define led4_SetAnalogMode()      do { ANSELAbits.ANSELA7 = 1; } while(0)
#define led4_SetDigitalMode()     do { ANSELAbits.ANSELA7 = 0; } while(0)

// get/set RB0 aliases
#define cs_TRIS                 TRISBbits.TRISB0
#define cs_LAT                  LATBbits.LATB0
#define cs_PORT                 PORTBbits.RB0
#define cs_WPU                  WPUBbits.WPUB0
#define cs_OD                   ODCONBbits.ODCB0
#define cs_ANS                  ANSELBbits.ANSELB0
#define cs_SetHigh()            do { LATBbits.LATB0 = 1; } while(0)
#define cs_SetLow()             do { LATBbits.LATB0 = 0; } while(0)
#define cs_Toggle()             do { LATBbits.LATB0 = ~LATBbits.LATB0; } while(0)
#define cs_GetValue()           PORTBbits.RB0
#define cs_SetDigitalInput()    do { TRISBbits.TRISB0 = 1; } while(0)
#define cs_SetDigitalOutput()   do { TRISBbits.TRISB0 = 0; } while(0)
#define cs_SetPullup()          do { WPUBbits.WPUB0 = 1; } while(0)
#define cs_ResetPullup()        do { WPUBbits.WPUB0 = 0; } while(0)
#define cs_SetPushPull()        do { ODCONBbits.ODCB0 = 0; } while(0)
#define cs_SetOpenDrain()       do { ODCONBbits.ODCB0 = 1; } while(0)
#define cs_SetAnalogMode()      do { ANSELBbits.ANSELB0 = 1; } while(0)
#define cs_SetDigitalMode()     do { ANSELBbits.ANSELB0 = 0; } while(0)

// get/set RB1 aliases
#define mosi_TRIS                 TRISBbits.TRISB1
#define mosi_LAT                  LATBbits.LATB1
#define mosi_PORT                 PORTBbits.RB1
#define mosi_WPU                  WPUBbits.WPUB1
#define mosi_OD                   ODCONBbits.ODCB1
#define mosi_ANS                  ANSELBbits.ANSELB1
#define mosi_SetHigh()            do { LATBbits.LATB1 = 1; } while(0)
#define mosi_SetLow()             do { LATBbits.LATB1 = 0; } while(0)
#define mosi_Toggle()             do { LATBbits.LATB1 = ~LATBbits.LATB1; } while(0)
#define mosi_GetValue()           PORTBbits.RB1
#define mosi_SetDigitalInput()    do { TRISBbits.TRISB1 = 1; } while(0)
#define mosi_SetDigitalOutput()   do { TRISBbits.TRISB1 = 0; } while(0)
#define mosi_SetPullup()          do { WPUBbits.WPUB1 = 1; } while(0)
#define mosi_ResetPullup()        do { WPUBbits.WPUB1 = 0; } while(0)
#define mosi_SetPushPull()        do { ODCONBbits.ODCB1 = 0; } while(0)
#define mosi_SetOpenDrain()       do { ODCONBbits.ODCB1 = 1; } while(0)
#define mosi_SetAnalogMode()      do { ANSELBbits.ANSELB1 = 1; } while(0)
#define mosi_SetDigitalMode()     do { ANSELBbits.ANSELB1 = 0; } while(0)

// get/set RB2 aliases
#define miso_TRIS                 TRISBbits.TRISB2
#define miso_LAT                  LATBbits.LATB2
#define miso_PORT                 PORTBbits.RB2
#define miso_WPU                  WPUBbits.WPUB2
#define miso_OD                   ODCONBbits.ODCB2
#define miso_ANS                  ANSELBbits.ANSELB2
#define miso_SetHigh()            do { LATBbits.LATB2 = 1; } while(0)
#define miso_SetLow()             do { LATBbits.LATB2 = 0; } while(0)
#define miso_Toggle()             do { LATBbits.LATB2 = ~LATBbits.LATB2; } while(0)
#define miso_GetValue()           PORTBbits.RB2
#define miso_SetDigitalInput()    do { TRISBbits.TRISB2 = 1; } while(0)
#define miso_SetDigitalOutput()   do { TRISBbits.TRISB2 = 0; } while(0)
#define miso_SetPullup()          do { WPUBbits.WPUB2 = 1; } while(0)
#define miso_ResetPullup()        do { WPUBbits.WPUB2 = 0; } while(0)
#define miso_SetPushPull()        do { ODCONBbits.ODCB2 = 0; } while(0)
#define miso_SetOpenDrain()       do { ODCONBbits.ODCB2 = 1; } while(0)
#define miso_SetAnalogMode()      do { ANSELBbits.ANSELB2 = 1; } while(0)
#define miso_SetDigitalMode()     do { ANSELBbits.ANSELB2 = 0; } while(0)

// get/set RB3 aliases
#define sck_TRIS                 TRISBbits.TRISB3
#define sck_LAT                  LATBbits.LATB3
#define sck_PORT                 PORTBbits.RB3
#define sck_WPU                  WPUBbits.WPUB3
#define sck_OD                   ODCONBbits.ODCB3
#define sck_ANS                  ANSELBbits.ANSELB3
#define sck_SetHigh()            do { LATBbits.LATB3 = 1; } while(0)
#define sck_SetLow()             do { LATBbits.LATB3 = 0; } while(0)
#define sck_Toggle()             do { LATBbits.LATB3 = ~LATBbits.LATB3; } while(0)
#define sck_GetValue()           PORTBbits.RB3
#define sck_SetDigitalInput()    do { TRISBbits.TRISB3 = 1; } while(0)
#define sck_SetDigitalOutput()   do { TRISBbits.TRISB3 = 0; } while(0)
#define sck_SetPullup()          do { WPUBbits.WPUB3 = 1; } while(0)
#define sck_ResetPullup()        do { WPUBbits.WPUB3 = 0; } while(0)
#define sck_SetPushPull()        do { ODCONBbits.ODCB3 = 0; } while(0)
#define sck_SetOpenDrain()       do { ODCONBbits.ODCB3 = 1; } while(0)
#define sck_SetAnalogMode()      do { ANSELBbits.ANSELB3 = 1; } while(0)
#define sck_SetDigitalMode()     do { ANSELBbits.ANSELB3 = 0; } while(0)

// get/set RB4 aliases
#define rst_TRIS                 TRISBbits.TRISB4
#define rst_LAT                  LATBbits.LATB4
#define rst_PORT                 PORTBbits.RB4
#define rst_WPU                  WPUBbits.WPUB4
#define rst_OD                   ODCONBbits.ODCB4
#define rst_ANS                  ANSELBbits.ANSELB4
#define rst_SetHigh()            do { LATBbits.LATB4 = 1; } while(0)
#define rst_SetLow()             do { LATBbits.LATB4 = 0; } while(0)
#define rst_Toggle()             do { LATBbits.LATB4 = ~LATBbits.LATB4; } while(0)
#define rst_GetValue()           PORTBbits.RB4
#define rst_SetDigitalInput()    do { TRISBbits.TRISB4 = 1; } while(0)
#define rst_SetDigitalOutput()   do { TRISBbits.TRISB4 = 0; } while(0)
#define rst_SetPullup()          do { WPUBbits.WPUB4 = 1; } while(0)
#define rst_ResetPullup()        do { WPUBbits.WPUB4 = 0; } while(0)
#define rst_SetPushPull()        do { ODCONBbits.ODCB4 = 0; } while(0)
#define rst_SetOpenDrain()       do { ODCONBbits.ODCB4 = 1; } while(0)
#define rst_SetAnalogMode()      do { ANSELBbits.ANSELB4 = 1; } while(0)
#define rst_SetDigitalMode()     do { ANSELBbits.ANSELB4 = 0; } while(0)

// get/set RC0 aliases
#define led2_TRIS                 TRISCbits.TRISC0
#define led2_LAT                  LATCbits.LATC0
#define led2_PORT                 PORTCbits.RC0
#define led2_WPU                  WPUCbits.WPUC0
#define led2_OD                   ODCONCbits.ODCC0
#define led2_ANS                  ANSELCbits.ANSELC0
#define led2_SetHigh()            do { LATCbits.LATC0 = 1; } while(0)
#define led2_SetLow()             do { LATCbits.LATC0 = 0; } while(0)
#define led2_Toggle()             do { LATCbits.LATC0 = ~LATCbits.LATC0; } while(0)
#define led2_GetValue()           PORTCbits.RC0
#define led2_SetDigitalInput()    do { TRISCbits.TRISC0 = 1; } while(0)
#define led2_SetDigitalOutput()   do { TRISCbits.TRISC0 = 0; } while(0)
#define led2_SetPullup()          do { WPUCbits.WPUC0 = 1; } while(0)
#define led2_ResetPullup()        do { WPUCbits.WPUC0 = 0; } while(0)
#define led2_SetPushPull()        do { ODCONCbits.ODCC0 = 0; } while(0)
#define led2_SetOpenDrain()       do { ODCONCbits.ODCC0 = 1; } while(0)
#define led2_SetAnalogMode()      do { ANSELCbits.ANSELC0 = 1; } while(0)
#define led2_SetDigitalMode()     do { ANSELCbits.ANSELC0 = 0; } while(0)

// get/set RC1 aliases
#define led1_TRIS                 TRISCbits.TRISC1
#define led1_LAT                  LATCbits.LATC1
#define led1_PORT                 PORTCbits.RC1
#define led1_WPU                  WPUCbits.WPUC1
#define led1_OD                   ODCONCbits.ODCC1
#define led1_ANS                  ANSELCbits.ANSELC1
#define led1_SetHigh()            do { LATCbits.LATC1 = 1; } while(0)
#define led1_SetLow()             do { LATCbits.LATC1 = 0; } while(0)
#define led1_Toggle()             do { LATCbits.LATC1 = ~LATCbits.LATC1; } while(0)
#define led1_GetValue()           PORTCbits.RC1
#define led1_SetDigitalInput()    do { TRISCbits.TRISC1 = 1; } while(0)
#define led1_SetDigitalOutput()   do { TRISCbits.TRISC1 = 0; } while(0)
#define led1_SetPullup()          do { WPUCbits.WPUC1 = 1; } while(0)
#define led1_ResetPullup()        do { WPUCbits.WPUC1 = 0; } while(0)
#define led1_SetPushPull()        do { ODCONCbits.ODCC1 = 0; } while(0)
#define led1_SetOpenDrain()       do { ODCONCbits.ODCC1 = 1; } while(0)
#define led1_SetAnalogMode()      do { ANSELCbits.ANSELC1 = 1; } while(0)
#define led1_SetDigitalMode()     do { ANSELCbits.ANSELC1 = 0; } while(0)

// get/set RC2 aliases
#define aux_TRIS                 TRISCbits.TRISC2
#define aux_LAT                  LATCbits.LATC2
#define aux_PORT                 PORTCbits.RC2
#define aux_WPU                  WPUCbits.WPUC2
#define aux_OD                   ODCONCbits.ODCC2
#define aux_ANS                  ANSELCbits.ANSELC2
#define aux_SetHigh()            do { LATCbits.LATC2 = 1; } while(0)
#define aux_SetLow()             do { LATCbits.LATC2 = 0; } while(0)
#define aux_Toggle()             do { LATCbits.LATC2 = ~LATCbits.LATC2; } while(0)
#define aux_GetValue()           PORTCbits.RC2
#define aux_SetDigitalInput()    do { TRISCbits.TRISC2 = 1; } while(0)
#define aux_SetDigitalOutput()   do { TRISCbits.TRISC2 = 0; } while(0)
#define aux_SetPullup()          do { WPUCbits.WPUC2 = 1; } while(0)
#define aux_ResetPullup()        do { WPUCbits.WPUC2 = 0; } while(0)
#define aux_SetPushPull()        do { ODCONCbits.ODCC2 = 0; } while(0)
#define aux_SetOpenDrain()       do { ODCONCbits.ODCC2 = 1; } while(0)
#define aux_SetAnalogMode()      do { ANSELCbits.ANSELC2 = 1; } while(0)
#define aux_SetDigitalMode()     do { ANSELCbits.ANSELC2 = 0; } while(0)

// get/set RC3 aliases
#define m1_TRIS                 TRISCbits.TRISC3
#define m1_LAT                  LATCbits.LATC3
#define m1_PORT                 PORTCbits.RC3
#define m1_WPU                  WPUCbits.WPUC3
#define m1_OD                   ODCONCbits.ODCC3
#define m1_ANS                  ANSELCbits.ANSELC3
#define m1_SetHigh()            do { LATCbits.LATC3 = 1; } while(0)
#define m1_SetLow()             do { LATCbits.LATC3 = 0; } while(0)
#define m1_Toggle()             do { LATCbits.LATC3 = ~LATCbits.LATC3; } while(0)
#define m1_GetValue()           PORTCbits.RC3
#define m1_SetDigitalInput()    do { TRISCbits.TRISC3 = 1; } while(0)
#define m1_SetDigitalOutput()   do { TRISCbits.TRISC3 = 0; } while(0)
#define m1_SetPullup()          do { WPUCbits.WPUC3 = 1; } while(0)
#define m1_ResetPullup()        do { WPUCbits.WPUC3 = 0; } while(0)
#define m1_SetPushPull()        do { ODCONCbits.ODCC3 = 0; } while(0)
#define m1_SetOpenDrain()       do { ODCONCbits.ODCC3 = 1; } while(0)
#define m1_SetAnalogMode()      do { ANSELCbits.ANSELC3 = 1; } while(0)
#define m1_SetDigitalMode()     do { ANSELCbits.ANSELC3 = 0; } while(0)

// get/set RC4 aliases
#define tx_to_lora_TRIS                 TRISCbits.TRISC4
#define tx_to_lora_LAT                  LATCbits.LATC4
#define tx_to_lora_PORT                 PORTCbits.RC4
#define tx_to_lora_WPU                  WPUCbits.WPUC4
#define tx_to_lora_OD                   ODCONCbits.ODCC4
#define tx_to_lora_ANS                  ANSELCbits.ANSELC4
#define tx_to_lora_SetHigh()            do { LATCbits.LATC4 = 1; } while(0)
#define tx_to_lora_SetLow()             do { LATCbits.LATC4 = 0; } while(0)
#define tx_to_lora_Toggle()             do { LATCbits.LATC4 = ~LATCbits.LATC4; } while(0)
#define tx_to_lora_GetValue()           PORTCbits.RC4
#define tx_to_lora_SetDigitalInput()    do { TRISCbits.TRISC4 = 1; } while(0)
#define tx_to_lora_SetDigitalOutput()   do { TRISCbits.TRISC4 = 0; } while(0)
#define tx_to_lora_SetPullup()          do { WPUCbits.WPUC4 = 1; } while(0)
#define tx_to_lora_ResetPullup()        do { WPUCbits.WPUC4 = 0; } while(0)
#define tx_to_lora_SetPushPull()        do { ODCONCbits.ODCC4 = 0; } while(0)
#define tx_to_lora_SetOpenDrain()       do { ODCONCbits.ODCC4 = 1; } while(0)
#define tx_to_lora_SetAnalogMode()      do { ANSELCbits.ANSELC4 = 1; } while(0)
#define tx_to_lora_SetDigitalMode()     do { ANSELCbits.ANSELC4 = 0; } while(0)

// get/set RC5 aliases
#define rx_from_lora_TRIS                 TRISCbits.TRISC5
#define rx_from_lora_LAT                  LATCbits.LATC5
#define rx_from_lora_PORT                 PORTCbits.RC5
#define rx_from_lora_WPU                  WPUCbits.WPUC5
#define rx_from_lora_OD                   ODCONCbits.ODCC5
#define rx_from_lora_ANS                  ANSELCbits.ANSELC5
#define rx_from_lora_SetHigh()            do { LATCbits.LATC5 = 1; } while(0)
#define rx_from_lora_SetLow()             do { LATCbits.LATC5 = 0; } while(0)
#define rx_from_lora_Toggle()             do { LATCbits.LATC5 = ~LATCbits.LATC5; } while(0)
#define rx_from_lora_GetValue()           PORTCbits.RC5
#define rx_from_lora_SetDigitalInput()    do { TRISCbits.TRISC5 = 1; } while(0)
#define rx_from_lora_SetDigitalOutput()   do { TRISCbits.TRISC5 = 0; } while(0)
#define rx_from_lora_SetPullup()          do { WPUCbits.WPUC5 = 1; } while(0)
#define rx_from_lora_ResetPullup()        do { WPUCbits.WPUC5 = 0; } while(0)
#define rx_from_lora_SetPushPull()        do { ODCONCbits.ODCC5 = 0; } while(0)
#define rx_from_lora_SetOpenDrain()       do { ODCONCbits.ODCC5 = 1; } while(0)
#define rx_from_lora_SetAnalogMode()      do { ANSELCbits.ANSELC5 = 1; } while(0)
#define rx_from_lora_SetDigitalMode()     do { ANSELCbits.ANSELC5 = 0; } while(0)

// get/set RC6 aliases
#define tx_to_pc_TRIS                 TRISCbits.TRISC6
#define tx_to_pc_LAT                  LATCbits.LATC6
#define tx_to_pc_PORT                 PORTCbits.RC6
#define tx_to_pc_WPU                  WPUCbits.WPUC6
#define tx_to_pc_OD                   ODCONCbits.ODCC6
#define tx_to_pc_ANS                  ANSELCbits.ANSELC6
#define tx_to_pc_SetHigh()            do { LATCbits.LATC6 = 1; } while(0)
#define tx_to_pc_SetLow()             do { LATCbits.LATC6 = 0; } while(0)
#define tx_to_pc_Toggle()             do { LATCbits.LATC6 = ~LATCbits.LATC6; } while(0)
#define tx_to_pc_GetValue()           PORTCbits.RC6
#define tx_to_pc_SetDigitalInput()    do { TRISCbits.TRISC6 = 1; } while(0)
#define tx_to_pc_SetDigitalOutput()   do { TRISCbits.TRISC6 = 0; } while(0)
#define tx_to_pc_SetPullup()          do { WPUCbits.WPUC6 = 1; } while(0)
#define tx_to_pc_ResetPullup()        do { WPUCbits.WPUC6 = 0; } while(0)
#define tx_to_pc_SetPushPull()        do { ODCONCbits.ODCC6 = 0; } while(0)
#define tx_to_pc_SetOpenDrain()       do { ODCONCbits.ODCC6 = 1; } while(0)
#define tx_to_pc_SetAnalogMode()      do { ANSELCbits.ANSELC6 = 1; } while(0)
#define tx_to_pc_SetDigitalMode()     do { ANSELCbits.ANSELC6 = 0; } while(0)

// get/set RC7 aliases
#define rx_from_pc_TRIS                 TRISCbits.TRISC7
#define rx_from_pc_LAT                  LATCbits.LATC7
#define rx_from_pc_PORT                 PORTCbits.RC7
#define rx_from_pc_WPU                  WPUCbits.WPUC7
#define rx_from_pc_OD                   ODCONCbits.ODCC7
#define rx_from_pc_ANS                  ANSELCbits.ANSELC7
#define rx_from_pc_SetHigh()            do { LATCbits.LATC7 = 1; } while(0)
#define rx_from_pc_SetLow()             do { LATCbits.LATC7 = 0; } while(0)
#define rx_from_pc_Toggle()             do { LATCbits.LATC7 = ~LATCbits.LATC7; } while(0)
#define rx_from_pc_GetValue()           PORTCbits.RC7
#define rx_from_pc_SetDigitalInput()    do { TRISCbits.TRISC7 = 1; } while(0)
#define rx_from_pc_SetDigitalOutput()   do { TRISCbits.TRISC7 = 0; } while(0)
#define rx_from_pc_SetPullup()          do { WPUCbits.WPUC7 = 1; } while(0)
#define rx_from_pc_ResetPullup()        do { WPUCbits.WPUC7 = 0; } while(0)
#define rx_from_pc_SetPushPull()        do { ODCONCbits.ODCC7 = 0; } while(0)
#define rx_from_pc_SetOpenDrain()       do { ODCONCbits.ODCC7 = 1; } while(0)
#define rx_from_pc_SetAnalogMode()      do { ANSELCbits.ANSELC7 = 1; } while(0)
#define rx_from_pc_SetDigitalMode()     do { ANSELCbits.ANSELC7 = 0; } while(0)

/**
 * @ingroup  pinsdriver
 * @brief GPIO and peripheral I/O initialization
 * @param none
 * @return none
 */
void PIN_MANAGER_Initialize (void);

/**
 * @ingroup  pinsdriver
 * @brief Interrupt on Change Handling routine
 * @param none
 * @return none
 */
void PIN_MANAGER_IOC(void);


#endif // PINS_H
/**
 End of File
*/