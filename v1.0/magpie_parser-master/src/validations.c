#include "validations.h"

unsigned char validate_flags(unsigned char args, unsigned short argument_flags){
        unsigned char res = 0;
        // If live is set, then output is necessary
        if ((argument_flags & LIVE_FLAG)) if ((argument_flags & OU_F_FLAG) || (argument_flags & STDOUT_FL)) res = 1;
        // If Input is set, then output is necessary
        if ((argument_flags & IN_F_FLAG)) if ((argument_flags & OU_F_FLAG)) res = 1;
        // If Auto, then no zigbee or wifi
        if ((argument_flags & AMODEFLAG)) if (!(argument_flags & ZIGB_FLAG) || !(argument_flags & WIFI_FLAG)) res = 1;
        // If Zigbee, then no wifi or auto
        if ((argument_flags & ZIGB_FLAG)) if (!(argument_flags & AMODEFLAG) || !(argument_flags & WIFI_FLAG)) res = 1;
        // If Wifi, then no auto or zigbee
        if ((argument_flags & WIFI_FLAG)) if (!(argument_flags & ZIGB_FLAG) || !(argument_flags & AMODEFLAG)) res = 1;
        // if IP Short, then no auto or zigbee or wifi
        if ((argument_flags & IP_SHOR_F)) if (!(argument_flags & ZIGB_FLAG) || !(argument_flags & WIFI_FLAG) || !(argument_flags & AMODEFLAG)) res = 1;
        return res;
}

unsigned char validate_line_zigbee(char *line){
        if(line == 0x00) return 0x00;
        while(*line++ != '\0'){
                if (*line == 'I'){
                        if (*(line+1) == 'E'){
                                if (*(line+2) == 'E') return 1; // IEEE
                        }
                }
                if (*line == 'Z'){
                        if (*(line+1) == 'i'){
                                if (*(line+2) == 'g') return 1; // IEEE
                        }
                }
        }
        return 0x00;
}

unsigned char validate_line_wifi(char *line){
        if (line == 0x00) return 0x00;
        while(*line++ != '\0'){
                if (*line == '8'){
                        if(*(line+1) == '0'){
                                if (*(line+2) == '2'){
                                        if(*(line+3) == '.'){
                                                if(*(line+4) == '1'){
                                                        if(*(line+5) == '1'){
                                                                return 1;
                                                        }
                                                }
                                        }
                                }
                        }
                }
        }
        return 0x00;
}
unsigned char validate_line_wifi_eapol(char *line){
        if (line == 0x00) return 0x00;
        while(*line++ != '\0'){
                if (*line == 'E'){
                        if (*(line+1) == 'A'){
                                if (*(line+2) == 'P'){
                                        if (*(line+3) == 'O'){
                                                if(*(line+4) == 'L'){
//                                                        eapol_flag = 1;
                                                        return 1;
                                                }
                                        }
                                }
                        }
                }
        }
        return 0;
}

unsigned char validate_ip_short(char *line){
        unsigned char validate = 0;
        while(*line++ != '\0'){
                if (*line == '.') validate++;
                if (validate == 7) return 1;
                // if arp break
                if (*line == 'A'){
                        if(*(line+1) == 'R'){
                                if (*(line+2) == 'P'){
                                        return 0;
                                }
                        }
                }
                // if MDNS the case of IPv6
                if (*line == 'M'){
                        if(*(line+1) == 'D'){
                                if (*(line+2) == 'N'){
                                        if (*(line+3) == 'S'){
                                                return 0;
                                        }
                                }
                        }
                }
        }
        return 0;

}

