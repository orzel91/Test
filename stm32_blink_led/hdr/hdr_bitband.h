/** \file hdr_bitband.h
 * \brief Header for bit-banding
 * \author Freddie Chopin, http://www.freddiechopin.info/
 * \date 2012-01-07
 */

/******************************************************************************
* chip: ARMv7-M (Cortex-M3)
* compiler: arm-none-eabi-gcc (Sourcery CodeBench Lite 2011.09-69) 4.6.1
******************************************************************************/

#ifndef HDR_BITBAND_H_
#define HDR_BITBAND_H_

/*
+=============================================================================+
| global definitions
+=============================================================================+
*/

#define BITBAND_SRAM_REF   					0x20000000
#define BITBAND_SRAM_BASE  					0x22000000

#define BITBAND_PERIPH_REF   				0x40000000
#define BITBAND_PERIPH_BASE  				0x42000000


enum { SRAM_BB_REGION_START = 0x20000000 };
enum { SRAM_BB_REGION_END = 0x200fffff };
enum { SRAM_BB_ALIAS = 0x22000000 };
enum { PERIPH_BB_REGION_START = 0x40000000 };
enum { PERIPH_BB_REGION_END = 0x400fffff };
enum { PERIPH_BB_ALIAS = 0x42000000 };

/*
+=============================================================================+
| strange variables
+=============================================================================+
*/

#define bitband_t *(volatile unsigned long*)

/*
+=============================================================================+
| macros
+=============================================================================+
*/

#define m_BITBAND_SRAM(address, bit)		(BITBAND_SRAM_BASE + (((unsigned long)address) - BITBAND_SRAM_REF) * 32 + (bit) * 4)
#define m_BITBAND_PERIPH(address, bit)		(BITBAND_PERIPH_BASE + (((unsigned long)address) - BITBAND_PERIPH_REF) * 32 + (bit) * 4)

#define SRAM_ADR_COND(adres)  ( (uint32_t)&adres >= SRAM_BB_REGION_START && (uint32_t)&adres <= SRAM_BB_REGION_END )
#define PERIPH_ADR_COND(adres)  ( (uint32_t)&adres >= PERIPH_BB_REGION_START && (uint32_t)&adres <= PERIPH_BB_REGION_END )
#define BB_SRAM2(adres, bit) ( SRAM_BB_ALIAS + ((uint32_t)&adres - SRAM_BB_REGION_START)*32u + (uint32_t)(bit*4u) )
#define BB_PERIPH(adres, bit) ( PERIPH_BB_ALIAS + ((uint32_t)&adres - PERIPH_BB_REGION_START)*32u + (uint32_t)(__builtin_ctz(bit))*4u)
/* bit - bit mask, not bit position! */
#define BB(adres, bit) *(__IO uint32_t *)( SRAM_ADR_COND(adres) ? BB_SRAM2(adres, bit) : \
( PERIPH_ADR_COND(adres) ? BB_PERIPH(adres, bit) : 0 ))
#define BB_SRAM(adres, bit) *(__IO uint32_t *)BB_SRAM2(adres, bit)

/******************************************************************************
* END OF FILE
******************************************************************************/
#endif /* HDR_BITBAND_H_ */
