/*
 * system.c
 *
 *  Created on: 23 paz 2018
 *      Author: jarek
 */


/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "system.h"
#include "../config.h"
#include "../hdr/hdr_rcc.h"
#include "app.h"


/*
+=============================================================================+
| defines
+=============================================================================+
*/


/*
+=============================================================================+
| typedefs
+=============================================================================+
*/
typedef struct
{
    TaskF Task;
    uint16_t Priority;
    uint16_t PriorityCount;
    TimerF TimerFunc;
    uint16_t TimerPeriod;
    uint16_t TimerCount;
    char *Name;
} SystemTask_t;

/*
+=============================================================================+
| module variables
+=============================================================================+
*/
SystemTask_t TaskTable[MAX_TASK];
volatile uint32_t Sys_TickCount;

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static void mcuClockInit(void);
static void clearTaskTable(void);


/*
+=============================================================================+
| global functions
+=============================================================================+
*/
void system_init(void)
{
	mcuClockInit();
	clearTaskTable();
	SysTick_Config(FREQUENCY/SYSTICK_FREQ);    // set systick
	app_init();
}


void system_scheduler(void)
{
    uint16_t i;

    for (i = 0; i < MAX_TASK; i++)
    {
        if (TaskTable[i].Task != 0) {
            if (TaskTable[i].PriorityCount == 0) {
                TaskTable[i].Task();
                TaskTable[i].PriorityCount = TaskTable[i].Priority;
            } else {
                TaskTable[i].PriorityCount--;
            }
        }
    }
}


TaskHandle_t system_createTask(TaskF task, TimerF timer, uint16_t timer_period, char *task_name, uint16_t priority)
{
    uint16_t i;

    for (i = 0; i < MAX_TASK; i++) {
        if (TaskTable[i].Task == 0) {
            break;
        }
    }

    if (i < MAX_TASK) {
        TaskTable[i].Task = task;
        TaskTable[i].Name = task_name;
        TaskTable[i].TimerFunc = timer;
        TaskTable[i].TimerPeriod = timer_period;
        TaskTable[i].TimerCount = timer_period;
        TaskTable[i].Priority = priority;
        TaskTable[i].PriorityCount = 0;

    }
    return (TaskHandle_t) i;
}


bool system_removeTask(const TaskHandle_t taskHandle)
{
    bool bResult = false;

    if (taskHandle < MAX_TASK) {
        TaskTable[taskHandle].Task = 0;
        TaskTable[taskHandle].Name = 0;
        TaskTable[taskHandle].TimerFunc = 0;
        TaskTable[taskHandle].TimerPeriod = 0;
        TaskTable[taskHandle].Priority = 0;
        TaskTable[taskHandle].PriorityCount = 0;

        bResult = true;
    }
    return bResult;
}


uint32_t system_getTickCount(void)
{
    return Sys_TickCount;
}


void system_changeTaskPriority(const uint16_t task, const uint16_t priority)
{
    if (TaskTable[task].Task != 0) {
        TaskTable[task].Priority = priority;
    }
}


/*
+=============================================================================+
| local functions
+=============================================================================+
*/
static void mcuClockInit(void)
{
    RCC_CR_HSION_bb = 1;    // enable HSI clock
    while(!RCC_CR_HSIRDY_bb);    // wait until HIS clock will be ready
    RCC_CFGR_PLLSRC_bb = 0;    // HSI oscillator clock / 2 selected as PLL input clock
    RCC->CFGR |= RCC_CFGR_PLLMULL9;    // set PLL to multiplay x9
    RCC_CR_PLLON_bb = 1;    // enable PLL
    FLASH->ACR |= FLASH_ACR_LATENCY_1; //  One wait state, if 24 MHz < SYSCLK ≤ 48 MHz
    while (!RCC_CR_PLLRDY_bb);   // wait until PLL will be ready
    RCC->CFGR |= RCC_CFGR_SW_PLL;   // change SYSCLK to PLL
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV1; // HCLK not divided
}


static void clearTaskTable(void)
{
	uint16_t i;

    for (i = 0; i < MAX_TASK; i++) {
        TaskTable[i].Task = 0;
        TaskTable[i].TimerFunc = 0;
    }
}

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/

void SysTick_Handler(void)
{
    uint16_t i;

    Sys_TickCount++;

    for (i = 0; i < MAX_TASK; i++) {
        if (TaskTable[i].TimerFunc == 0) {
            continue;
        }

        TaskTable[i].TimerCount--;
        if (TaskTable[i].TimerCount == 0) {
            TaskTable[i].TimerCount = TaskTable[i].TimerPeriod;
            TaskTable[i].TimerFunc();
        }
    }
}

/******************************************************************************
* END OF FILE
******************************************************************************/
