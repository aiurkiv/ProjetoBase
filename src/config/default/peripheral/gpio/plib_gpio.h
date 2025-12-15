/*******************************************************************************
  GPIO PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_gpio.h UUUUUUUUU

  Summary:
    GPIO PLIB Header File

  Description:
    This library provides an interface to control and interact with Parallel
    Input/Output controller (GPIO) module.

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#ifndef PLIB_GPIO_H
#define PLIB_GPIO_H

#include <device.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data types and constants
// *****************************************************************************
// *****************************************************************************


/*** Macros for PINO_BTN_BACK pin ***/
#define PINO_BTN_BACK_Set()               (LATGSET = (1U<<9))
#define PINO_BTN_BACK_Clear()             (LATGCLR = (1U<<9))
#define PINO_BTN_BACK_Toggle()            (LATGINV= (1U<<9))
#define PINO_BTN_BACK_OutputEnable()      (TRISGCLR = (1U<<9))
#define PINO_BTN_BACK_InputEnable()       (TRISGSET = (1U<<9))
#define PINO_BTN_BACK_Get()               ((PORTG >> 9) & 0x1U)
#define PINO_BTN_BACK_GetLatch()          ((LATG >> 9) & 0x1U)
#define PINO_BTN_BACK_PIN                  GPIO_PIN_RG9
#define PINO_BTN_BACK_InterruptEnable()   (CNENGSET = (1U<<9))
#define PINO_BTN_BACK_InterruptDisable()  (CNENGCLR = (1U<<9))

/*** Macros for PINO_BTN_ENTER pin ***/
#define PINO_BTN_ENTER_Set()               (LATASET = (1U<<12))
#define PINO_BTN_ENTER_Clear()             (LATACLR = (1U<<12))
#define PINO_BTN_ENTER_Toggle()            (LATAINV= (1U<<12))
#define PINO_BTN_ENTER_OutputEnable()      (TRISACLR = (1U<<12))
#define PINO_BTN_ENTER_InputEnable()       (TRISASET = (1U<<12))
#define PINO_BTN_ENTER_Get()               ((PORTA >> 12) & 0x1U)
#define PINO_BTN_ENTER_GetLatch()          ((LATA >> 12) & 0x1U)
#define PINO_BTN_ENTER_PIN                  GPIO_PIN_RA12
#define PINO_BTN_ENTER_InterruptEnable()   (CNENASET = (1U<<12))
#define PINO_BTN_ENTER_InterruptDisable()  (CNENACLR = (1U<<12))

/*** Macros for PINO_MUX_B pin ***/
#define PINO_MUX_B_Set()               (LATASET = (1U<<11))
#define PINO_MUX_B_Clear()             (LATACLR = (1U<<11))
#define PINO_MUX_B_Toggle()            (LATAINV= (1U<<11))
#define PINO_MUX_B_OutputEnable()      (TRISACLR = (1U<<11))
#define PINO_MUX_B_InputEnable()       (TRISASET = (1U<<11))
#define PINO_MUX_B_Get()               ((PORTA >> 11) & 0x1U)
#define PINO_MUX_B_GetLatch()          ((LATA >> 11) & 0x1U)
#define PINO_MUX_B_PIN                  GPIO_PIN_RA11

/*** Macros for PINO_MUX_A pin ***/
#define PINO_MUX_A_Set()               (LATASET = (1U<<0))
#define PINO_MUX_A_Clear()             (LATACLR = (1U<<0))
#define PINO_MUX_A_Toggle()            (LATAINV= (1U<<0))
#define PINO_MUX_A_OutputEnable()      (TRISACLR = (1U<<0))
#define PINO_MUX_A_InputEnable()       (TRISASET = (1U<<0))
#define PINO_MUX_A_Get()               ((PORTA >> 0) & 0x1U)
#define PINO_MUX_A_GetLatch()          ((LATA >> 0) & 0x1U)
#define PINO_MUX_A_PIN                  GPIO_PIN_RA0

/*** Macros for PINO_BTN_BAIXO pin ***/
#define PINO_BTN_BAIXO_Set()               (LATBSET = (1U<<2))
#define PINO_BTN_BAIXO_Clear()             (LATBCLR = (1U<<2))
#define PINO_BTN_BAIXO_Toggle()            (LATBINV= (1U<<2))
#define PINO_BTN_BAIXO_OutputEnable()      (TRISBCLR = (1U<<2))
#define PINO_BTN_BAIXO_InputEnable()       (TRISBSET = (1U<<2))
#define PINO_BTN_BAIXO_Get()               ((PORTB >> 2) & 0x1U)
#define PINO_BTN_BAIXO_GetLatch()          ((LATB >> 2) & 0x1U)
#define PINO_BTN_BAIXO_PIN                  GPIO_PIN_RB2
#define PINO_BTN_BAIXO_InterruptEnable()   (CNENBSET = (1U<<2))
#define PINO_BTN_BAIXO_InterruptDisable()  (CNENBCLR = (1U<<2))

/*** Macros for PINO_BTN_CIMA pin ***/
#define PINO_BTN_CIMA_Set()               (LATBSET = (1U<<3))
#define PINO_BTN_CIMA_Clear()             (LATBCLR = (1U<<3))
#define PINO_BTN_CIMA_Toggle()            (LATBINV= (1U<<3))
#define PINO_BTN_CIMA_OutputEnable()      (TRISBCLR = (1U<<3))
#define PINO_BTN_CIMA_InputEnable()       (TRISBSET = (1U<<3))
#define PINO_BTN_CIMA_Get()               ((PORTB >> 3) & 0x1U)
#define PINO_BTN_CIMA_GetLatch()          ((LATB >> 3) & 0x1U)
#define PINO_BTN_CIMA_PIN                  GPIO_PIN_RB3
#define PINO_BTN_CIMA_InterruptEnable()   (CNENBSET = (1U<<3))
#define PINO_BTN_CIMA_InterruptDisable()  (CNENBCLR = (1U<<3))

/*** Macros for PINO_RELE2_TAP pin ***/
#define PINO_RELE2_TAP_Set()               (LATCSET = (1U<<0))
#define PINO_RELE2_TAP_Clear()             (LATCCLR = (1U<<0))
#define PINO_RELE2_TAP_Toggle()            (LATCINV= (1U<<0))
#define PINO_RELE2_TAP_OutputEnable()      (TRISCCLR = (1U<<0))
#define PINO_RELE2_TAP_InputEnable()       (TRISCSET = (1U<<0))
#define PINO_RELE2_TAP_Get()               ((PORTC >> 0) & 0x1U)
#define PINO_RELE2_TAP_GetLatch()          ((LATC >> 0) & 0x1U)
#define PINO_RELE2_TAP_PIN                  GPIO_PIN_RC0

/*** Macros for PINO_RELE1_TAP pin ***/
#define PINO_RELE1_TAP_Set()               (LATCSET = (1U<<1))
#define PINO_RELE1_TAP_Clear()             (LATCCLR = (1U<<1))
#define PINO_RELE1_TAP_Toggle()            (LATCINV= (1U<<1))
#define PINO_RELE1_TAP_OutputEnable()      (TRISCCLR = (1U<<1))
#define PINO_RELE1_TAP_InputEnable()       (TRISCSET = (1U<<1))
#define PINO_RELE1_TAP_Get()               ((PORTC >> 1) & 0x1U)
#define PINO_RELE1_TAP_GetLatch()          ((LATC >> 1) & 0x1U)
#define PINO_RELE1_TAP_PIN                  GPIO_PIN_RC1

/*** Macros for PINO_ZERO_CROSS pin ***/
#define PINO_ZERO_CROSS_Set()               (LATCSET = (1U<<2))
#define PINO_ZERO_CROSS_Clear()             (LATCCLR = (1U<<2))
#define PINO_ZERO_CROSS_Toggle()            (LATCINV= (1U<<2))
#define PINO_ZERO_CROSS_OutputEnable()      (TRISCCLR = (1U<<2))
#define PINO_ZERO_CROSS_InputEnable()       (TRISCSET = (1U<<2))
#define PINO_ZERO_CROSS_Get()               ((PORTC >> 2) & 0x1U)
#define PINO_ZERO_CROSS_GetLatch()          ((LATC >> 2) & 0x1U)
#define PINO_ZERO_CROSS_PIN                  GPIO_PIN_RC2
#define PINO_ZERO_CROSS_InterruptEnable()   (CNENCSET = (1U<<2))
#define PINO_ZERO_CROSS_InterruptDisable()  (CNENCCLR = (1U<<2))

/*** Macros for PINO_BUZZER pin ***/
#define PINO_BUZZER_Set()               (LATCSET = (1U<<11))
#define PINO_BUZZER_Clear()             (LATCCLR = (1U<<11))
#define PINO_BUZZER_Toggle()            (LATCINV= (1U<<11))
#define PINO_BUZZER_OutputEnable()      (TRISCCLR = (1U<<11))
#define PINO_BUZZER_InputEnable()       (TRISCSET = (1U<<11))
#define PINO_BUZZER_Get()               ((PORTC >> 11) & 0x1U)
#define PINO_BUZZER_GetLatch()          ((LATC >> 11) & 0x1U)
#define PINO_BUZZER_PIN                  GPIO_PIN_RC11

/*** Macros for PINO_LCD_RS pin ***/
#define PINO_LCD_RS_Set()               (LATESET = (1U<<12))
#define PINO_LCD_RS_Clear()             (LATECLR = (1U<<12))
#define PINO_LCD_RS_Toggle()            (LATEINV= (1U<<12))
#define PINO_LCD_RS_OutputEnable()      (TRISECLR = (1U<<12))
#define PINO_LCD_RS_InputEnable()       (TRISESET = (1U<<12))
#define PINO_LCD_RS_Get()               ((PORTE >> 12) & 0x1U)
#define PINO_LCD_RS_GetLatch()          ((LATE >> 12) & 0x1U)
#define PINO_LCD_RS_PIN                  GPIO_PIN_RE12
#define PINO_LCD_RS                     LATEbits.LATE12

/*** Macros for PINO_LCD_RW pin ***/
#define PINO_LCD_RW_Set()               (LATESET = (1U<<13))
#define PINO_LCD_RW_Clear()             (LATECLR = (1U<<13))
#define PINO_LCD_RW_Toggle()            (LATEINV= (1U<<13))
#define PINO_LCD_RW_OutputEnable()      (TRISECLR = (1U<<13))
#define PINO_LCD_RW_InputEnable()       (TRISESET = (1U<<13))
#define PINO_LCD_RW_Get()               ((PORTE >> 13) & 0x1U)
#define PINO_LCD_RW_GetLatch()          ((LATE >> 13) & 0x1U)
#define PINO_LCD_RW_PIN                  GPIO_PIN_RE13
#define PINO_LCD_RW                     LATEbits.LATE13

/*** Macros for PINO_LCD_EN pin ***/
#define PINO_LCD_EN_Set()               (LATESET = (1U<<14))
#define PINO_LCD_EN_Clear()             (LATECLR = (1U<<14))
#define PINO_LCD_EN_Toggle()            (LATEINV= (1U<<14))
#define PINO_LCD_EN_OutputEnable()      (TRISECLR = (1U<<14))
#define PINO_LCD_EN_InputEnable()       (TRISESET = (1U<<14))
#define PINO_LCD_EN_Get()               ((PORTE >> 14) & 0x1U)
#define PINO_LCD_EN_GetLatch()          ((LATE >> 14) & 0x1U)
#define PINO_LCD_EN_PIN                  GPIO_PIN_RE14
#define PINO_LCD_EN                     LATEbits.LATE14

/*** Macros for PINO_LCD_D4 pin ***/
#define PINO_LCD_D4_Set()               (LATESET = (1U<<15))
#define PINO_LCD_D4_Clear()             (LATECLR = (1U<<15))
#define PINO_LCD_D4_Toggle()            (LATEINV= (1U<<15))
#define PINO_LCD_D4_OutputEnable()      (TRISECLR = (1U<<15))
#define PINO_LCD_D4_InputEnable()       (TRISESET = (1U<<15))
#define PINO_LCD_D4_Get()               ((PORTE >> 15) & 0x1U)
#define PINO_LCD_D4_GetLatch()          ((LATE >> 15) & 0x1U)
#define PINO_LCD_D4_PIN                  GPIO_PIN_RE15
#define PINO_LCD_D4_OUT                 LATEbits.LATE15

/*** Macros for PINO_LCD_D5 pin ***/
#define PINO_LCD_D5_Set()               (LATASET = (1U<<8))
#define PINO_LCD_D5_Clear()             (LATACLR = (1U<<8))
#define PINO_LCD_D5_Toggle()            (LATAINV= (1U<<8))
#define PINO_LCD_D5_OutputEnable()      (TRISACLR = (1U<<8))
#define PINO_LCD_D5_InputEnable()       (TRISASET = (1U<<8))
#define PINO_LCD_D5_Get()               ((PORTA >> 8) & 0x1U)
#define PINO_LCD_D5_GetLatch()          ((LATA >> 8) & 0x1U)
#define PINO_LCD_D5_PIN                  GPIO_PIN_RA8
#define PINO_LCD_D5_OUT                 LATAbits.LATA8

/*** Macros for PINO_LCD_D6 pin ***/
#define PINO_LCD_D6_Set()               (LATBSET = (1U<<4))
#define PINO_LCD_D6_Clear()             (LATBCLR = (1U<<4))
#define PINO_LCD_D6_Toggle()            (LATBINV= (1U<<4))
#define PINO_LCD_D6_OutputEnable()      (TRISBCLR = (1U<<4))
#define PINO_LCD_D6_InputEnable()       (TRISBSET = (1U<<4))
#define PINO_LCD_D6_Get()               ((PORTB >> 4) & 0x1U)
#define PINO_LCD_D6_GetLatch()          ((LATB >> 4) & 0x1U)
#define PINO_LCD_D6_PIN                  GPIO_PIN_RB4
#define PINO_LCD_D6_OUT                 LATBbits.LATB4

/*** Macros for PINO_LCD_D7 pin ***/
#define PINO_LCD_D7_Set()               (LATASET = (1U<<4))
#define PINO_LCD_D7_Clear()             (LATACLR = (1U<<4))
#define PINO_LCD_D7_Toggle()            (LATAINV= (1U<<4))
#define PINO_LCD_D7_OutputEnable()      (TRISACLR = (1U<<4))
#define PINO_LCD_D7_InputEnable()       (TRISASET = (1U<<4))
#define PINO_LCD_D7_Get()               ((PORTA >> 4) & 0x1U)
#define PINO_LCD_D7_GetLatch()          ((LATA >> 4) & 0x1U)
#define PINO_LCD_D7_PIN                  GPIO_PIN_RA4
#define PINO_LCD_D7_OUT                 LATAbits.LATA4

/*** Macros for VBUSON pin ***/
#define VBUSON_Set()               (LATBSET = (1U<<7))
#define VBUSON_Clear()             (LATBCLR = (1U<<7))
#define VBUSON_Toggle()            (LATBINV= (1U<<7))
#define VBUSON_OutputEnable()      (TRISBCLR = (1U<<7))
#define VBUSON_InputEnable()       (TRISBSET = (1U<<7))
#define VBUSON_Get()               ((PORTB >> 7) & 0x1U)
#define VBUSON_GetLatch()          ((LATB >> 7) & 0x1U)
#define VBUSON_PIN                  GPIO_PIN_RB7

/*** Macros for UART2_TX pin ***/
#define UART2_TX_Get()               ((PORTB >> 9) & 0x1U)
#define UART2_TX_GetLatch()          ((LATB >> 9) & 0x1U)
#define UART2_TX_PIN                  GPIO_PIN_RB9

/*** Macros for PINO_EEPROM_WP pin ***/
#define PINO_EEPROM_WP_Set()               (LATCSET = (1U<<8))
#define PINO_EEPROM_WP_Clear()             (LATCCLR = (1U<<8))
#define PINO_EEPROM_WP_Toggle()            (LATCINV= (1U<<8))
#define PINO_EEPROM_WP_OutputEnable()      (TRISCCLR = (1U<<8))
#define PINO_EEPROM_WP_InputEnable()       (TRISCSET = (1U<<8))
#define PINO_EEPROM_WP_Get()               ((PORTC >> 8) & 0x1U)
#define PINO_EEPROM_WP_GetLatch()          ((LATC >> 8) & 0x1U)
#define PINO_EEPROM_WP_PIN                  GPIO_PIN_RC8

/*** Macros for PINO_TRIAC_GB pin ***/
#define PINO_TRIAC_GB_Set()               (LATDSET = (1U<<5))
#define PINO_TRIAC_GB_Clear()             (LATDCLR = (1U<<5))
#define PINO_TRIAC_GB_Toggle()            (LATDINV= (1U<<5))
#define PINO_TRIAC_GB_OutputEnable()      (TRISDCLR = (1U<<5))
#define PINO_TRIAC_GB_InputEnable()       (TRISDSET = (1U<<5))
#define PINO_TRIAC_GB_Get()               ((PORTD >> 5) & 0x1U)
#define PINO_TRIAC_GB_GetLatch()          ((LATD >> 5) & 0x1U)
#define PINO_TRIAC_GB_PIN                  GPIO_PIN_RD5

/*** Macros for SINAL_TF_220V pin ***/
#define SINAL_TF_220V_Set()               (LATCSET = (1U<<9))
#define SINAL_TF_220V_Clear()             (LATCCLR = (1U<<9))
#define SINAL_TF_220V_Toggle()            (LATCINV= (1U<<9))
#define SINAL_TF_220V_OutputEnable()      (TRISCCLR = (1U<<9))
#define SINAL_TF_220V_InputEnable()       (TRISCSET = (1U<<9))
#define SINAL_TF_220V_Get()               ((PORTC >> 9) & 0x1U)
#define SINAL_TF_220V_GetLatch()          ((LATC >> 9) & 0x1U)
#define SINAL_TF_220V_PIN                  GPIO_PIN_RC9

/*** Macros for SINAL_HP pin ***/
#define SINAL_HP_Set()               (LATBSET = (1U<<12))
#define SINAL_HP_Clear()             (LATBCLR = (1U<<12))
#define SINAL_HP_Toggle()            (LATBINV= (1U<<12))
#define SINAL_HP_OutputEnable()      (TRISBCLR = (1U<<12))
#define SINAL_HP_InputEnable()       (TRISBSET = (1U<<12))
#define SINAL_HP_Get()               ((PORTB >> 12) & 0x1U)
#define SINAL_HP_GetLatch()          ((LATB >> 12) & 0x1U)
#define SINAL_HP_PIN                  GPIO_PIN_RB12

/*** Macros for SINAL_GBT pin ***/
#define SINAL_GBT_Set()               (LATBSET = (1U<<13))
#define SINAL_GBT_Clear()             (LATBCLR = (1U<<13))
#define SINAL_GBT_Toggle()            (LATBINV= (1U<<13))
#define SINAL_GBT_OutputEnable()      (TRISBCLR = (1U<<13))
#define SINAL_GBT_InputEnable()       (TRISBSET = (1U<<13))
#define SINAL_GBT_Get()               ((PORTB >> 13) & 0x1U)
#define SINAL_GBT_GetLatch()          ((LATB >> 13) & 0x1U)
#define SINAL_GBT_PIN                  GPIO_PIN_RB13

/*** Macros for SINAL_TF_127V pin ***/
#define SINAL_TF_127V_Set()               (LATASET = (1U<<10))
#define SINAL_TF_127V_Clear()             (LATACLR = (1U<<10))
#define SINAL_TF_127V_Toggle()            (LATAINV= (1U<<10))
#define SINAL_TF_127V_OutputEnable()      (TRISACLR = (1U<<10))
#define SINAL_TF_127V_InputEnable()       (TRISASET = (1U<<10))
#define SINAL_TF_127V_Get()               ((PORTA >> 10) & 0x1U)
#define SINAL_TF_127V_GetLatch()          ((LATA >> 10) & 0x1U)
#define SINAL_TF_127V_PIN                  GPIO_PIN_RA10


// *****************************************************************************
/* GPIO Port

  Summary:
    Identifies the available GPIO Ports.

  Description:
    This enumeration identifies the available GPIO Ports.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all ports are available on all devices.  Refer to the specific
    device data sheet to determine which ports are supported.
*/


#define    GPIO_PORT_A  (0)
#define    GPIO_PORT_B  (1)
#define    GPIO_PORT_C  (2)
#define    GPIO_PORT_D  (3)
#define    GPIO_PORT_E  (4)
#define    GPIO_PORT_F  (5)
#define    GPIO_PORT_G  (6)
typedef uint32_t GPIO_PORT;

typedef enum
{
    GPIO_INTERRUPT_ON_MISMATCH,
    GPIO_INTERRUPT_ON_RISING_EDGE,
    GPIO_INTERRUPT_ON_FALLING_EDGE,
    GPIO_INTERRUPT_ON_BOTH_EDGES,
}GPIO_INTERRUPT_STYLE;

// *****************************************************************************
/* GPIO Port Pins

  Summary:
    Identifies the available GPIO port pins.

  Description:
    This enumeration identifies the available GPIO port pins.

  Remarks:
    The caller should not rely on the specific numbers assigned to any of
    these values as they may change from one processor to the next.

    Not all pins are available on all devices.  Refer to the specific
    device data sheet to determine which pins are supported.
*/


#define     GPIO_PIN_RA0  (0U)
#define     GPIO_PIN_RA1  (1U)
#define     GPIO_PIN_RA4  (4U)
#define     GPIO_PIN_RA7  (7U)
#define     GPIO_PIN_RA8  (8U)
#define     GPIO_PIN_RA10  (10U)
#define     GPIO_PIN_RA11  (11U)
#define     GPIO_PIN_RA12  (12U)
#define     GPIO_PIN_RB0  (16U)
#define     GPIO_PIN_RB1  (17U)
#define     GPIO_PIN_RB2  (18U)
#define     GPIO_PIN_RB3  (19U)
#define     GPIO_PIN_RB4  (20U)
#define     GPIO_PIN_RB5  (21U)
#define     GPIO_PIN_RB6  (22U)
#define     GPIO_PIN_RB7  (23U)
#define     GPIO_PIN_RB8  (24U)
#define     GPIO_PIN_RB9  (25U)
#define     GPIO_PIN_RB10  (26U)
#define     GPIO_PIN_RB11  (27U)
#define     GPIO_PIN_RB12  (28U)
#define     GPIO_PIN_RB13  (29U)
#define     GPIO_PIN_RB14  (30U)
#define     GPIO_PIN_RB15  (31U)
#define     GPIO_PIN_RC0  (32U)
#define     GPIO_PIN_RC1  (33U)
#define     GPIO_PIN_RC2  (34U)
#define     GPIO_PIN_RC6  (38U)
#define     GPIO_PIN_RC7  (39U)
#define     GPIO_PIN_RC8  (40U)
#define     GPIO_PIN_RC9  (41U)
#define     GPIO_PIN_RC10  (42U)
#define     GPIO_PIN_RC11  (43U)
#define     GPIO_PIN_RC12  (44U)
#define     GPIO_PIN_RC13  (45U)
#define     GPIO_PIN_RC15  (47U)
#define     GPIO_PIN_RD5  (53U)
#define     GPIO_PIN_RD6  (54U)
#define     GPIO_PIN_RD8  (56U)
#define     GPIO_PIN_RE12  (76U)
#define     GPIO_PIN_RE13  (77U)
#define     GPIO_PIN_RE14  (78U)
#define     GPIO_PIN_RE15  (79U)
#define     GPIO_PIN_RF0  (80U)
#define     GPIO_PIN_RF1  (81U)
#define     GPIO_PIN_RG6  (102U)
#define     GPIO_PIN_RG7  (103U)
#define     GPIO_PIN_RG8  (104U)
#define     GPIO_PIN_RG9  (105U)

    /* This element should not be used in any of the GPIO APIs.
       It will be used by other modules or application to denote that none of the GPIO Pin is used */
#define    GPIO_PIN_NONE   (-1)

typedef uint32_t GPIO_PIN;

typedef  void (*GPIO_PIN_CALLBACK) ( GPIO_PIN pin, uintptr_t context);

void GPIO_Initialize(void);

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on multiple pins of a port
// *****************************************************************************
// *****************************************************************************

uint32_t GPIO_PortRead(GPIO_PORT port);

void GPIO_PortWrite(GPIO_PORT port, uint32_t mask, uint32_t value);

uint32_t GPIO_PortLatchRead ( GPIO_PORT port );

void GPIO_PortSet(GPIO_PORT port, uint32_t mask);

void GPIO_PortClear(GPIO_PORT port, uint32_t mask);

void GPIO_PortToggle(GPIO_PORT port, uint32_t mask);

void GPIO_PortInputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortOutputEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptEnable(GPIO_PORT port, uint32_t mask);

void GPIO_PortInterruptDisable(GPIO_PORT port, uint32_t mask);

// *****************************************************************************
// *****************************************************************************
// Section: Local Data types and Prototypes
// *****************************************************************************
// *****************************************************************************

typedef struct {

    /* target pin */
    GPIO_PIN                 pin;

    /* Callback for event on target pin*/
    GPIO_PIN_CALLBACK        callback;

    /* Callback Context */
    uintptr_t               context;

} GPIO_PIN_CALLBACK_OBJ;

// *****************************************************************************
// *****************************************************************************
// Section: GPIO Functions which operates on one pin at a time
// *****************************************************************************
// *****************************************************************************

static inline void GPIO_PinWrite(GPIO_PIN pin, bool value)
{
     uint32_t xvalue = (uint32_t)value;
    GPIO_PortWrite((pin>>4U), (uint32_t)(0x1U) << (pin & 0xFU), (xvalue) << (pin & 0xFU));
}

static inline bool GPIO_PinRead(GPIO_PIN pin)
{
    return ((((GPIO_PortRead((GPIO_PORT)(pin>>4U))) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline bool GPIO_PinLatchRead(GPIO_PIN pin)
{
    return (((GPIO_PortLatchRead((GPIO_PORT)(pin>>4U)) >> (pin & 0xFU)) & 0x1U) != 0U);
}

static inline void GPIO_PinToggle(GPIO_PIN pin)
{
    GPIO_PortToggle((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinSet(GPIO_PIN pin)
{
    GPIO_PortSet((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinClear(GPIO_PIN pin)
{
    GPIO_PortClear((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinInputEnable(GPIO_PIN pin)
{
    GPIO_PortInputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

static inline void GPIO_PinOutputEnable(GPIO_PIN pin)
{
    GPIO_PortOutputEnable((pin>>4U), (uint32_t)0x1U << (pin & 0xFU));
}

#define GPIO_PinInterruptEnable(pin)       GPIO_PinIntEnable(pin, GPIO_INTERRUPT_ON_MISMATCH)
#define GPIO_PinInterruptDisable(pin)      GPIO_PinIntDisable(pin)

void GPIO_PinIntEnable(GPIO_PIN pin, GPIO_INTERRUPT_STYLE style);
void GPIO_PinIntDisable(GPIO_PIN pin);

bool GPIO_PinInterruptCallbackRegister(
    GPIO_PIN pin,
    const   GPIO_PIN_CALLBACK callback,
    uintptr_t context
);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END
#endif // PLIB_GPIO_H
