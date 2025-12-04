/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    medida_gb.c

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

#include "medida_gb.h"
#include "FreeRTOS.h"
#include "task.h"
#include "menu_display.h"   // para poder mudar estado do menu
//#include "definitions.h"

MEDIDA_GB_DATA medida_gbData;

// ==== AJUSTE OS PINOS AQUI CONFORME SUA HARMONY ====
// Exemplo: Zero-cross em INT1 (pino RB2), Gate em RB5

//#define ZC_TRIS          TRISBbits.TRISB2   // entrada
//#define ZC_PORT          PORTBbits.RB2      // se precisar ler

//#define TRIAC_GATE_TRIS  TRISBbits.TRISB5   // saída
//#define TRIAC_GATE_LAT   LATBbits.LATB5

// ====== Variáveis de controle ======

static volatile uint8_t  g_powerPercent   = 0;       // 0..100 %
static volatile uint32_t g_delayTicks     = 0;       // atraso até disparo (em ticks TMR6)

// Estado do Timer 6
typedef enum
{
    TMR6_STATE_IDLE = 0,
    TMR6_STATE_WAIT_DELAY,   // esperando momento de disparar
    TMR6_STATE_GATE_ON       // gate está em nível alto
} TMR6_STATE_t;

static volatile TMR6_STATE_t g_tmr6State = TMR6_STATE_IDLE;

// Converte porcentagem de potência em atraso (em ticks)
static uint32_t TRIAC_ComputeDelayTicks(uint8_t powerPercent)
{
    if (powerPercent > TRIAC_POWER_MAX)
        powerPercent = TRIAC_POWER_MAX;
    if (powerPercent < TRIAC_POWER_MIN)
        powerPercent = TRIAC_POWER_MIN;

    // 100% => delay = 0 ticks (disparo no início do semiciclo)
    // 0%   => delay = HALF_CYCLE_TICKS (sem condução)
    uint32_t delay = (uint32_t)(100U - powerPercent) * HALF_CYCLE_TICKS / 100U;

    // Evita delay tão grande que não sobra tempo pra gate ligado
    if (delay > (HALF_CYCLE_TICKS - (MIN_GATING_TICKS + SAFETY_TICKS_TO_ZERO)))
    {
        delay = HALF_CYCLE_TICKS - (MIN_GATING_TICKS + SAFETY_TICKS_TO_ZERO);
    }

    return delay;
}

// ====== API pública ======

void TRIAC_SetPowerPercent(uint8_t percent)
{
    taskENTER_CRITICAL();
    g_powerPercent = percent;
    g_delayTicks   = TRIAC_ComputeDelayTicks(g_powerPercent);
    taskEXIT_CRITICAL();
}

void TRIAC_Control_Initialize(void)
{
    // Config gate como saída e desliga
//    TRIAC_GATE_TRIS = 0;
//    TRIAC_GATE_LAT  = 0;
    PINO_TRIAC_GB_Clear();
    
    // Aciona relé 2, necessários fazer isso antes de iniciar a medida
    PINO_RELE2_TAP_Set();

    // Zero-cross como entrada
//    ZC_TRIS = 1;

    // ---- TMR6 ----
    // No Harmony você já configurou o TMR6 com PBCLK=60MHz, prescale=1:8.
    // Aqui só paramos, definimos período inicial e registramos callback.
    TMR6_Stop();
    TMR6_InterruptDisable();

    // Talvez usar TMR6_PeriodSet(0)? MPLAB não está reconhecendo TMR6_CounterSet()
    TMR6_CounterSet(0);
    TMR6_PeriodSet(1000U);        // valor qualquer inicial

    TMR6_CallbackRegister(TMR6_Callback, (uintptr_t)NULL);

    g_tmr6State   = TMR6_STATE_IDLE;
    g_powerPercent = 0;
    g_delayTicks   = TRIAC_ComputeDelayTicks(g_powerPercent);

    // Potência inicial baixa
    TRIAC_SetPowerPercent(0);

    // A configuração de INTx (zero-cross) é feita no Harmony.
    // No handler de INTx você chamará ZC_InterruptHandler().
}

// ====== Handler chamado pela interrupção de zero-cross ======

void ZC_InterruptHandler(GPIO_PIN pin, uintptr_t context)
{
    // Foi detectado um zero-cross: novo semiciclo iniciando.

    // Se potência zero, não disparamos TRIAC
    if (g_powerPercent == 0)
    {
        //TRIAC_GATE_LAT = 0;
        PINO_TRIAC_GB_Clear();
        g_tmr6State = TMR6_STATE_IDLE;
        TMR6_Stop();
        TMR6_InterruptDisable();
        return;
    }

    uint32_t delay = g_delayTicks;

    // Programa Timer6 para esperar o atraso até o disparo
    TMR6_Stop();
    // Talvez usar TMR6_PeriodSet(0)? MPLAB não está reconhecendo TMR6_CounterSet()
    TMR6_CounterSet(0);
    TMR6_PeriodSet((uint16_t)delay);  // HALF_CYCLE_TICKS cabe em 16 bits

    g_tmr6State = TMR6_STATE_WAIT_DELAY;
    TMR6_InterruptEnable();
    TMR6_Start();
}

// ====== Callback do Timer 6 ======

void TMR6_Callback(uint32_t status, uintptr_t context)
{
    (void)status;
    (void)context;

    switch (g_tmr6State)
    {
        case TMR6_STATE_WAIT_DELAY:
        {
            // Chegou a hora de disparar TRIAC
            //TRIAC_GATE_LAT = 1;
            PINO_TRIAC_GB_Set();
            g_tmr6State = TMR6_STATE_GATE_ON;

            // Calcula por quanto tempo manter o gate alto:
            // do ponto atual até próximo do zero-cross:
            uint32_t delay  = g_delayTicks;
            uint32_t gateTicks;

            if (delay >= HALF_CYCLE_TICKS)
            {
                gateTicks = MIN_GATING_TICKS;  // não deveria acontecer
            }
            else
            {
                uint32_t maxGate = HALF_CYCLE_TICKS - delay - SAFETY_TICKS_TO_ZERO;
                if (maxGate < MIN_GATING_TICKS)
                    gateTicks = MIN_GATING_TICKS;
                else
                    gateTicks = maxGate;
            }
            // Talvez usar TMR6_PeriodSet(0)? MPLAB não está reconhecendo TMR6_CounterSet()
            TMR6_CounterSet(0);
            TMR6_PeriodSet((uint16_t)gateTicks);
            break;
        }

        case TMR6_STATE_GATE_ON:
        {
            // Tempo de gate alto acabou, desliga gate
            //TRIAC_GATE_LAT = 0;
            PINO_TRIAC_GB_Clear();
            g_tmr6State = TMR6_STATE_IDLE;

            // Timer só volta a ser usado no próximo zero-cross
            TMR6_Stop();
            TMR6_InterruptDisable();
            break;
        }

        default:
        {
            // Recuperação simples
            //TRIAC_GATE_LAT = 0;
            PINO_TRIAC_GB_Clear();
            
            // Desativa o relé 2 ao final do teste
            PINO_RELE2_TAP_Clear();
            
            g_tmr6State = TMR6_STATE_IDLE;
            TMR6_Stop();
            TMR6_InterruptDisable();
            break;
        }
    }
}

void MEDIDA_GB_RunTestTask(void *pvParameters)
{
    (void) pvParameters;

    const TickType_t testDuration = pdMS_TO_TICKS(5000);   // 5 segundos
    const TickType_t samplePeriod = pdMS_TO_TICKS(100);     // lê corrente a cada 100 ms
    TickType_t startTick;

    // Garante estado inicial previsível
    medida_gbData.correnteA = 0.0f;

    // Inicializa controle do TRIAC (timer, callback etc.)
    TRIAC_Control_Initialize();

    // Potência baixa fixa (ex.: 10%)
    TRIAC_SetPowerPercent(10);

    // Habilita zero-cross (começa a disparar TRIAC via TMR6)
    PINO_ZERO_CROSS_InterruptEnable();

    // Marca início do teste
    startTick = xTaskGetTickCount();

    float corrente = 0.0f;  // por enquanto, dummy
    while ((xTaskGetTickCount() - startTick) < testDuration)
    {
        // TODO: aqui você faz a leitura real de corrente via ADC
        corrente += 0.1f;

        medida_gbData.correnteA = corrente;
        
        // Garante que o estado do display fica em ENSAIO_GB_STATE_ENSAIANDO
        menu_displayData.state = ENSAIO_GB_STATE_ENSAIANDO;
        // Como leu um novo valor de corrente, empilha a ação para atualizar o display
        ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);
        // Delay com valor de 'samplePeriod'
        vTaskDelay(samplePeriod);
    }

    // Fim do teste: desabilita TRIAC e zero-cross
    TRIAC_SetPowerPercent(0);
    PINO_TRIAC_GB_Clear();
    TMR6_Stop();
    TMR6_InterruptDisable();
    PINO_ZERO_CROSS_InterruptDisable();

    // Desliga o relé 2 ao final do teste
    PINO_RELE2_TAP_Clear();

    // Volta o menu para a tela GB
    menu_displayData.state = MENU_DISPLAY_STATE_GB;
    // Atualiza o display novamente para o menu GB
    ACTION_SendEventFromTask(ACT_NONE, ACT_EVENT_DISPLAY_UPDATE);

    // Limpa handle e auto-destrói a task (libera memória do stack)
    xMEDIDA_GB_Tasks = NULL;
    vTaskDelete(NULL);
}


/*******************************************************************************
  Function:
    void MEDIDA_GB_Initialize ( void )

  Remarks:
    See prototype in medida_gb.h.
 */

void MEDIDA_GB_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    medida_gbData.state = MEDIDA_GB_STATE_INIT;
    
    GPIO_PinInterruptCallbackRegister(PINO_ZERO_CROSS_PIN, ZC_InterruptHandler, 0);
    PINO_ZERO_CROSS_InterruptDisable();
    //PINO_ZERO_CROSS_InterruptEnable();
}


/******************************************************************************
  Function:
    void MEDIDA_GB_Tasks ( void )

  Remarks:
    See prototype in medida_gb.h.
 */

void MEDIDA_GB_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( medida_gbData.state )
    {
        /* Application's initial state. */
        case MEDIDA_GB_STATE_INIT:
        {
            bool appInitialized = true;


            if (appInitialized)
            {

                medida_gbData.state = MEDIDA_GB_STATE_SERVICE_TASKS;
            }
            break;
        }

        case MEDIDA_GB_STATE_SERVICE_TASKS:
        {

            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
