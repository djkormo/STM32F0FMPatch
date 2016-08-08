/*
 * algorithm.c
 *
 *  Created on: 11 lip 2016
 *      Author: kormo
 */
#include "algorithm.h"
#include "resources.h"
#include <math.h>

/*
Zak³adaj¹c, ¿e mamy wartoœæ x z przedzia³u [x_min, x_max]
i chcemy j¹ przenieœæ do [y_min, y_max], mo¿na skorzystaæ ze wzoru:

*/
int rangeScaleLinear
(int x,
		int x_min,
		int x_max,
		int y_min,
		int y_max)

{
	return (int) y_min+(x-x_min)*(y_max-y_min)/(x_max-x_min);
};

// f(v)= f0*2^ (v-v0/v0)

double rangeScaleVoltPerOclave
(double v,
		double v0,
		double f0
		)

{
	double f;
	double power;
	uint8_t pint;
	uint8_t powindex;
	double pfrac;

	power = ((v-v0)/v0);

	pint=(int)floor(power);
	pfrac=power-pint;

	powindex = (int)(pint/0.001);

	//f=f0 * pow(2.0,power);
	//PowerOf2_16bit


	f=f0*PowerOf2_16bit[powindex];

	//*TODO dodac obslugê pint>0
	//*TODO dodac obsluge pint <0

	return f;

};
