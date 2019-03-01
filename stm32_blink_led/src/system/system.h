#ifndef _SYSTEM_H
#define _SYSTEM_H

/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include <stdbool.h>
#include <stdint.h>

/*
+=============================================================================+
| defines
+=============================================================================+
*/
#define MAX_TASK 5

#define TIMER_START(timer, value) 	{timer.cnt = value; timer.flag = false;}
#define TIMER_STOP(timer)			{timer.cnt = 0; timer.flag = false;}


/*
+=============================================================================+
| global definitions
+=============================================================================+
*/
typedef struct {
	uint16_t cnt;
	bool flag;
} BasicTimer;

typedef struct {
	uint32_t cnt;
	bool flag;
} AdvancedTimer;

typedef void (*TaskF)(void);
typedef void (*TimerF)(void);
typedef uint16_t TaskHandle_t;


/*
+=============================================================================+
| global variables
+=============================================================================+
*/



/*
+=============================================================================+
| global functions declarations
+=============================================================================+
*/
void system_init(void);

TaskHandle_t system_createTask(TaskF task, TimerF timer, uint16_t timer_period, char *task_name, uint16_t priority);

bool system_removeTask(const TaskHandle_t taskHandle);

void system_scheduler(void);

uint32_t system_getTickCount(void);

void system_changeTaskPriority(const uint16_t task, const uint16_t priority);


/******************************************************************************
* END OF FILE
******************************************************************************/

#endif // _SYSTEM_H
