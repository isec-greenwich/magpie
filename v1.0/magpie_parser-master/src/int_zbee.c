#include "int_zbee.h"

void ieee_man_frames_handler(char *after_length_ptr, ZigBee_Frame *zb_object, Enum_Type *Enumerator){
        char *t_ptr = after_length_ptr;
        while(*t_ptr != '\0'){
                if (*t_ptr++ == ','){
                        t_ptr -=1; // compensate space
                        // get the value from after_length_ptr to t_ptr
                        zb_object->frame_type = manage_enumerations(Enumerator, after_length_ptr, t_ptr);
                        // compensate ", "
                        t_ptr +=2;
                        //reusing all local variables
                        if (!(zb_object->flags & 1))zb_object->flags |= manage_comparison(t_ptr, '\0', "Bad FCS");
                        // quickly find Bad FCS
                        if (zb_object->flags == 0){
                                // test special cases
                                while(*t_ptr++ != '\0'){
                                        //look for B
                                        if (*t_ptr == 'B'){
                                                if (*(t_ptr+1) == 'a'){
                                                        // at this point it will be Ba(d FCS)
                                                        zb_object->flags = 1;
                                                }
                                        }
                                }
                        }
                        break;
                }
                if (*t_ptr == '\0'){
                        zb_object->frame_type = manage_enumerations(Enumerator, after_length_ptr, t_ptr);;
                        break;
                }
        }
}


void zbee_ieee_handler(char *line, zbee_struct_internal *zbee_frm, Enum_Type *Enumerator){
	unsigned int index = 14; // to point at len
	unsigned int ind = 0;
	// get length
	while(line[index+ind] != ' ') ind++;
	line[index+ind] = '\0';
	zbee_frm->len = atoi(&line[index]);
	line[index+ind] = ' '; index += ind+1; // compensating space
	// classify the enumerator pkt types
	ind = 0;
	while(1){ ind++; if ((line[index+ind] == '\0') || (line[index+ind] == '\n') || (line[index+ind] == ',')) break;}
	if ( (line[index+ind] == ',') ){ // check for bad fcs
		line[index+ind] = '\0';
		zbee_frm->pkt_type = enum_find_frame_type(&line[index], Enumerator);
		if (zbee_frm->pkt_type == 0xFFFF){ // not found
			zbee_frm->pkt_type = enum_add(&line[index], Enumerator);
		}
		line[index+ind] = ',';
		index += ind+1;
		if (strncmp(&line[index], "Bad FCS", 7) == 0){ // case 0xb10c → 0x1177       IEEE 802.15.4 26 Unknown Command, Bad FCS
			zbee_frm->fcs = 1;
		} else {
			ind = 0;
			while(1){
				ind++;
				if ( (line[index+ind] == '\0') || (line[index+ind] == '\n') ){
					line[index+ind] = '\0';
					if (strncmp(&line[index+ind-7], "Bad FCS", 7) == 0){
						zbee_frm->fcs = 1;
						break;
					}  else if (strncmp(&line[index+ind-18], "[Malformed Packet]", 18) == 0){ //  300 2018-07-22 17:28:59.222173473              → 4d:d4:8c:f4:f5:55:55:55 IEEE 802.15.4 26 Multipurpose, Dst: 4d:d4:8c:f4:f5:55:55:55[Malformed Packet]
						zbee_frm->fcs = 1;
						break;
					} else {
						fprintf(stderr, "Unhandled complex else condition in pro_zbee_int (int_zbee.c)\nLine: %s", line);
						break;
					}
				}
			}
		}
	} else { // terminated with null or new line
		line[index+ind] = '\0';
		zbee_frm->pkt_type = enum_find_frame_type(&line[index], Enumerator);
		if (zbee_frm->pkt_type == 0xFFFF){ // not found
			zbee_frm->pkt_type = enum_add(&line[index], Enumerator);
		}	
	}
}

void zbee_zigbee_handler(char *line, zbee_struct_internal *zbee_frm, Enum_Type *Enumerator){
	unsigned int index = 7; // to point at len
	unsigned int ind = 0;
	// get length
	while(line[index+ind] != ' ') ind++;
	line[index+ind] = '\0';
	zbee_frm->len = atoi(&line[index]);
	line[index+ind] = ' '; index += ind+1; // compensating space
	// classify the enumerator pkt types
	//  1228 2018-07-22 17:45:24.261615352       0x1c7c → Broadcast    ZigBee 64 Command, Dst: Broadcast, Src: 0x1c7c
	ind = 0;
	while(1){ ind++; if ((line[index+ind] == '\0') || (line[index+ind] == '\n') || (line[index+ind] == ',')) break;}
	if ( (line[index+ind] == ',') ){ // check for bad fcs
		line[index+ind] = '\0';
		zbee_frm->pkt_type = enum_find_frame_type(&line[index], Enumerator);
		if (zbee_frm->pkt_type == 0xFFFF){ // not found
			zbee_frm->pkt_type = enum_add(&line[index], Enumerator);
		}
		line[index+ind] = ',';
		index += ind+1;
		if (strncmp(&line[index], "Bad FCS", 7) == 0){ // case 0xb10c → 0x1177       IEEE 802.15.4 26 Unknown Command, Bad FCS
			zbee_frm->fcs = 1;
		}
		
	} else { // terminated with null or new line
		line[index+ind] = '\0';
		zbee_frm->pkt_type = enum_find_frame_type(&line[index], Enumerator);
		if (zbee_frm->pkt_type == 0xFFFF){ // not found
			zbee_frm->pkt_type = enum_add(&line[index], Enumerator);
		}	
	}
}

void pro_zbee_int(char *line, zbee_struct_internal *zbee_frm, Enum_Type *Enumerator){
		unsigned int index = 0;
		unsigned int ind = 0;
		if (strcmp(line, "\n") == 0) {free(zbee_frm); zbee_frm = 0; return;}
	    // extract epoch time
		// example     1 2018-07-22 17:24:11.316703005       0xd344 → 0x0000       IEEE 802.15.4 26 Data Request
	    if (line[index] == ' '){ while(line[index] == ' ')index++; while(line[index] != ' ')index++; index++; }// skip the number and spacing afterwards; should be in position of 2018-07...005
		while(line[index+ind] != ' ') ind++; // skip the date
		ind++; // compensate the space
		while(line[index+ind] != ' ') ind++; // skip the time with seconds;
		
		char *timestamp = (char*)calloc(ind+1, sizeof(char));
		memcpy(timestamp, &line[index], ind);
		zbee_frm->timestamp = date_to_epoch(timestamp);
		free(timestamp);
		index = ind+index;
	    // proceed with processing
	    // debug r -l -z -t < ../../dataset/zb_sample.csv
	    while(line[index] == ' ') index++; // go to 0xd344 or -> symbol
	    if (strncmp(&line[index], "\xe2\x86\x92", 3) == 0){ // due to the formatting
				index += 4;
				while(line[index] == ' ') index++;
				ind = 0;
				
				zbee_frm->src_id = 0xFFFF;
				// symbol it means no source address, coming from broadcast
				while(line[index+ind] == ' ') ind++;
				if (((line[index+ind] == '0') && (line[index+ind+1] == 'x')) || (line[index+ind] == 'B')){
					if ((line[index+ind] == 'B')) zbee_frm->dst_id = 0xFFFF; // Broadcast
					// convert address or broadcast to short 
				} else {
					// check if IEEE
					if (strncmp(&line[index+ind], "IEEE 802.15.4", 13) == 0){
						// data request/ack packet or other
						// if this has case has been captured i.e. no dst id
						zbee_frm->dst_id = 0xFFFF;
					} else if (strncmp(&line[index+ind], "ZigBee", 6) == 0){
						zbee_zigbee_handler(&line[index], zbee_frm, Enumerator);
						// process as zigbee packet
					} else {
						// extended packet 4d:d4:8c:f4:f5:55:55:55 IEEE 802.15.4 26 Multipurpose, Dst: 4d:d4:8c:f4:f5:55:55:55[Malformed Packet]\n"
						while(line[index] != ' ') index++;
						index++;
						if (strncmp(&line[index], "IEEE 802.15.4", 13) == 0){
							zbee_frm->dst_id = 0xFFFE;
							zbee_ieee_handler(&line[index], zbee_frm, Enumerator);							
						} else if (strncmp(&line[index], "ZigBee", 6) == 0){
							zbee_frm->dst_id = 0xFFFE;
							zbee_zigbee_handler(&line[index], zbee_frm, Enumerator);							
						}
						//fprintf(stderr, "Captured else condition in pro_zbee_int (int_zbee.c) (check IEEE)\n");
					}
				}
				// check case → 0x0000 where 0x0000 doesn't exist
		} else if (strncmp(&line[index], "0x", 2) == 0){ // check if legit address 
				index += 2;
				line[index+4] = '\0';
				zbee_frm->src_id = (short)strtol(&line[index], NULL, 16);
				line[index+4] = ' '; index += 5;
				if (strncmp(&line[index], "\xe2\x86\x92", 3) == 0){ index += 4; } else { index += 2; }
				// if there 
				if (strncmp(&line[index], "0x", 2) == 0){
					index += 2;
					line[index+4] = '\0';
					zbee_frm->dst_id = (short)strtol(&line[index], NULL, 16);
					line[index+4] = ' '; index += 5;
					while(line[index] == ' ') index++;
				} else if (strncmp(&line[index], "Broadcast", 9) == 0){
					zbee_frm->dst_id = 0xFFFF;
					index += 9;
					while(line[index] == ' ') index++;
				} else {
					while(line[index] == ' ') index++;
					// No address found, reached IEEE/Zigbee
					if (strncmp(&line[index], "IEEE 802.15.4", 13) == 0){
						zbee_ieee_handler(&line[index], zbee_frm, Enumerator);
					} else if (strncmp(&line[index], "ZigBee", 6) == 0){
						zbee_zigbee_handler(&line[index], zbee_frm, Enumerator);
					} else {
						fprintf(stderr, "Unhandled else condition in pro_zbee_int (int_zbee.c)\n");
					}
				}
			
			if (strncmp(&line[index], "IEEE 802.15.4", 13) == 0){
				zbee_ieee_handler(&line[index], zbee_frm, Enumerator);

			} else if (strncmp(&line[index], "ZigBee", 6) == 0){
				zbee_zigbee_handler(&line[index], zbee_frm, Enumerator);
			} else {
				fprintf(stderr, "Unhandled else condition in pro_zbee_int (int_zbee.c)\n");
			}
			//printf(".");
		} else {
			fprintf(stderr, "Captured else condition in pro_zbee_int (int_zbee.c)\n");
		}
	   
	    
}

void zigb_man_frames_handler(char *after_length_ptr, ZigBee_Frame *zb_object, Enum_Type *Enumerator){
        char *t_ptr = after_length_ptr;
        while(*t_ptr != '\0'){
                if (*t_ptr++ == ','){
                        t_ptr -=1; // compensate space
                        // get the value from after_length_ptr to t_ptr
                        zb_object->frame_type = manage_enumerations(Enumerator, after_length_ptr, t_ptr);
                        // compensate ", "
                        t_ptr +=2;
                        //reusing all local variables
                        if (!(zb_object->flags & 1))zb_object->flags |= manage_comparison(t_ptr, '\0', "Bad FCS");
                        break;
                }
                if (*t_ptr == '\0'){
                        zb_object->frame_type = manage_enumerations(Enumerator, after_length_ptr, t_ptr);
                        break;
                }
        }
}

unsigned int exctract_source_id(char *line, ZigBee_Frame *zb_object, Enum_Type *Enumerator){
        // use int to store src and dst
        unsigned short src = 0;
        unsigned short dst = 0;
        unsigned int srcdst= 0;
        char *t_ptr = line;

        while(*t_ptr++ != ' '); // skip the number
        while(*t_ptr++ != ' '); // skip the date
        while(*t_ptr++ != ' '); // skip the time
        while(*t_ptr++ == ' '); // skip spaces
        // do the stuff, extract the source
        if (*t_ptr == (char)0x86 || *t_ptr == (char)0x3e){ // 0xe2 0x86 (arrow) (No addressess provided) (check if IEEE 802.15.4)
                t_ptr++;
                //0x86 compensate
                while(*++t_ptr == ' '); // go to next field IEEE, ZigBee
                unsigned short bcast = 0x0000;
                if (*t_ptr == 'B'){ bcast = 0xFFFF; while(*++t_ptr != ' '); while(*++t_ptr == ' ');}
                char ieee[5] = {0, 0, 0, 0, 0};
                char zigb[7] = {0, 0, 0, 0, 0, 0 ,0};
                char strd[9] = {0, 0, 0, 0, 0, 0 ,0, 0, 0};
                memcpy(ieee, t_ptr, sizeof(char)*4);
                if (strcmp(ieee, "IEEE") == 0){
                        // IEEE
                        while(*t_ptr++ != ' ');
                        memcpy(strd, t_ptr, sizeof(char)*8);
                        if (strcmp(strd, "802.15.4") == 0){
                                // source
                                srcdst = 0; // no addresses involved i.e. src and dst = 0;
                                while(*t_ptr++ != ' '); //shift to text
                                char *_diff = t_ptr;
                                while(*_diff++ != ' ');
                                unsigned char diff_v = (_diff-t_ptr);
                                char *len = (char*)calloc(diff_v+1, sizeof(char));
                                memcpy(len, t_ptr, sizeof(char)*diff_v);
                                unsigned char ln = (char)atoi(len);
                                t_ptr += diff_v;
                                zb_object->src_id = 0x0000;
                                zb_object->dst_id = 0x0000;
                                if (bcast != 0) zb_object->dst_id = bcast;
                                zb_object->packet_size = ln;
                                // Ack Packet handler
                                if ((zb_object->packet_size == 19) && (manage_comparison(t_ptr, '\0', "Ack") == 1)){
                                        zb_object->src_id = 0xFFFF;
                                        zb_object->dst_id = 0xFFFF;
                                }
                                if ((zb_object->src_id == 0x0000) && (zb_object->dst_id == 0x0000)){
                                        zb_object->flags = 1; // Reserved[Malformed Packet] case
                                }
                                ieee_man_frames_handler(t_ptr, zb_object, Enumerator);
                                free(len);
                                //extract the length
                                //extract the frame type
                        }
                } else {
                        memcpy(zigb, t_ptr, sizeof(char)*6);
                        if (strcmp(zigb, "ZigBee") == 0){
                                // ZigBee
                                srcdst = 0; // no addresses involved i.e. src and dst = 0;
                                while(*t_ptr++ != ' '); //shift to text
                                char *_diff = t_ptr;
                                while(*_diff++ != ' ');
                                unsigned char diff_v = (_diff-t_ptr);
                                char *len = (char*)calloc((_diff-t_ptr)+1, sizeof(char));
                                memcpy(len, t_ptr, sizeof(char)*(_diff-t_ptr));
                                unsigned char ln = (char)atoi(len);
                                t_ptr += diff_v;
                                zb_object->src_id = src;
                                zb_object->dst_id = dst;
                                if (bcast != 0) zb_object->dst_id = bcast;
                                zb_object->packet_size = ln;
                                if ((zb_object->src_id == 0x0000) && (zb_object->dst_id == 0x0000)){
                                        zb_object->flags = 1; // Reserved[Malformed Packet] case
                                }
                                zigb_man_frames_handler(t_ptr, zb_object, Enumerator);
                                free(len);
                        }
                }
        } else if (*t_ptr == 'x'){ // i.e. 0x
                t_ptr++;
                char *src_p = (char*)calloc(5, sizeof(char));
                char *dst_p = (char*)calloc(5, sizeof(char));
                memcpy(src_p, t_ptr, sizeof(char)*4);
                src = (short)strtol(src_p, NULL, 16);
                while(1){
                        if (*t_ptr == 'x') break;
                        if (*t_ptr == 'B') break;
                        if (*t_ptr == 'Z'){t_ptr=t_ptr-3; *t_ptr='B'; t_ptr--; break; } //  17:51:57.945093901       0x0000 →            ZigBee 42 Beacon, S... case
                        //if (*t_ptr == '\0') return;
                        t_ptr++; // while not other address or Broadcast || (*t_ptr != 'B')
                }
                if((*t_ptr++) == 'x'){
                        memcpy(dst_p, t_ptr, sizeof(char)*4);
                        dst = (short)strtol(dst_p, NULL, 16);
                } else { // Broadcast
                        dst = 0xFFFF;
                }
                srcdst = (src<<16) | dst;
                while(*++t_ptr != ' ');
                while(*++t_ptr == ' ');
                char ieee[5] = {0, 0, 0, 0, 0};
                char zigb[7] = {0, 0, 0, 0, 0, 0 ,0};
                char strd[9] = {0, 0, 0, 0, 0, 0 ,0, 0, 0};
                memcpy(ieee, t_ptr, sizeof(char)*4);
                if (strcmp(ieee, "IEEE") == 0){
                        // IEEE
                        while(*t_ptr++ != ' ');
                        memcpy(strd, t_ptr, sizeof(char)*8);
                        if (strcmp(strd, "802.15.4") == 0){
                                // source
                                srcdst = 0; // no addresses involved i.e. src and dst = 0;
                                while(*t_ptr++ != ' '); //shift to text
                                char *_diff = t_ptr;
                                while(*_diff++ != ' ');
                                unsigned char diff_v = (_diff-t_ptr);
                                char *len = (char*)calloc((_diff-t_ptr)+1, sizeof(char));
                                memcpy(len, t_ptr, sizeof(char)*(_diff-t_ptr));
                                unsigned char ln = (char)atoi(len);
                                t_ptr += diff_v;
                                zb_object->src_id = src;
                                zb_object->dst_id = dst;
                                zb_object->packet_size = ln;
                                if ((zb_object->src_id == 0x0000) && (zb_object->dst_id == 0x0000)){
                                        zb_object->flags = 1; // Reserved[Malformed Packet] case
                                }
                                ieee_man_frames_handler(t_ptr, zb_object, Enumerator);
                                free(len);
                                //extract the length
                                //extract the frame type
                        }
                } else {
                        memcpy(zigb, t_ptr, sizeof(char)*6);
                        if (strcmp(zigb, "ZigBee") == 0){
                                // ZigBee
                                srcdst = 0; // no addresses involved i.e. src and dst = 0;
                                while(*t_ptr++ != ' '); //shift to text
                                char *_diff = t_ptr;
                                while(*_diff++ != ' ');
                                unsigned char diff_v = (_diff-t_ptr);
                                char *len = (char*)calloc((_diff-t_ptr)+1, sizeof(char));
                                memcpy(len, t_ptr, sizeof(char)*(_diff-t_ptr));
                                unsigned char ln = (char)atoi(len);
                                t_ptr += diff_v;
                                zb_object->src_id = src;
                                zb_object->dst_id = dst;
                                zb_object->packet_size = ln;
                                if ((zb_object->src_id == 0x0000) && (zb_object->dst_id == 0x0000)){
                                        zb_object->flags = 1; // Reserved[Malformed Packet] case
                                }
                                zigb_man_frames_handler(t_ptr, zb_object, Enumerator);
                                //printf("ZigBee!!! Length: %d\n", (int)ln);
                                free(len);
                        }
                }
                free(src_p);
                free(dst_p);
        }
        return srcdst;
}


ZigBee_Frame *convert_line_to_zb_header(ZigBee_Frame *frame, char *line){
        ZigBee_Frame *ret_ptr = (ZigBee_Frame*)calloc(1, sizeof(ZigBee_Frame));
        ret_ptr->timestamp = convert_date_to_epoch(line);
        ret_ptr->src_id = 0x0000;
        ret_ptr->dst_id = 0x0000;
        ret_ptr->packet_size = 0;
        ret_ptr->frame_type = 0;
        ret_ptr->flags = 0;

        return ret_ptr;
}

ZigBee_Frame **process_zigbee_lines(char *ptr, unsigned long lines, unsigned int *filtered, Enum_Type *Enumerator){
        char *line = ptr;
        char **t_upd = (char**)calloc(1, sizeof(t_upd));
        ZigBee_Frame **zb_arr = (ZigBee_Frame**)calloc(lines, sizeof(zb_arr));
        int zb_arr_i = 0;
        unsigned int l_count = 0;
        while(l_count < lines){
                //printf("\n%08x\n", line);
                line = extract_line(line, t_upd);
                //printf("\nExtracted Line: %s", line);
                if (!validate_line_zigbee(line)){
                        if (line == 0x00){
                //              printf("\nline==0x00\n");
                                free(line);
                                l_count++;
                                line = t_upd[0];
                                break;
                        }
                //      printf("line=t_upd\n");
                        free(line); l_count++;
                        line = t_upd[0];
                        continue;
                }
        //      printf("\nValidation went fine");
                zb_arr[zb_arr_i] = (ZigBee_Frame*)calloc(1, sizeof(ZigBee_Frame));
                zb_arr[zb_arr_i]->timestamp = convert_date_to_epoch(line);
                exctract_source_id(line, zb_arr[zb_arr_i], Enumerator);
                free(line);
                line = t_upd[0];
                zb_arr_i++;
                l_count++;
        }
        *filtered = --zb_arr_i;
        free(t_upd);
        return zb_arr;
}

void process_zigbee_file_input_live(unsigned char live_mode, char *line_buffer, char *out_filename_ptr, unsigned short argument_flags, char *args, Enum_Type *Enumerator){
        unsigned int line_count = 0;
        unsigned int filtered = 0;
	int live_descriptor_write = 0; // Find a solution
        ZigBee_Frame **zb_arr_ptr = process_zigbee_lines(line_buffer, 1, &filtered, Enumerator);
        if (filtered != 0xFFFFFFFF) write_out_frames_new((void*)zb_arr_ptr, 1,'\0', line_count, out_filename_ptr, argument_flags, args);
        fflush(stdout);
        if ((filtered != 0xFFFFFFFF) && (live_descriptor_write)){write_descriptor(Enumerator, "zbee_protocols.csv", out_filename_ptr, argument_flags, args); live_descriptor_write = 0;}
}
