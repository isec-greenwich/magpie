// Audio inteface functions header
#include "definitions.h"
#ifndef _INT_SPECT_

typedef struct spec_struct_internal{
	uint64_t timestamp;
	unsigned int n;
	double *array;	
}spec_struct_internal;

double *add_double_to_array(double *array, double val);
void process_rf_output(char *ptr, spec_struct_internal *spec_frm);

#define _INT_SPECT_
#endif

