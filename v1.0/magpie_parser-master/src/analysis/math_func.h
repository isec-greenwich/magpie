// Header file for maths functions
#ifndef _ANALYSIS_MATHS_
#include <stdlib.h>
#include <inttypes.h>
//#include <stdint.h>
#include <limits.h>
#include <math.h>

void _math_minmax(unsigned int *array, unsigned int n, unsigned int *min, unsigned int *max);
void _math_minmax_dbl(double *array, unsigned int n, double *min, double *max);
double _math_average(unsigned int *array, unsigned int n);
double _math_average_i(int *array, unsigned int n);
double _math_average_dbl(double *array, unsigned int n);
double _math_variance(unsigned int *array, double average, unsigned int n);
double _math_variance_i(int *array, double average, unsigned int n);
double _math_variance_dbl(double *array, double average, unsigned int n);
double _math_stdev(double variance);
double _math_avg_dev(unsigned int *array, unsigned int n);
double _math_avg_dev_dbl(double *array, unsigned int n);
int _math_count_threshold(double *array, unsigned int n, double threshold);
unsigned int* _math_generate_latency_array(uint64_t *array, unsigned int n);

#define _ANALYSIS_MATHS_
#endif
