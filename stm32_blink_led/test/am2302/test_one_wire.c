#include "unity.h"
#include <string.h>
#include "one_wire.h"
#include "mock_gpio.h"

#define BUFF_SIZE 34
#define PHASE_SIZE 6

uint16_t input[BUFF_SIZE] = {
		20,
		30,
		60,
		90,
		120,
		150,
		180,
		210,
		240,
		270,
		300,
		330,
		360,
		390,
		420,
		450,
		480,
		510,
		540,
		570,
		600,
		630,
		660,
		690,
		720,
		750,
		780,
		810,
		840,
		870,
		900,
		930,
		960,
		990
};

uint16_t output[BUFF_SIZE] = {
		1,
		2,
		5,
		7,
		10,
		12,
		15,
		17,
		20,
		22,
		25,
		27,
		30,
		32,
		35,
		37,
		40,
		42,
		45,
		47,
		50,
		52,
		55,
		57,
		60,
		62,
		65,
		67,
		70,
		72,
		75,
		77,
		80,
		82,

};

one_wire_timeThr posibTim[PHASE_SIZE]= {
	{20, 40},
	{78, 82},
	{48, 52},
	{68, 72},
	{25, 29},
	{49, 52}
};

uint8_t timeIn[PHASE_SIZE] =  {30, 79, 51, 70, 26, 53};
uint8_t timeRes[PHASE_SIZE] = { 1,  1,  1,  1,  1,  0 };

void setUp(void)
{
}

void tearDown(void)
{
}

/*
void test_one_wire_timeConvert(void)
{
	uint8_t res = 0, idx;
	for (idx = 0; idx < BUFF_SIZE; idx++) {

		res = one_wire_timeConvert(input[idx]);

		printf("Lp.%d In:%d Exp: %d Res: %d\n", idx, input[idx], output[idx], res);

		TEST_ASSERT_EQUAL_UINT8(output[idx], res);
	}

}
*/

void test_one_wire_checkTime(void)
{
	uint16_t res = 0;
	uint16_t idx = 0;

	for (idx = 0; idx < PHASE_SIZE; idx++) {

		res = one_wire_checkTime(timeIn[idx], posibTim[idx]);

		printf("Lp.%d  %d < %d < %d  res: %d\n", idx, posibTim[idx].low, timeIn[idx], posibTim[idx].high, res);

		TEST_ASSERT_EQUAL_UINT8(timeRes[idx], res);
	}
}


#define BIT_AMOUNT 8

volatile uint8_t buffTim[BIT_AMOUNT*2] = { 50, 70, 50, 27, 50, 70, 50, 26, 50, 70, 50, 70, 50, 26, 50, 26 };
uint8_t buffExp[BIT_AMOUNT] =   { 1, 0, 1, 0, 1, 1, 0, 0};


void test_one_wire_bitCheck(void)
{
	one_wire_posibBits res = 0;
	uint16_t index = 0;
	uint16_t bitIdx = 0;

	for (bitIdx = 0; bitIdx < BIT_AMOUNT; bitIdx++)
	{
		res = one_wire_checkBit(buffTim, &index);

		printf("Lp.%d  res: %d exp:%d\n", bitIdx, res, buffExp[bitIdx]);

		TEST_ASSERT_EQUAL_UINT8(buffExp[bitIdx], (uint8_t)res);
	}

}

volatile uint8_t buffTimFail[2] = { 50, 23};

void test_one_wire_bitCheckFail(void)
{
	one_wire_posibBits res = 0;
	uint16_t index = 0;

	res = one_wire_checkBit(buffTimFail, &index);

	printf("Fail__res: %d exp:%d\n", res, ONE_WIRE_ERROR_BIT);

	TEST_ASSERT_EQUAL_UINT8(ONE_WIRE_ERROR_BIT, (uint8_t)res);


}

#define SHIFT_BIT_AMOUNT 40

one_wire_frame tempMeas;
uint8_t buffShiftData[SHIFT_BIT_AMOUNT] = {
		1,0,1,0,1,0,0,0,0,0,1,1,1,1,0,1,
		0,0,1,1,0,0,1,0,0,0,1,1,1,1,1,1,
		0,0,0,0,1,1,1,0
};

void test_one_wire_bitShift(void)
{
	uint16_t idxBit;
	bool bitRes = true;

	memset(&tempMeas, 0 , sizeof(tempMeas));

	for (idxBit = 0; idxBit < SHIFT_BIT_AMOUNT; idxBit++)
	{
		bitRes = one_wire_shiftBit(&tempMeas, buffShiftData[idxBit], idxBit);
		TEST_ASSERT_TRUE(bitRes);
	}

	printf("HUM: %d TEMP:%d CHCK: %d \n", tempMeas.hum, tempMeas.temp, tempMeas.chck);
}



#define ONES_CONFINGS 4

uint16_t oneTmp[ONES_CONFINGS] = {
		0b1111000000001111,
		0b1111111100000000,
		0b0000000000000001,
		0b1100000000000011,
};
uint8_t oneExp[ONES_CONFINGS] = {255, 255, 1, 0b11000011};

void test_one_wire_checkOnes(void)
{
	uint16_t idxOne;
	uint8_t ones = 0;

	printf("CheckOnes\n");

	for (idxOne = 0; idxOne < ONES_CONFINGS; idxOne++)
	{
		ones = one_wire_checkOne(oneTmp[idxOne]);

		printf("Lp.%d  res: %d exp: %d \n", idxOne, ones, oneExp[idxOne]);

		TEST_ASSERT_EQUAL_UINT8(oneExp[idxOne], ones);
	}
}



one_wire_frame tmpChck;

void test_one_wire_checkSum(void)
{
	bool resChck = false;
	memset(&tmpChck, 0, sizeof(tmpChck));

	printf("CheckSum\n");

	tmpChck.hum  = 0b1000000100000011;
	tmpChck.temp = 0b1010001001111011;
	tmpChck.chck = 0b10100001;

	resChck = one_wire_checkSum(tmpChck);

	TEST_ASSERT_TRUE(resChck);
}


#define DECODE_RAW_DATA_SIZE 84

uint8_t buffDecode[] = {
		22, 78, 81, 53, 26, 53, 26, 53, 26, 53,
		26, 53, 26, 53, 26, 53, 73, 53, 72, 53,
		26, 53, 26, 53, 26, 53, 26, 53, 73, 53,
		73, 53, 26, 53, 72, 53, 26, 53, 26, 53,
		26, 53, 26, 53, 26, 53, 26, 53, 26, 53,
		72, 53, 26, 53, 26, 53, 26, 53, 73, 53,
		73, 53, 73, 53, 26, 53, 25, 53, 26, 53,
		26, 53, 73, 53, 26, 53, 73, 53, 73, 53,
		26, 53, 72, 46
};

uint8_t buffDecode2[] = {
		22, 78, 81, 53, 26, 53, 26, 53, 26, 53,
		26, 53, 26, 53, 26, 53, 26, 53, 72, 67,
		73, 53, 26, 53, 26, 53, 73, 53, 73, 53,
		26, 53, 26, 53, 72, 67, 26, 53, 26, 53,
		26, 53, 26, 53, 26, 53, 26, 53, 26, 53,
		25, 67, 73, 53, 26, 53, 73, 53, 73, 53,
		73, 53, 73, 53, 26, 53, 25, 64, 26, 53,
		73, 53, 26, 53, 73, 53, 26, 53, 73, 53,
		73, 53, 25, 46
};


void test_one_wire_decoder(void)
{
	bool resDecode = false;

	printf("Decoder1\n");

	one_wire_test_fillBuff(buffDecode, DECODE_RAW_DATA_SIZE);

	resDecode = one_wire_frameDecoder();

	TEST_ASSERT_TRUE(resDecode);

	resDecode = false;

	printf("Decoder2\n");

	one_wire_test_fillBuff(buffDecode2, DECODE_RAW_DATA_SIZE);

	resDecode = one_wire_frameDecoder();

	TEST_ASSERT_TRUE(resDecode);

	printf("All pass!!!\n");
}




