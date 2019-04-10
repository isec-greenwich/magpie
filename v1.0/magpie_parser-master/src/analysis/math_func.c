// Math Function implementations
#include "math_func.h"

void _math_minmax(unsigned int *array, unsigned int n, unsigned int *min, unsigned int *max){
        unsigned int _min = INT_MAX;
        unsigned int _max = 0;
        unsigned int i = 0;
        while (i < n){
                if (_min > array[i]) _min = array[i];
                if (_max < array[i]) _max = array[i];
                i++;
        }
        *min = _min;
        *max = _max;

}
void _math_minmax_dbl(double *array, unsigned int n, double *min, double *max){
        double _min = 9999999999.0;
        double _max = 0;
        unsigned int i = 0;
        while (i < n){
                if (_min > array[i]) _min = array[i];
                if (_max < array[i]) _max = array[i];
                i++;
        }
        *min = _min;
        *max = _max;
}
double _math_average(unsigned int *array, unsigned int n){
        double result = 0.0;
        unsigned int i = 0;
        while(i < n){
                result += array[i];
                i++;
        }
        result = result/n;
        return result;
}
double _math_average_i(int *array, unsigned int n){
        double result = 0.0;
        unsigned int i = 0;
        while(i < n){
                result += array[i];
                i++;
        }
        result = result/n;
        return result;
}

double _math_average_dbl(double *array, unsigned int n){
        double result = 0.0;
        unsigned int i = 0;
        while(i < n){
                result += array[i];
                i++;
        }
        result = result/n;
        return result;
}
double _math_variance(unsigned int *array, double average, unsigned int n){
        double result = 0.0;
        unsigned i = 0;
        while(i < n){
                result += pow(((double)array[i]-average), 2);
                i++;
        }
        result = result/(n-1);
        return result;
}
double _math_variance_i(int *array, double average, unsigned int n){
        double result = 0.0;
        unsigned i = 0;
        while(i < n){
                result += pow(((double)array[i]-average), 2);
                i++;
        }
        result = result/(n-1);
        return result;
}

double _math_variance_dbl(double *array, double average, unsigned int n){
        double result = 0.0;
        unsigned i = 0;
        while(i < n){
                result += pow(((double)array[i]-average), 2);
                i++;
        }
        result = result/(n-1);
        return result;
}
double _math_stdev(double variance){
        double result;
        result = sqrt(variance);
        return result;
}

double _math_avg_dev(unsigned int *array, unsigned int n){
        double result;
        unsigned int i = 1;
        while(i < n){
                result = fabs((double)array[i]-(double)array[i-1]);
                i++;
        }
        result = result/(n-1);
        return result;
}
double _math_avg_dev_dbl(double *array, unsigned int n){
        double result;
        unsigned int i = 1;
        while(i < n){
                result = fabs((double)array[i]-(double)array[i-1]);
                i++;
        }
        result = result/(n-1);
        return result;
}
int _math_count_threshold(double *array, unsigned int n, double threshold){
        int result = 0;
        unsigned int i = 0;
        while (i < n){
                if (array[i++] > threshold) result++;
        }
        return result;
}
unsigned int* _math_generate_latency_array(uint64_t *array, unsigned int n){
	unsigned int *result = (unsigned int *)calloc(n, sizeof(int));
	unsigned int i = 1;
	while(i < n){
		uint64_t _t = (array[i] - array[i-1]);
		result[i-1] = (unsigned int)_t;
		i++;
	}
	return result;
}
