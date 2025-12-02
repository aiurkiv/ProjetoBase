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
#include "definitions.h"


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


// Fila de eventos para o menu
QueueHandle_t xButtonEventQueue = NULL;

#define BTN_DEBOUNCE_MS     15
#define BTN_LONGPRESS_MS    500   // tempo segurando para começar auto-repeat
#define BTN_REPEAT_MS       200   // após isso, repete a cada 100 ms

// Máscara de estado estável (debounced): bit 0=BACK, 1=ENTER, 2=MAIS, 3=MENOS
static volatile uint8_t g_stableMask = 0;

// Contador de debounce em ms (quando >0, estamos esperando estabilizar)
static volatile uint16_t g_debounceCounter = 0;

// Contadores de tempo "segurando" cada botão (para long-press/repeat)
static volatile uint16_t g_holdMs[BTN_COUNT];
static volatile bool     g_repeatActive[BTN_COUNT];

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

// Função rápida para ler o estado atual dos pinos em forma de máscara:
static inline uint8_t BUTTONS_ReadRawMask(void)
{
    uint8_t mask = 0;

    // assumindo 0 = pressionado
    if (PINO_BTN_BACK_Get()  == 0) mask |= (1 << BTN_BACK);
    if (PINO_BTN_ENTER_Get() == 0) mask |= (1 << BTN_ENTER);
    if (PINO_BTN_CIMA_Get()  == 0) mask |= (1 << BTN_CIMA);
    if (PINO_BTN_BAIXO_Get() == 0) mask |= (1 << BTN_BAIXO);

    return mask;
}

// Enviar evento para fila (para usar em ISR):
static inline void BUTTON_SendEventFromISR(BUTTON_ID id, BUTTON_EVENT_TYPE type,
                                           BaseType_t *pxHigherPriorityTaskWoken)
{
    BUTTON_EVENT ev;
    ev.id   = id;
    ev.type = type;

    xQueueSendFromISR(xButtonEventQueue, &ev, pxHigherPriorityTaskWoken);
}

/*  switch_handler()
 * Função única que configuro no Callback dos botões.
 * Todos os botões chamam a mesma função pois identifico qual botão foi apertado
 * no debounce do timer 3.
 */
void switch_handler(GPIO_PIN pin, uintptr_t context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // Inicia (ou mantém) o contador de debounce
    g_debounceCounter = BTN_DEBOUNCE_MS;

    // Zera contadores de hold para todos (vai começar de novo)
    for (int i = 0; i < BTN_COUNT; i++)
    {
        g_holdMs[i] = 0;
        g_repeatActive[i] = false;
    }

    // Garante que o Timer de 1ms está rodando
    TMR3_Start();            // ou TMRx_Start conforme Harmony
    TMR3_InterruptEnable();  // se não estiver habilitado

    // Se você usar alguma API FreeRTOS aqui, finalize:
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}


/* void setup_switches()
 * Configura os Callback (interrupções) das GPIOs dos botões.
 */
void setup_switches()
{
    GPIO_PinInterruptCallbackRegister(PINO_BTN_BACK_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_ENTER_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_CIMA_PIN, switch_handler, 0);
    GPIO_PinInterruptCallbackRegister(PINO_BTN_BAIXO_PIN, switch_handler, 0);
    PINO_BTN_BACK_InterruptEnable();
    PINO_BTN_ENTER_InterruptEnable();
    PINO_BTN_CIMA_InterruptEnable();
    PINO_BTN_BAIXO_InterruptEnable();
}

/* TMR3_Callback()
 * Trata o Callback (interrupção) do Timer3.
 * Uso o timer 3 para lidar com o debounce dos botões + apertar e segurar um botão
 */
void TMR3_Callback(uint32_t status, uintptr_t context)  // ou TMR3_InterruptHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    // 1) Tratamento de debounce
    if (g_debounceCounter > 0)
    {
        g_debounceCounter--;

        if (g_debounceCounter == 0)
        {
            // Momento de "congelar" o estado e comparar com o estável
            uint8_t newMask = BUTTONS_ReadRawMask();
            uint8_t diff    = newMask ^ g_stableMask; // bits que mudaram

            for (int i = 0; i < BTN_COUNT; i++)
            {
                uint8_t bit = (1 << i);

                if (diff & bit)
                {
                    if (newMask & bit)
                    {
                        // botão i agora está pressionado (debounced)
                        g_stableMask |= bit;
                        g_holdMs[i] = 0;
                        g_repeatActive[i] = false;

                        BUTTON_SendEventFromISR((BUTTON_ID)i, BTN_EVENT_PRESS,
                                                &xHigherPriorityTaskWoken);
                    }
                    else
                    {
                        // botão i agora está solto
                        g_stableMask &= ~bit;
                        g_holdMs[i] = 0;
                        g_repeatActive[i] = false;

                        BUTTON_SendEventFromISR((BUTTON_ID)i, BTN_EVENT_RELEASE,
                                                &xHigherPriorityTaskWoken);
                    }
                }
            }
        }
    }
    else
    {
        // 2) Debounce já terminou. Se existir botão pressionado, trata hold/repeat.

        if (g_stableMask != 0)
        {
            for (int i = 0; i < BTN_COUNT; i++)
            {
                uint8_t bit = (1 << i);

                if (g_stableMask & bit)
                {
                    g_holdMs[i]++;

                    if (!g_repeatActive[i])
                    {
                        if (g_holdMs[i] >= BTN_LONGPRESS_MS)
                        {
                            g_repeatActive[i] = true;
                            g_holdMs[i] = 0;

                            // opcional: já manda o primeiro repeat aqui
                            BUTTON_SendEventFromISR((BUTTON_ID)i, BTN_EVENT_REPEAT,
                                                    &xHigherPriorityTaskWoken);
                        }
                    }
                    else
                    {
                        if (g_holdMs[i] >= BTN_REPEAT_MS)
                        {
                            g_holdMs[i] = 0;
                            BUTTON_SendEventFromISR((BUTTON_ID)i, BTN_EVENT_REPEAT,
                                                    &xHigherPriorityTaskWoken);
                        }
                    }
                }
                else
                {
                    g_holdMs[i] = 0;
                    g_repeatActive[i] = false;
                }
            }
        }
        else
        {
            // NENHUM botão está pressionado e não tem debounce rolando:
            // podemos parar o timer para não gastar CPU à toa.
            TMR3_InterruptDisable();
            TMR3_Stop();
        }
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
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

    // cria fila de eventos
    xButtonEventQueue = xQueueCreate(16, sizeof(BUTTON_EVENT));
    configASSERT(xButtonEventQueue != NULL);

    g_stableMask = 0;
    g_debounceCounter = 0;
    memset((void*)g_holdMs, 0, sizeof(g_holdMs));
    memset((void*)g_repeatActive, 0, sizeof(g_repeatActive));
    
    // *** REGISTRA O CALLBACK DO TIMER 3 ***
    TMR3_CallbackRegister(TMR3_Callback, 0);

    // Configura botões (CN) normalmente
    setup_switches();

    // Configura TMR2 no Harmony para período de 1ms (mas NÃO precisa deixar startado)
    TMR3_Stop();
    TMR3_InterruptDisable();
}

static bool MENU_DISPLAY_DetectEventBTN_CIMA(void)
{
    bool needRedraw = false;
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            if (menu_displayData.currentItem > 0)
                menu_displayData.currentItem--;
            else
                menu_displayData.currentItem = 2;
            needRedraw = true;
            break;
        }
        default:
            break;
    }
    return needRedraw;
}

static bool MENU_DISPLAY_DetectEventBTN_BAIXO(void)
{
    bool needRedraw = false;
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            if (menu_displayData.currentItem < 2)
                menu_displayData.currentItem++;
            else
                menu_displayData.currentItem = 0;
            needRedraw = true;
            break;
        }
        default:
            break;
    }
    return needRedraw;
}

static bool MENU_DISPLAY_DetectEventBTN_ENTER(void)
{
    bool needRedraw = false;
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            if (menu_displayData.currentItem == 0) menu_displayData.state = MENU_DISPLAY_STATE_HP;
            else if (menu_displayData.currentItem == 1) menu_displayData.state = MENU_DISPLAY_STATE_GB;
            else if (menu_displayData.currentItem == 2) menu_displayData.state = MENU_DISPLAY_STATE_TF;
            needRedraw = true;
            break;
        }
        default:
            break;
    }
    return needRedraw;
}

static bool MENU_DISPLAY_DetectEventBTN_BACK(void)
{
    bool needRedraw = false;
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_HP:
        case MENU_DISPLAY_STATE_GB:
        case MENU_DISPLAY_STATE_TF:
        {
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            needRedraw = true;
            break;
        }
        default:
        {
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            needRedraw = true;
            break;
        }
    }
    return needRedraw;
}

static void MENU_DISPLAY_HandleButtonEvent(const BUTTON_EVENT *ev)
{
    switch (ev->id)
    {
        case BTN_CIMA:
            if (ev->type == BTN_EVENT_PRESS || ev->type == BTN_EVENT_REPEAT)
            {
                MENU_DISPLAY_DetectEventBTN_CIMA();
            }
            break;

        case BTN_BAIXO:
            if (ev->type == BTN_EVENT_PRESS || ev->type == BTN_EVENT_REPEAT)
            {
                MENU_DISPLAY_DetectEventBTN_BAIXO();
            }
            break;

        case BTN_ENTER:
            if (ev->type == BTN_EVENT_PRESS)
            {
                // entra na opção
                MENU_DISPLAY_DetectEventBTN_ENTER();
            }
            break;

        case BTN_BACK:
            if (ev->type == BTN_EVENT_PRESS)
            {
                // volta
                MENU_DISPLAY_DetectEventBTN_BACK();
            }
            break;

        case BTN_COUNT:
        default:
        {
            // Volta ao estado inicial
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            menu_displayData.currentItem = 0;
            //MENU_DISPLAY_DrawHome();
            break;
        }  
    }
}

/******************************************************************************
  Function:
    void MENU_DISPLAY_Tasks ( void )

  Remarks:
    See prototype in menu_display.h.
 */

void MENU_DISPLAY_Tasks ( void )
{
    switch (menu_displayData.state)
    {
        case MENU_DISPLAY_STATE_INIT:
        {
            MENU_DISPLAY_DrawHome();
            atualiza_lcd((char*)menu_displayData.lcd);

            //menu_displayData.state = MENU_DISPLAY_STATE_SERVICE_TASKS;
            break;
        }
        case MENU_DISPLAY_STATE_HP:
        {
            MENU_DISPLAY_DrawHP();
            atualiza_lcd((char*)menu_displayData.lcd);

            //menu_displayData.state = MENU_DISPLAY_STATE_SERVICE_TASKS;
            break;
        }
        case MENU_DISPLAY_STATE_GB:
        {
            MENU_DISPLAY_DrawGB();
            atualiza_lcd((char*)menu_displayData.lcd);

            //menu_displayData.state = MENU_DISPLAY_STATE_SERVICE_TASKS;
            break;
        }
        case MENU_DISPLAY_STATE_TF:
        {
            MENU_DISPLAY_DrawTF();
            atualiza_lcd((char*)menu_displayData.lcd);

            //menu_displayData.state = MENU_DISPLAY_STATE_SERVICE_TASKS;
            break;
        }
/*
        case MENU_DISPLAY_STATE_SERVICE_TASKS:
        {
            BUTTON_EVENT ev;
            // Bloqueia um pouco esperando eventos
            if (xQueueReceive(xButtonEventQueue, &ev, portMAX_DELAY) == pdPASS)
            {
                MENU_DISPLAY_HandleButtonEvent(&ev);
            }
            break;
        }
*/
        default:
            menu_displayData.state = MENU_DISPLAY_STATE_INIT;
            break;
    }
    BUTTON_EVENT ev;
    // Bloqueia um pouco esperando eventos
    if (xQueueReceive(xButtonEventQueue, &ev, portMAX_DELAY) == pdPASS)
        MENU_DISPLAY_HandleButtonEvent(&ev);
}
/************** Daqui para baixo são os estados e funções de impressão no display **************/
void MENU_DISPLAY_DrawHome(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));

    // Linha 0 - título
    const char *titulo = "  Menu Principal  ";
    memcpy(menu_displayData.lcd[0], titulo, strlen(titulo));

    // Linhas 1,2,3 - itens (exemplo)
    // currentItem define onde fica o '>'
    const char *it1 = "Ensaio HP";
    const char *it2 = "Ensaio GB";
    const char *it3 = "Ensaio TF";

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

void MENU_DISPLAY_DrawHP(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio HP", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void MENU_DISPLAY_DrawGB(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio GB", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

void MENU_DISPLAY_DrawTF(void)
{
    // Limpa o buffer
    memset(menu_displayData.lcd, ' ', sizeof(menu_displayData.lcd));
    memcpy(menu_displayData.lcd[0], "     Ensaio TF", 14);
    memcpy(menu_displayData.lcd[3], "<BACK>       <ENTER>", 20);
}

/*******************************************************************************
 End of File
 */
