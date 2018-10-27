#ifndef _MEASURE_H
#define _MEASURE_H

/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include <stdint.h>

/*
+=============================================================================+
| defines
+=============================================================================+
*/


/*
+=============================================================================+
| global definitions
+=============================================================================+
*/
typedef struct {
	uint16_t potentiometer;
} measurmentData;

/*
+=============================================================================+
| strange variables
+=============================================================================+
*/



/*
+=============================================================================+
| global variables
+=============================================================================+
*/



/*
+=============================================================================+
| global functions' declarations
+=============================================================================+
*/
void measure_init(void);
uint16_t measure_getVoltage(void);


/******************************************************************************
* END OF FILE
******************************************************************************/

#endif // _MEASURE_H