#include "int_wifi.h"

void free_wifi_struct(wifi_struct_internal *wifi_frm){
//	free(wifi_frm->src_mac);
//	free(wifi_frm->dst_mac);
}

void pro_wifi_int(char *line, wifi_struct_internal *wifi_frm, Enum_Type *Enumerator_Addr){
	// process one line
	// in:	1529703131.189647282,70:56:81:88:e2:59,8c:0d:76:64:69:2d,623,2,32,-61,Evening,Friday,0
	unsigned char i = 0;
	char *o_ptr = line; // operating ptr
	char *i_ptr = line; // index ptr

	while(*o_ptr++ != ',')i++;
	i_ptr = o_ptr;
	line[i] = 0;
	i = 0;
	wifi_frm->timestamp = (atof(line)*(double)1000000.0); // gen timestamp ms precision

	while(*o_ptr++ != ',')i++;
	i_ptr[i] = 0;
	wifi_frm->src_mac = enum_find_frame_type(i_ptr, Enumerator_Addr);
	if (wifi_frm->src_mac == 0xFFFF){ // not found
                wifi_frm->src_mac = enum_add(i_ptr, Enumerator_Addr);
        }
	i_ptr = o_ptr;
	i = 0;

	while(*o_ptr++ != ',')i++;
	i_ptr[i] = 0;
	wifi_frm->dst_mac = enum_find_frame_type(i_ptr, Enumerator_Addr);
	if (wifi_frm->dst_mac == 0xFFFF){ // not found
        	wifi_frm->dst_mac = enum_add(i_ptr, Enumerator_Addr);
        }
	i_ptr = o_ptr;
	i = 0;

	while(*o_ptr++ != ',')i++;	//len
	i_ptr[i] = 0;
	wifi_frm->len = atoi(i_ptr);
	i_ptr = o_ptr; i = 0;

	while(*o_ptr++ != ',')i++;	// type
	i_ptr[i] = 0;
	wifi_frm->type = atoi(i_ptr);
	i_ptr = o_ptr; i = 0;

	while(*o_ptr++ != ',')i++;	// subtype
	i_ptr[i] = 0;
	wifi_frm->subtype = atoi(i_ptr);
	i_ptr = o_ptr; i = 0;

	while(*o_ptr++ != ','){
		if ((*o_ptr == '\n') || (*o_ptr == 0)) break;
		i++;
	}	// rssi last/end/cut
	i_ptr[i] = 0;
	wifi_frm->rssi = atoi(i_ptr);
	i_ptr = o_ptr; i = 0;
}

void process_wifi_frame(char *line, WiFi_Frame *wifi_frm, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto){
        // use int to store src and dst
        char *t_ptr = line;

        while(*t_ptr++ != ' '); // skip the number
        while(*t_ptr++ != ' '); // skip the date
        while(*t_ptr++ != ' '); // skip the time
        if (*t_ptr == ' ' ){
                while(*t_ptr++ == ' '); // skip spaces
                t_ptr--;
        }
        // compensate the ++
        // do the stuff, extract the source
        // extract source mac
        if ((*t_ptr == (char)0x86) || (*t_ptr == (char)0x3e)){ // no source found, arrow
                printf("\nWe have this unhandled case!");
        } else {
                //t_ptr++; //
                char *start_mac = t_ptr;
                char *end_mac = t_ptr;
                unsigned char i = 0;
                while(*end_mac++ != ' ')i++; // get to the end of the mac address
                        // copy the mac address and write down descriptor
                char *source_mac = (char*)calloc(1, i+1); // FREE
                char *destin_mac = 0x00;
                memcpy(source_mac, start_mac, i);
                t_ptr += i+1; // +1 skip space
                while(*t_ptr++ != ' ');
                i = 0;
                start_mac = t_ptr;
                end_mac = t_ptr;
                while(*end_mac++ != ' ') i++;
                destin_mac = (char*)calloc(1, i+1);
                memcpy(destin_mac, start_mac, i);
                // extracted source and dest
                wifi_frm->src_id = enum_find_frame_type(source_mac, Enumerator_Addr);
                wifi_frm->dst_id = enum_find_frame_type(destin_mac, Enumerator_Addr);
                if (wifi_frm->src_id == 0xFFFF){ // not found
                        wifi_frm->src_id = enum_add(source_mac, Enumerator_Addr);
                }
                if (wifi_frm->dst_id == 0xFFFF){ // not found
                        wifi_frm->dst_id = enum_add(destin_mac, Enumerator_Addr);
                }
                free(source_mac);
                free(destin_mac);
                        // we have timestamp, src_id, dst_id, skip to len, then
                //special EAPOL packet case
                //
                while(*t_ptr++ != ' '); // get to padding
                while(*t_ptr++ == ' '); // skipp padding
                while(*t_ptr++ != ' '){
                        //if (eapol_flag == 1){
                                t_ptr -= 2;
                                if(manage_comparison(t_ptr, ' ', "EAPOL ")){
                                        start_mac = t_ptr;
                                        end_mac = t_ptr;
                                        i = 0;
                                        while(*end_mac++ != ' ') i++; // get to the end
                                        source_mac = (char*)calloc(1, i+1); // aloc
                                        memcpy(source_mac, start_mac, i);
                                        wifi_frm->frame_type = enum_find_frame_type(source_mac, Enumerator_Proto);
                                        if (wifi_frm->frame_type == 0xFFFF){
                                               wifi_frm->frame_type = enum_add(source_mac, Enumerator_Proto);
                                        }
                                        free(source_mac); // frr
                                        t_ptr += i+1;
                                        start_mac = t_ptr;
                                        end_mac = t_ptr;
                                        i = 0;
                                        while(*end_mac++ != ' ') i++; // example ...f3:4e:66:e6 EAPOL 151 Key (Message 1 of 4)00
                                        source_mac = (char*)calloc(1, i+1); // aloc
                                        memcpy(source_mac, start_mac, i);
                                        wifi_frm->frame_length = atoi(source_mac);
                                        free(source_mac); // frr
                                        //eapol_flag = 0;
                                        return;
                                }
				t_ptr += 2;
                        //}
                } // skip 802.11
                // reusing variables
                start_mac = t_ptr;
                end_mac = t_ptr;
                i = 0;
                while(*end_mac++ != ' ')i++;
                source_mac = (char*)calloc(1, i+1); // allocate mem for length
                memcpy(source_mac, start_mac, i);
                wifi_frm->frame_length = atoi(source_mac);
                free(source_mac); // free allocated memory
                t_ptr += i+1;
                // got the length
                        // frame types
                start_mac = t_ptr;
                end_mac = t_ptr;
                i = 0;
                while(*end_mac++ != ',') i++;
                source_mac = (char*)calloc(1, i+1); // aloc
                memcpy(source_mac, start_mac, i);
                wifi_frm->frame_type = enum_find_frame_type(source_mac, Enumerator_Proto);
                if (wifi_frm->frame_type == 0xFFFF){
                        wifi_frm->frame_type = enum_add(source_mac, Enumerator_Proto);
                }
                free(source_mac); // frr
                // stored frame type
                        // extract SN
                while(*t_ptr++ != '=');
                start_mac = t_ptr;
                end_mac = t_ptr;
                i=0;
                while(*end_mac++ != ','){ i++; if (*end_mac == ' '){ i--; break; }}
                source_mac = (char*)calloc(1, i+1);
                memcpy(source_mac, start_mac, i);
                wifi_frm->frame_sn = atoi(source_mac);
                free(source_mac);
                t_ptr += i+1;
                        // extract FN
                while(*t_ptr++ != '=');
                start_mac = t_ptr;
                end_mac = t_ptr;
                i=0;
                while(*end_mac++ != ' '){ i++; if (*end_mac == ' '){ i--; break; }}
                source_mac = (char*)calloc(1, i+1);
                memcpy(source_mac, start_mac, i);
                wifi_frm->frame_fn = atoi(source_mac);
                free(source_mac);
                t_ptr += i+1;
                        // process Flags if needed, for version 2 etc...
                        // pointer currently is pointing at flags
                while(*t_ptr++ != '\n'){ if (*t_ptr == '\0'){ return; } else if (*t_ptr == ','){ break; } }
                // hacky solution to test ssid and store
                while(*t_ptr++ != 'S'){
                        if ( *t_ptr == '\n' ||
                                *t_ptr == '\0' ||
                                *t_ptr == '\r'){
                                // This case means that end of file or line, self-destruct )
                                return;
                        }
                }
                if (strncmp(t_ptr, "SID", 3) != 0) return; // it is not SSID it means return, failure
                // If I'm here I got the SSID
                t_ptr += 4; // Skip to number taking into account S[SID=]Mega
                start_mac = t_ptr;
                end_mac = t_ptr;
                i=0;
                while(*end_mac++ != '\0') i++;// if (*end_mac == '\0'){ i--; break; }}
                source_mac = (char*)calloc(1, i+1);
                memcpy(source_mac, start_mac, i);
                wifi_frm->bssid = source_mac;
                wifi_frm->bssid_len = i;
        }
}

WiFi_Frame **process_wifi_lines(char *ptr, unsigned long lines, unsigned int *filtered, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto){
        char *line = ptr;
        char **t_upd = (char**)calloc(1, sizeof(t_upd));
        WiFi_Frame **wifi_arr = (WiFi_Frame**)calloc(lines, sizeof(wifi_arr));
        int wifi_arr_i = 0;
        unsigned int l_count = 0;
        while(l_count < lines){
                //printf("\n%08x\n", line);
                line = extract_line(line, t_upd);
                //printf("\nExtracted Line: %s", line);
                if (!validate_line_wifi(line)){
                        if (line == 0x00){
                //              printf("\nline==0x00\n");
                                free(line);
                                l_count++;
                                line = t_upd[0];
                                break;
                        }
                //      printf("line=t_upd\n");
                        if(!validate_line_wifi_eapol(line)){
                                free(line);
                                l_count++;
                                line = t_upd[0];
                                continue;
                        }
//                        free(line); l_count++;
//                        line = t_upd[0];
//                        continue;
                }
        //      printf("\nValidation went fine");
                wifi_arr[wifi_arr_i] = (WiFi_Frame*)calloc(1, sizeof(WiFi_Frame));
                wifi_arr[wifi_arr_i]->timestamp = convert_date_to_epoch(line);
                process_wifi_frame(line, wifi_arr[wifi_arr_i], Enumerator_Addr, Enumerator_Proto);
                free(line);
                line = t_upd[0];
                wifi_arr_i++;
                l_count++;
        }
        *filtered = wifi_arr_i;
        free(t_upd);
        return wifi_arr;
}

void process_wifi_file_input_live(unsigned char live_mode, char *line_buffer, char *out_filename_ptr, unsigned short argument_flags, char *args, Enum_Type *Enumerator_Addr, Enum_Type *Enumerator_Proto){
        unsigned int line_count = 0;
        unsigned int filtered = 0;
	int live_descriptor_write = 0;
        WiFi_Frame **wifi_arr_ptr= process_wifi_lines(line_buffer, 1, &filtered, Enumerator_Addr, Enumerator_Proto);
        if (filtered != 0xFFFFFFFF) write_out_frames_new((void*)wifi_arr_ptr, 1,'\0', line_count, out_filename_ptr, argument_flags, args);
	if ((filtered != 0xFFFFFFFF) && (live_descriptor_write)){write_descriptor(Enumerator_Proto, "wifi_protocols.csv", out_filename_ptr, argument_flags, args); write_descriptor(Enumerator_Addr, "wifi_addresses.csv", out_filename_ptr, argument_flags, args); live_descriptor_write = 0;}
}

