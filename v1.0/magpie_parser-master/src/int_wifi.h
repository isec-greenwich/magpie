// Interface Wifi 802.11 functions header file
#include "definitions.h"
#include "formatting.h"
#include "fileutils.h"
#include "epoch_conv.h"
#include "validations.h"

#ifndef _INT_WIFI_

typedef struct wifi_struct_internal{
        uint64_t timestamp;
        unsigned short src_mac;
        unsigned short dst_mac;
        unsigned int len;
        unsigned short type;
        unsigned short subtype;
        int rssi;
}wifi_struct_internal;

void pro_wifi_int(char *line, wifi_struct_internal *wifi_frm, Enum_Type *Enumerator_Address);
void free_wifi_struct(wifi_struct_internal *wifi_frm);
void process_wifi_frame(char *line, WiFi_Frame *wifi_frm, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto);
WiFi_Frame **process_wifi_lines(char *ptr, unsigned long lines, unsigned int *filtered, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto);
void process_wifi_file_input_live(unsigned char live_mode, char *line_buffer, char *out_filename_ptr, unsigned short argument_flags, char *args, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto);
#define _INT_WIFI_
#endif
