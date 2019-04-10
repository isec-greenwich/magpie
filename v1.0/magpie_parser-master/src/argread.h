// Argument read functions header file
#include "definitions.h"
#include "time.h"
#ifndef _ARGREAD_
unsigned short extract_flag(char *arg, unsigned short argument_flags, char *in_filename_ptr, char *out_filename_ptr, unsigned char *window_val, unsigned long *epoch);
unsigned short argument_flagger(int argc, char* argv[], unsigned short argument_flags, char *in_file, char *out_file, unsigned char *window_val, unsigned long *epoch);
#define _ARGREAD_
#endif
