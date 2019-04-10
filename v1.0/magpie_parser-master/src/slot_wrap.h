// slot wrapper function header file
#ifndef _SLOT_WRAP_
#include "./analysis/windowing.h"
#include "int_wifi.h"
#include "int_zbee.h"
#include "int_spectrum.h"
#include "int_audio.h"
#include "./analysis/math_func.h"
#include "./analysis/generic.h"
#include "./analysis/processor.h"
#include "semaphore.h"
#include "./synchronizer/inet_local.h"

unsigned short *add_short_to_array(unsigned short *array, unsigned short val);

struct PT_GLOB{
        SLOT *slot;
        struct Enum_Type *Addresses;
        sem_t *semaphore;
        unsigned char type;
	int window;
};


GLOBAL_KNOWLEDGE *perform_global_features(SLOT *slot, unsigned char type);
void free_slot_frame_type(SLOT *slot, unsigned char type);
void analyse_slot_add(SLOT *, void *, unsigned char,  unsigned char, Enum_Type *Enumerator, unsigned char *process_flag, unsigned char window_size, LOCAL_SOCKET *sock);
void *analyse_thread_IP(void *Some_Structure);

#define _SLOT_WRAP_
#endif
