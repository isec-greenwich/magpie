// Validation functions header file

#include "definitions.h"


#ifndef _VALIDATIONS_
unsigned char validate_flags(unsigned char args, unsigned short argument_flags);
unsigned char validate_line_zigbee(char *line);
unsigned char validate_line_wifi(char *line);
unsigned char validate_line_wifi_eapol(char *line);
unsigned char validate_ip_short(char *line);
#define _VALIDATIONS_
#endif
