// Functions for windowing header file
#ifndef _ANALYSIS_WINDOWING_
#include "../definitions.h"
#include "generic.h"

typedef struct FRAME{
        unsigned int frame_len;
        char *frame_ptr;
        struct FRAME *next;
}FRAME;

typedef struct SLOT{
        uint64_t slot_start_time;
        unsigned int n;
        FRAME *frame_array;
        uint64_t slot_stop_time;
	unsigned char tag;
}SLOT;

SLOT* slot_init();
void free_slot(SLOT *slot);
void frame_add(SLOT *slot, void *object, unsigned char object_size, unsigned char type);
void update_stop_start_times(SLOT *slot, uint64_t global_start_time, unsigned int *multiplier, unsigned int interval);
unsigned short get_unique_number_frames(SLOT *slot, unsigned char frame_type, GLOBAL_KNOWLEDGE *glob);
unsigned short get_unique_number_wi(SLOT *slot, GLOBAL_KNOWLEDGE *glob);
unsigned short get_unique_number(SLOT *slot, GLOBAL_KNOWLEDGE *glob);


#define _ANALYSIS_WINDOWING_
#endif
