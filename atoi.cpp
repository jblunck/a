#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "stop_watch.hpp"

volatile const char str[] = "12345";

unsigned int version1(const char *str)
{
	unsigned int i,result = 0;
	unsigned int exp[5] = { 10000, 1000, 100, 10, 1 };

	for (i=0;i<5;++i) {
		unsigned int val = str[i] - '0';
		if (val > 9)
			abort();
		result += val * exp[i];
	}
	return result;
}

unsigned int version2(const char *str)
{
	unsigned int result, val;

	result  = (str[0] - '0') * 10000;
	if (result > 90000)
		abort();
	val = result;
	result = (str[1] - '0') * 1000;
	if (result > 9000)
		abort();
	val += result;
	result = (str[2] - '0') * 100;
	if (result > 900)
		abort();
	val += result;
	result = (str[3] - '0') * 10;
	if (result > 90)
		abort();
	val += result;
	result =  str[4] - '0';
	if (result > 9)
		abort();
	val += result;
	return val;
}

unsigned int version3(const char *str)
{
	unsigned int result[5];
	unsigned int exp[5] = { 10000, 1000, 100, 10, 1 };
	unsigned int lim[5] = { 90000, 9000, 900, 90, 9 };

	result[0] = (str[0] - '0') * exp[0];
	result[1] = (str[1] - '0') * exp[1];
	result[2] = (str[2] - '0') * exp[2];
	result[3] = (str[3] - '0') * exp[3];
	result[4] = (str[4] - '0') * exp[4];

	unsigned int val;
	val = result[0] > lim[0];
	val += result[1] > lim[1];
	val += result[2] > lim[2];
	val += result[3] > lim[3];
	val += result[4] > lim[4];
	if ( val )
		abort();

	return result[0] + result[1] + result[2] + result[3] + result[4];
}

unsigned int limits(unsigned int val, unsigned int i) __attribute__((always_inline));

unsigned int limits(unsigned int val, unsigned int i)
{
	unsigned int exp[5] = { 10000, 1000, 100, 10, 1 };

	switch(val)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		return val * exp[i];
	default:
		abort();
	}
}

unsigned int version4(const char *str)
{
	unsigned int result[5];

	result[0] = limits(str[0] - '0', 0);
	result[1] = limits(str[1] - '0', 1);
	result[2] = limits(str[2] - '0', 2);
	result[3] = limits(str[3] - '0', 3);
	result[4] = limits(str[4] - '0', 4);

	return result[0] + result[1] + result[2] + result[3] + result[4];
}

unsigned int version5(const char *str)
{
	unsigned int result[5];
	unsigned int lim[5];

	result[0] = str[0] - '0';
	lim[0] = result[0] > 9 ? 1 : 0;
	result[1] = str[1] - '0';
	lim[1] = result[1] > 9 ? 1 : 0;
	result[2] = str[2] - '0';
	lim[2] = result[2] > 9 ? 1 : 0;
	result[3] = str[3] - '0';
	lim[3] = result[3] > 9 ? 1 : 0;
	result[4] = str[4] - '0';
	lim[4] = result[4] > 9 ? 1 : 0;

	if (lim[0] & lim[1] & lim[2] & lim[3] & lim[4])
		abort();

	unsigned int exp[5] = { 10000, 1000, 100, 10, 1 };
	return  result[0] * exp[0] +
		result[1] * exp[1] +
		result[2] * exp[2] +
		result[3] * exp[3] +
		result[4] * exp[4];
}

int main(int argc, char *argv[])
{
	int i;
	const char * str = argv[1];

	stop_watch sw;
	for (i=0; i<1e9;++i)
		assert(version5(str) == 12345);
	uint64_t t = sw.elapsed_ns();
	printf("%ld\n", t);

	sw.restart();
	for (i=0; i<1e9;++i)
		assert(version5(str) == 12345);
	t = sw.elapsed_ns();
	printf("%ld\n", t);

	sw.restart();
	for (i=0; i<1e9;++i)
		assert(version4(str) == 12345);
	t = sw.elapsed_ns();
	printf("%ld\n", t);

	sw.restart();
	for (i=0; i<1e9;++i)
		assert(version3(str) == 12345);
	t = sw.elapsed_ns();
	printf("%ld\n", t);

	sw.restart();
	for (i=0; i<1e9;++i)
		assert(version2(str) == 12345);
	t = sw.elapsed_ns();
	printf("%ld\n", t);

	sw.restart();
	for (i=0; i<1e9;++i)
		assert(version1(str) == 12345);
	t = sw.elapsed_ns();
	printf("%ld\n", t);

	return 0;
}


