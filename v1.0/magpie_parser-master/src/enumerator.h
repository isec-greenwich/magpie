// Enumerator header file
#include "definitions.h"
#ifndef _ENUMERATOR_
typedef struct Enum_Type{
        unsigned short frame_type;
        char *name;
        struct Enum_Type *next;
}Enum_Type;

void enum_init(Enum_Type *Enum);
unsigned short enum_add(char *name, Enum_Type *Enum);
unsigned short enum_add_num(char *name, Enum_Type *Enum, unsigned short frame_id);
char* enum_find_frame_name(unsigned short id, Enum_Type *Enum);
unsigned short enum_find_frame_type(char *name, Enum_Type *Enum);
unsigned short manage_enumerations(Enum_Type *Enum_Start, char *start_pointer, char* end_pointer);
unsigned char manage_comparison(char *start_pointer, char terminating_char, const char *value);
#define _ENUMERATOR_
#endif
