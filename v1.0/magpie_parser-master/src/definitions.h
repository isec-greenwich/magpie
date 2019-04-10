#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fstream>

#ifndef _DEFINITIONS_ 

// Main definitions file

#define LIVE_FLAG  1<<0 // Live feed
#define IN_F_FLAG  1<<1 // Input filename parameter
#define OU_F_FLAG  1<<2 // Out filename parameter
#define WIFI_FLAG  1<<3 // Wifi mode
#define ZIGB_FLAG  1<<4 // Zigbee mode
#define AMODEFLAG  1<<5 // Auto mode, identify whether ZigBee or Wifi
#define HELP_FLAG  1<<6 // Help
#define PARAMETERF 1<<7 // Parameter for input/output is parsed next
#define IP_SHOR_F  1<<8 // IP Short version
#define AUDIO_FLA  1<<9 // Audio flag in a format of <timestamp>,<value>
#define STDOUT_FL  1<<10 // --stout equivalent
#define SPECT_FLA  1<<11 // spectrum analysis
#define WINDO_FLA  1<<12 // window provided
#define WINDOW_VA  1<<13 // Window value
#define SYNCHRONI  1<<14 // Synchronize
#define CPU_TIMES  1<<15 // Write out cpu times
#define WIN_MODE   1
#define LINE_INIT  100000 // Lines initialized


// Structure definitions

typedef struct ZigBee_Frame{
	uint64_t timestamp;
	unsigned short src_id;
	unsigned short dst_id;
	unsigned char frame_type;
	unsigned char packet_size;
	unsigned char flags; // 1 Bad_Checksum
}ZigBee_Frame;
typedef struct IP_Frame{
	uint64_t timestamp;
	unsigned short src_ip;
	unsigned short dst_ip; // Potentially can be switched to short, analysis is required.
	unsigned short protocol;
	unsigned short packet_size;
}IP_Frame;

typedef struct Audio_Frame{
	uint64_t timestamp;
	double db;
}Audio_Frame;

typedef struct WiFi_Frame{
	uint64_t timestamp;
	unsigned short src_id;
	unsigned short dst_id;
	unsigned char  bssid_len;
	char*   bssid;
	unsigned char essid_len;
	char*   essid;
	unsigned short frame_type;
	unsigned short frame_length;
	unsigned short frame_sn;
	unsigned char  frame_fn;
}WiFi_Frame;

typedef struct Timestamp{
        unsigned long timestamp_s;
        unsigned long timestamp_ms;
}Timestamp;

// End of structure definitions
#define _DEFINITIONS_
#endif
