#include "int_spectrum.h"
#include "formatting.h"
#include "epoch_conv.h"
#include "fileutils.h"
double *add_double_to_array(double *array, double val, unsigned int index){
	double *_temp_array = array;
	if (_temp_array == 0){ _temp_array = (double*)calloc(index, sizeof(double)); _temp_array[0] = val; return _temp_array; }
        double *_new_array = (double*)calloc(index, sizeof(double));
        memcpy(_new_array, _temp_array, sizeof(double)*(index-1));
        free(_temp_array);
        _new_array[index-1] = val;
	return _new_array;
}

void process_rf_output(char *ptr, spec_struct_internal *spec_frm){
	// 2018-07-22, 18:10:16, 2401000000, 2403000000, 2000000.00, 1, 15.75, 15.75
	// ....47 lines
	// 2018-07-22, 18:10:16, 2493000000, 2495000000, 2000000.00, 1, 15.88, 15.88
	// one output
	unsigned char validate = 0; // test
	unsigned char t_valid = strncmp(ptr, "2018", 4);
	if (t_valid != 0 ){
		t_valid = strncmp(ptr, "2019", 4);
	}
	if (t_valid == 0){ // validated line
		// create timestamp, extract only db's
		char *t_ptr = ptr;
		int i = 0;
		while (t_ptr[i] != ',')i++;
		char *timestamp = (char*)calloc(20, sizeof(char));
		memcpy(timestamp, ptr, 10);i++; // skip ","
		memcpy(timestamp+i-1, ptr+i, 9);
		spec_frm->timestamp = date_to_epoch(timestamp);
		free(timestamp);
		// go to the value
		while (t_ptr[i] != ',')i++; i++;
		while (t_ptr[i] != ',')i++; i++;
		while (t_ptr[i] != ',')i++; i++;
		while (t_ptr[i] != ',')i++; i++;
		while (t_ptr[i] != ',')i++; i++;
		char *n_ptr = &ptr[i+1];
		while (t_ptr[i] != ',')i++;
		t_ptr[i] = 0;
		spec_frm->n++;
		// add value
		spec_frm->array = add_double_to_array(spec_frm->array, atof(n_ptr), spec_frm->n);
	} else {
		free(spec_frm);
		spec_frm = 0;
		// skip, filtering is done via year, if line starts not from year 2018, skip
	}
}

				
