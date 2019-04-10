// File functions header

#include "definitions.h"
#include "formatting.h"
#include "enumerator.h"

#ifndef _FILEUTILS_
FILE *open_file(char *filename, const char *mode, char *path);
uint64_t get_number_of_lines(FILE *file, uint64_t *line_offsets, uint64_t line_offset_index);
unsigned char load_maps(unsigned short argument_flags, Enum_Type *Enumerator, char *args);
char *extract_next_line(FILE *file, int *len);
void write_descriptor(Enum_Type *Enum, const char *file, char *out_filename_ptr, unsigned short argument_flags, char *args);
char *extract_number_of_lines(FILE *file, long start_line_offset, long end_line_offset, uint64_t *line_offsets);
void write_out_frames_new(void *Object, int num, char feedback_char, unsigned int overall_lines, char *out_filename_ptr, unsigned short argument_flags, char *args);
void process_zigbee_file_input(FILE *file, unsigned long number_of_lines);
void process_wifi_file_input(FILE *file, unsigned long number_of_lines);
#define _FILEUTILS_
#endif
