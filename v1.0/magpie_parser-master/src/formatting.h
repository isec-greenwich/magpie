// Formatting functions header file

#include "definitions.h"

#ifndef _FORMATTING_
char *extract_path(char *arg);
char *concat_path_and_filename(char *filename, char *path);
char *extract_line(char *lines, char **updated_offset);
char *return_me_not_char(char *ptr, char not_char);
#define _FORMATTING_
#endif
