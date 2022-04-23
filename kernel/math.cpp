#ifndef _MATH_C_
#define _MATH_C_

#include "math.h"

double abs(double x)
{
	if (x < 0)
	{
		return -x;
	}
	return x;
}

double pown(double x, int n)
{
	double r = 1.0;
	for (int i = 0; i < n; ++i)
	{
		r *= x;
	}
	return r;
}





double __sqrt(float a)
{
	double x, y;
	x = 0.0;
	y = a / 2;
	while (x != y)
	{
		x = y;
		y = (x + a / x) / 2;
	}
	return x;
}

double sqrt(double x)
{
	if (x < 0)
	{
		return -1.0;
	}
	if (x == 0)
	{
		return 0.0;
	}
	double x0, x1;
	x0 = x;
	x1 = x / 2.0;
	while (abs(x0 - x1) > 0.0000000000000001)
	{
		x0 = x1;
		x1 = (x0 + (x / x0)) / 2;
	}
	return x1;
}



float __abs(float x)
{
	if (x < 0) 
		x = 0 - x;
	return x;
}

float sin(float x)
{
	const float B = 1.2732395447;
	const float C = -0.4052847346;
	const float P = 0.2310792853;//0.225; 
	float y = B * x + C * x * abs(x);
	y = P * (y * abs(y) - y) + y;
	return y;
}


float cos(float x)
{
	double Q = 1.5707963268;

	x += Q;
	if (x > PI)
		x -= 2 * PI;

	return(sin(x));
}



float pow(float a, int b)
{
	float r = a;
	if (b > 0)
	{
		while (--b)
			r *= a;

	}
	else if (b < 0)
	{
		while (++b)     
			r *= a;
		r = 1.0 / r;
	}
	else 
		r = 0;
	return r;
}

#endif
