#ifndef ALGORITHM_H_
#define ALGORITHM_H_



extern int rangeScaleLinear
(int x,
		int x_min,
		int x_max,
		int y_min,
		int y_max);


extern double rangeScaleVoltPerOclave
(double v,
		double v0,
		double f0
		);

#endif /* ALGORITHM_H_ */
