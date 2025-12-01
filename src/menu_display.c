/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    menu_display.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include <string.h>
#include "menu_display.h"
#include "app_display.h"     // atualiza_lcd()

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the MENU_DISPLAY_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

MENU_DISPLAY_DATA menu_displayData;

// Flags de eventos de botão (setadas na interrupção, lidas na task)
static volatile bool btn_back_event  = false;
static volatile bool btn_enter_event = false;
static volatile bool btn_mais_event  = false;
static volatile bool btn_menos_event = false;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************
// ----------------- Funções locais (estáticas) -----------------

static void MENU_DISPLAY_ClearBuffer(void)
{
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
}

static void MENU_DISPLAY_DrawHome(void)
{
    MENU_DISPLAY_ClearBuffer();

    // Linha 0 - título
    const char *titulo = "  Menu Principal  ";
    memcpy(menu_displayData.lcd[0], titulo, strlen(titulo));

    // Linhas 1,2,3 - itens (exemplo)
    // currentItem define onde fica o '>'
    const char *it1 = "Opcao 1";
    const char *it2 = "Opcao 2";
    const char *it3 = "Opcao 3";

    // Preenche com espaços primeiro
    // Já foram preenchidos com ' ' na ClearBuffer

    if (menu_displayData.currentItem == 0)
        menu_displayData.lcd[1][0] = '>';
    if (menu_displayData.currentItem == 1)
        menu_displayData.lcd[2][0] = '>';
    if (menu_displayData.currentItem == 2)
        menu_displayData.lcd[3][0] = '>';

    memcpy(&menu_displayData.lcd[1][2], it1, strlen(it1));
    memcpy(&menu_displayData.lcd[2][2], it2, strlen(it2));
    memcpy(&menu_displayData.lcd[3][2], it3, strlen(it3));
}

static void MENU_DISPLAY_Render(void)
{
    // Envia o buffer 4x20 para a task do display
    atualiza_lcd((char*)menu_displayData.lcd);
}

// Tratar navegação no home
static void MENU_DISPLAY_HandleHomeButtons(void)
{
    bool needRedraw = false;

    if (btn_mais_event)
    {
        btn_mais_event = false;

        if (menu_displayData.currentItem < 2)
        {
            menu_displayData.currentItem++;
        }
        else
        {
            menu_displayData.currentItem = 0;
        }
        needRedraw = true;
    }

    if (btn_menos_event)
    {
        btn_menos_event = false;

        if (menu_displayData.currentItem > 0)
        {
            menu_displayData.currentItem--;
        }
        else
        {
            menu_displayData.currentItem = 2;
        }
        needRedraw = true;
    }

    if (btn_enter_event)
    {
        btn_enter_event = false;
        // Aqui você poderia mudar de tela,
        // ex: MENU_SCREEN_PARAM1, MENU_SCREEN_PARAM2, etc.
        // Por enquanto só marca que precisa redesenhar (ou mostrar algo).
        needRedraw = true;
    }

    if (btn_back_event)
    {
        btn_back_event = false;
        // Exemplo: voltar para HOME ou outra tela
        // Aqui já estamos no HOME, então pode ignorar ou usar p/ outra coisa
        needRedraw = true;
    }

    if (needRedraw)
    {
        MENU_DISPLAY_DrawHome();
        MENU_DISPLAY_Render();
    }
}
/*******************************************************************************
  Function:
    void MENU_DISPLAY_Initialize ( void )

  Remarks:
    See prototype in menu_display.h.
 */

void MENU_DISPLAY_Initialize ( void )
{
    menu_displayData.state         = MENU_DISPLAY_STATE_INIT;
    menu_displayData.currentScreen = MENU_SCREEN_HOME;
    menu_displayData.currentItem   = 0;

    MENU_DISPLAY_ClearBuffer();

    // Configura interrupções de tecla
    setup_switches();
}

void setup_switches()
{
    GPIO_PinInterruptCallbackRegister(PINO_BTN_BACK_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_ENTER_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_MAIS_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_MENOS_PIN, switch_handler, 0);
    PINO_BTN_BACK_InterruptEnable();
    PINO_BTN_ENTER_InterruptEnable();
    PINO_BTN_MAIS_InterruptEnable();
    PINO_BTN_MENOS_InterruptEnable();
}
void switch_handler(GPIO_PIN pin, uintptr_t context)
{
    // Aqui é importante ser bem rápido: só marca flags.

    // Ajuste conforme sua lógica: se o botão é ativo em nível baixo ou alto.
    // Vou assumir que "pressionado" = nível baixo (0).
    if (pin == PINO_BTN_BACK_PIN)
    {
        if (PINO_BTN_BACK_Get() == 0)
            btn_back_event = true;
    }
    else if (pin == PINO_BTN_ENTER_PIN)
    {
        if (PINO_BTN_ENTER_Get() == 0)
            btn_enter_event = true;
    }
    else if (pin == PINO_BTN_MAIS_PIN)
    {
        if (PINO_BTN_MAIS_Get() == 0)
            btn_mais_event = true;
    }
    else if (pin == PINO_BTN_MENOS_PIN)
    {
        if (PINO_BTN_MENOS_Get() == 0)
            btn_menos_event = true;
    }
    /*
    if(pin == PINO_BTN_BACK_PIN)
    {
        if(PINO_BTN_BACK_Get())
            SINAL_TF_127V_Clear();
        else
            SINAL_TF_127V_Set();
    }
    else if(pin == PINO_BTN_ENTER_PIN)
    {
        if(PINO_BTN_ENTER_Get())
            SINAL_TF_220V_Clear();
        else
            SINAL_TF_220V_Set();
    }
    else if(pin == PINO_BTN_MAIS_PIN)
    {
        if(PINO_BTN_MAIS_Get())
            SINAL_HP_Clear();
        else
            SINAL_HP_Set();
    }
    else if(pin == PINO_BTN_MENOS_PIN)
    {
        if(PINO_BTN_MENOS_Get())
            SINAL_GBT_Clear();
        else
            SINAL_GBT_Set();
    }
    */
}


/******************************************************************************
  Function:
    void MENU_DISPLAY_Tasks ( void )

  Remarks:
    See prototype in menu_display.h.
 */

void MENU_DISPLAY_Tasks ( void )
{
    switch ( menu_displayData.state )
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            // Desenha a tela inicial e envia ao display
            MENU_DISPLAY_DrawHome();
            MENU_DISPLAY_Render();

            menu_displayData.state = MENU_DISPLAY_STATE_SERVICE_TASKS;
            break;
        }

        case MENU_DISPLAY_STATE_SERVICE_TASKS:
        {
            // Aqui tratamos eventos de botão e atualizamos o LCD
            switch (menu_displayData.currentScreen)
            {
                case MENU_SCREEN_HOME:
                    MENU_DISPLAY_HandleHomeButtons();
                    break;

                case MENU_SCREEN_PARAM1:
                    // Aqui você implementa as telas adicionais
                    break;

                case MENU_SCREEN_PARAM2:
                    // Outra tela
                    break;

                default:
                    break;
            }
            break;
        }

        default:
        {
            // Estado inesperado ? poderia resetar o menu
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
