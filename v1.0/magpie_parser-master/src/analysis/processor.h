// Processor functions header file
#ifndef _ANALYSIS_PROCESS_

#include "windowing.h"
#include "math_func.h"
#include "../enumerator.h" 
#include "../int_wifi.h"
#include "../int_ipshort.h"
#include "../int_zbee.h"

void cpu_wifi_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator);
void cpu_ip_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator);
void cpu_zbee_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator);
void pro_audio(SLOT *slot);
void pro_zbee(SLOT *slot, GLOBAL_KNOWLEDGE *glob);
void pro_wifi(SLOT *slot, GLOBAL_KNOWLEDGE *glob);
void pro_ip_short(SLOT *slot, GLOBAL_KNOWLEDGE *glob);
void process_slot(SLOT *slot, GLOBAL_KNOWLEDGE *glob, unsigned char type);

#define _ANALYSIS_PROCESS_
#endif
