#include "fileutils.h"

FILE *open_file(char *filename, const char *mode, char *arg){
        FILE *t = 0;
	char *path = extract_path(arg);
        char *full_path = concat_path_and_filename(filename, path);
        t = fopen(full_path, mode);
        free(full_path);
        return t;
}

uint64_t get_number_of_lines(FILE *file, uint64_t *line_offsets, uint64_t line_offset_index){
        uint64_t res = 0;
        int i = 0;
        int byte_read = 0;
        uint64_t pos = 0;
        char buff[4096];
	unsigned long line_offset_increments = LINE_INIT;
        while((byte_read = fread(buff, sizeof(char), sizeof(buff), file)) > 0){
                while(i < byte_read){
                        if (buff[i] == '\n'){
                                *(line_offsets+line_offset_index) = pos+1; // compensate '\n'
                                res++;
                                line_offset_index++;
                                if (line_offset_index == (line_offset_increments)){ // more than initial, allocate double, copy current, free old pointer
                                        uint64_t *new_ptr = (uint64_t*)calloc(((line_offset_increments*2)), sizeof(uint64_t));
                                        memcpy(new_ptr, line_offsets, sizeof(uint64_t)*line_offset_index);
                                        free(line_offsets);
                                        line_offsets = new_ptr;
                                        line_offset_increments *=2;
                                }
                        }
                        pos++;
                        i++;
                }
                i=0;
        }
        // clear out unneccessary memory
        if ((res) < (line_offset_increments*2)){
                uint64_t *new_ptr = (uint64_t*)calloc(line_offset_index, sizeof(uint64_t));
                //memcpy(new_ptr, line_offsets, sizeof(uint64_t)*line_offset_index);
                memcpy(new_ptr, line_offsets, sizeof(uint64_t)*line_offset_index);
                free(line_offsets);
                line_offsets = new_ptr;
        }
        /*while(_t != EOF){
                _t = (char)fgetc(file);
                if (_t == '\n') res++;
        }*/
        return res;
}

unsigned char load_maps(unsigned short argument_flags, Enum_Type *Enumerator, char *args){
        FILE *address_map;
        FILE *protocol_map;
        char fname_adr[] = "address_map.csv";
        char fname_pro[] = "protocol_map.csv";
        char buffer[1024];
        unsigned short addr;
        if ((access(fname_adr, F_OK) != -1) && (access(fname_pro, F_OK) != -1)){
                address_map = open_file(fname_adr, "r+", args);
                protocol_map= open_file(fname_pro, "r+", args);
                unsigned comma_index;
                while(fgets(buffer, sizeof(buffer), address_map)){
                        comma_index = 0;
                        char *t_ptr = (char*)buffer;
                        while(*t_ptr != 0){     // get to comma
                                if (*t_ptr == ','){ *t_ptr =0; t_ptr++; break;}
                                comma_index++;
                                t_ptr++;
                        }
                        addr = atoi(buffer);
                        char *tt_ptr = t_ptr;
                        while (t_ptr != 0){
                                if (*t_ptr == '\n'){ *t_ptr =0; break;}
                                t_ptr++;
                        }
                        if ((argument_flags & IP_SHOR_F) == IP_SHOR_F)enum_add_num(tt_ptr, Enumerator, addr);
                        if ((argument_flags & WIFI_FLAG) == WIFI_FLAG)enum_add_num(tt_ptr, Enumerator, addr);
                }
                while(fgets(buffer, sizeof(buffer), protocol_map)){
                        comma_index = 0;
                        char *t_ptr = (char*)buffer;
                        while(*t_ptr != 0){     // get to comma
                                if (*t_ptr == ','){ *t_ptr =0; t_ptr++; break;}
                                comma_index++;
                                t_ptr++;
                        }
                        addr = atoi(buffer);
                        char *tt_ptr = t_ptr;
                        while (t_ptr != 0){
                                if (*t_ptr == '\n'){ *t_ptr =0; break;}
                                t_ptr++;
                        }
                        if ((argument_flags & IP_SHOR_F) == IP_SHOR_F)enum_add_num(tt_ptr, Enumerator, addr);
                        if ((argument_flags & WIFI_FLAG) == WIFI_FLAG)enum_add_num(tt_ptr, Enumerator, addr);
                        if ((argument_flags & ZIGB_FLAG) == ZIGB_FLAG)enum_add_num(tt_ptr, Enumerator, addr);
                }
        }
        return 0;
}

char *extract_next_line(FILE *file, int *len){
        char *ret_ptr = 0;
        // allocate temp buffer
        char *buffer = (char*)calloc(1024, sizeof(char)); // max line length is 1024mlk_alloc++;
        char *buf_ptr = buffer;
        int i = 0;
        int cur_pos = 0;
        do{
                *buf_ptr++ = (char)fgetc(file);
                cur_pos++;
                if (*(buf_ptr-1) == '\n' || *(buf_ptr-1) == 0xFF){  cur_pos--; break; }
                if (*(buf_ptr-1) == EOF) { free(buffer); return 0x00; }
        }while(*(buf_ptr-1) != EOF);
        //trim spaces
        buf_ptr = buffer;
        while(*(buf_ptr+i++) == ' '); // point at not space char
        i--; buf_ptr += i;

        ret_ptr = (char*)calloc((cur_pos-i+1), sizeof(char)); // allocate sentence space
        memcpy(ret_ptr, buf_ptr, (cur_pos-i));
        free(buffer);
        return ret_ptr;
}

void write_descriptor(Enum_Type *Enum, const char *file, char *out_filename_ptr, unsigned short argument_flags, char *args){
//      char filename[64] = { 0 };
        int i = 0;
        while(*(file+i) != '\0')i++;
        int j = 0;
        while(*(out_filename_ptr+j) != '\0')j++;
        char *final = (char*)calloc(1, sizeof(char)*(i+j+2));
        if ((argument_flags & STDOUT_FL) != STDOUT_FL){
                memcpy(final, out_filename_ptr, j-4);
                memcpy((final+j-4), "_", 1);
                memcpy((final+j+1-4), file, i);
        } else {
                memcpy(final, file, i);
        }
//              memcpy(filename, file, i);
        FILE *desc_file = open_file(final, "w+", args);
        Enum_Type *ptr = Enum;
        // loop through all Enums
        char buf[128];
        unsigned char str_len = 0;
        while(ptr->name != 0x00){
                //sprintf_s(buf, "%d,%s\n", ptr->frame_type, ptr->name);
                snprintf(buf, sizeof(buf), "%d,%s\n", ptr->frame_type, ptr->name);
                while(buf[str_len++] != '\0');
                str_len--;
                fwrite(buf, str_len, 1, desc_file);
                ptr = ptr->next;
                str_len = 0;
        }
        free(final);
        fclose(desc_file);
}

char *extract_number_of_lines(FILE *file, long start_line_offset, long end_line_offset, uint64_t *line_offsets){
        start_line_offset = *(line_offsets+start_line_offset);
        end_line_offset = *(line_offsets+end_line_offset);
        long diff = (end_line_offset-start_line_offset);
        fseek(file, start_line_offset,SEEK_SET);
        char *ptr = (char*)calloc(diff+1, sizeof(char));
        fread(ptr, sizeof(char), diff, file);
        return ptr;
}
#include <inttypes.h>
void write_out_frames_new(void *Object, int num, char feedback_char, unsigned int overall_lines, char *out_filename_ptr, unsigned short argument_flags, char *args){
        if (feedback_char != '\0')printf("%c", feedback_char);
        int i = 0;
        if ( (  (argument_flags & (OU_F_FLAG | ZIGB_FLAG))    == (OU_F_FLAG | ZIGB_FLAG) ) ||
                ((argument_flags & (STDOUT_FL | ZIGB_FLAG))==(STDOUT_FL | ZIGB_FLAG) )){
                ZigBee_Frame **frames = (ZigBee_Frame**)Object;
                if (Object == 0x00){ free(Object); return;}
                //ZigBee_Frame *frames = (ZigBee_Frame*)Object;
                //int *arr_addr = (int*)Object;
                //int *addr = (int*)*arr_addr;
                //unsigned int *addr = (int*)*arr_addr;
                char frame_size = sizeof(ZigBee_Frame)-1; // -1 because of structure padding
                FILE *out_file;
                if ((STDOUT_FL & argument_flags) == STDOUT_FL){
                        out_file = stdout;
                } else {
                        out_file = open_file(out_filename_ptr, "r+b", args);
                        fseek(out_file, 0, SEEK_END);
                }
//              fseek(out_file, 0, SEEK_END);
                while(i < num){
                        //ZigBee_Frame *frame_ptr = (ZigBee_Frame*)*frames;
                        if( frames[i]->timestamp == 0) break;
                        //fwrite(&frame_size, sizeof(char), 1, out_file);
                        //fwrite(frames[i++], sizeof(ZigBee_Frame)-1,1, out_file); // -1 because of structure padding
			printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n", frames[i]->timestamp, frames[i]->src_id, frames[i]->dst_id, frames[i]->frame_type, frames[i]->packet_size, frames[i]->flags);
			fflush(stdout);
			i++;
                }
                if ((argument_flags & STDOUT_FL) != STDOUT_FL){
                        fseek(out_file, sizeof(int), SEEK_SET);
                        int write_processed = 0;
                        fread(&write_processed, sizeof(int), 1, out_file);
                        write_processed += i;
                        fseek(out_file, sizeof(int), SEEK_SET);
                        fwrite(&write_processed, sizeof(int), 1, out_file);
                        fclose(out_file);
                }
                i = 0;
                while(i < num){
                        free(frames[i++]);
                }
        } else if (((argument_flags & (OU_F_FLAG | WIFI_FLAG)) == (OU_F_FLAG | WIFI_FLAG))){
                // write out wifi frames
                // free bssid if bssid Length is > 0
                WiFi_Frame **frames = (WiFi_Frame**)Object;
                FILE *out_file = open_file(out_filename_ptr, "r+b", args);
                fseek(out_file, 0, SEEK_END);
                // first we need to identify length of a dynamic frame
                // if
                while(i < num){
                        unsigned char obj_size = 8 + 2 + 2 + 1 + 1 + 1 + 2 + 2 +1; // time,src_id,dst_id,bssid_len,essid_len,frm_type,len,sn,fn
                        if (frames[i]->bssid_len > 0){
                                obj_size += frames[i]->bssid_len;
                        }
                        if (frames[i]->essid_len > 0){
                                obj_size += frames[i]->essid_len;
                        }
                        fwrite(&obj_size, sizeof(char), 1, out_file);
                        if( frames[i]->timestamp == 0) break;
                        fwrite(&frames[i]->timestamp, sizeof(char), 8, out_file);
                        fwrite(&frames[i]->src_id, sizeof(char), 2, out_file); // short, avoiding x64 clash
                        fwrite(&frames[i]->dst_id, sizeof(char), 2, out_file); // short, avoiding x64 clash
                        fwrite(&frames[i]->bssid_len, sizeof(char), 1, out_file);
                        //printf("\n%d\n", frames[i]->bssid_len);
                        if (frames[i]->bssid_len > 0){ fwrite(frames[i]->bssid, sizeof(char), frames[i]->bssid_len, out_file); free(frames[i]->bssid); }
                        fwrite(&frames[i]->essid_len, sizeof(char), 1, out_file);
                        if (frames[i]->essid_len > 0){ fwrite(frames[i]->essid, sizeof(char), frames[i]->essid_len, out_file); free(frames[i]->essid); }
                        fwrite(&frames[i]->frame_type, sizeof(char), 1, out_file);
                        fwrite(&frames[i]->frame_sn, sizeof(char), 2, out_file);
                        fwrite(&frames[i]->frame_fn, sizeof(char), 1, out_file);
                        fwrite(&frames[i]->frame_length, sizeof(char), 2, out_file);
                        i++;
                }
                fseek(out_file, sizeof(int), SEEK_SET);
                int write_processed = 0;
                fread(&write_processed, sizeof(int), 1, out_file);
                write_processed += i;
                fseek(out_file, sizeof(int), SEEK_SET);
                fwrite(&write_processed, sizeof(int), 1, out_file);
                fclose(out_file);
                i = 0;
                while(i < num){
                        free(frames[i++]);
                }
        } else if (((argument_flags & (OU_F_FLAG | IP_SHOR_F)) == (OU_F_FLAG | IP_SHOR_F))){
                unsigned char obj_size = sizeof(IP_Frame);
                IP_Frame **frames = (IP_Frame**)Object;
                FILE *out_file = open_file(out_filename_ptr, "r+b", args);
                fseek(out_file, 0, SEEK_END);
                while(i < num){
                        fwrite(&obj_size, sizeof(char), 1, out_file);
                        if(frames[i]->timestamp == 0) break;
                        fwrite(&frames[i]->timestamp, sizeof(char), 8, out_file);
                        fwrite(&frames[i]->src_ip, sizeof(short), 1, out_file); // short, avoiding x64 clash
                        fwrite(&frames[i]->dst_ip, sizeof(short), 1, out_file); // short, avoiding x64 clash
                        fwrite(&frames[i]->protocol, sizeof(short), 1, out_file);
                        fwrite(&frames[i]->packet_size, sizeof(short), 1, out_file);
                        i++;
                }
                fseek(out_file, sizeof(int), SEEK_SET);
                int write_processed = 0;
                fread(&write_processed, sizeof(int), 1, out_file);
                write_processed += i;
                fseek(out_file, sizeof(int), SEEK_SET);
                fwrite(&write_processed, sizeof(int), 1, out_file);
                fclose(out_file);
                i = 0;
                while(i < num){
                        free(frames[i++]);
                }
        } else if (((argument_flags &(OU_F_FLAG | AUDIO_FLA)) == (OU_F_FLAG | AUDIO_FLA)) ||
                         ((argument_flags & (STDOUT_FL | AUDIO_FLA))==(STDOUT_FL | AUDIO_FLA) )){
                unsigned char obj_size = sizeof(Audio_Frame);
                Audio_Frame **frames = (Audio_Frame**)Object;
                FILE *out_file;
                if ((STDOUT_FL & argument_flags) == STDOUT_FL){
                        out_file = stdout;
                } else {
                        out_file = open_file(out_filename_ptr, "r+b", args);
                        fseek(out_file, 0, SEEK_END);
                }
                open_file(out_filename_ptr, "r+b", args);
//                fseek(out_file, 0, SEEK_END);
                while(i < num){
                        //ZigBee_Frame *frame_ptr = (ZigBee_Frame*)*frames;
                        if( frames[i]->timestamp == 0) break;
                        fwrite(&obj_size, sizeof(char), 1, out_file);
                        fwrite(frames[i++], sizeof(Audio_Frame),1, out_file); // -1 because of structure padding
                }
                if ((argument_flags & STDOUT_FL) != STDOUT_FL){
                        fseek(out_file, sizeof(int), SEEK_SET);
                        int write_processed = 0;
                        fread(&write_processed, sizeof(int), 1, out_file);
                        write_processed += i;
                        fseek(out_file, sizeof(int), SEEK_SET);
                        fwrite(&write_processed, sizeof(int), 1, out_file);
                        fclose(out_file);
                }
                i = 0;
                while(i < num){
                        free(frames[i++]);
                }
/*
                while(i < num){
                        fwrite(&obj_size, sizeof(char), 1, out_file);
                        if(frames[i]->timestamp == 0) break;
                        fwrite(&frames[i]->timestamp, sizeof(char), 8, out_file);
                        fwrite(&frames[i]->db, sizeof(double), 1, out_file);
                        i++;
                }
                fseek(out_file, sizeof(int), SEEK_SET);
                int write_processed = 0;
                fread(&write_processed, sizeof(int), 1, out_file);
                write_processed += i;
                fseek(out_file, sizeof(int), SEEK_SET);
                fwrite(&write_processed, sizeof(int), 1, out_file);
                fclose(out_file);
                i = 0;
                while(i < num){
                        free(frames[i++]);
                }
        }
        free(Object);*/
        }
}
/*
void process_zigbee_file_input(FILE *file, unsigned long number_of_lines, char *out_filename_ptr, unsigned short argument_flags, unsigned int Version){
        char *ptr = 0;
        unsigned int line_count = 0;
        unsigned int filtered = 0;
        if ((argument_flags & STDOUT_FL) != STDOUT_FL){
                FILE *out_file = open_file(out_filename_ptr, "w+");
                fwrite(&Version, sizeof(int), 1, out_file);
                fwrite(&line_count, sizeof(int), 1, out_file);
                //fwrite(&zb_frame_len, sizeof(int), 1, out_file);
                fclose(out_file);
        }

        printf("Started processing ZigBee Input File!");
        //
        long file_content_line_slot = number_of_lines/100;
        if (file_content_line_slot == 0) file_content_line_slot = number_of_lines ; // less than a hundred, assign number of lines
        unsigned long next_read = 0;
        unsigned long slot = file_content_line_slot;
        unsigned long overall_filtered = 0;
        while(next_read < number_of_lines){
                ptr = extract_number_of_lines(file, next_read, next_read+slot);
                ZigBee_Frame **zb_arr_ptr = process_zigbee_lines(ptr, slot, &filtered);
                write_out_frames_new((void*)zb_arr_ptr, filtered, '.', line_count);
                next_read += file_content_line_slot;
                free(ptr);
                overall_filtered += filtered;
                if ((next_read+slot) > number_of_lines){ slot = number_of_lines-next_read; }
                if ((filtered != 0xFFFFFFFF) && (live_descriptor_write)){write_descriptor(&Enum_Start, "protocols.csv"); live_descriptor_write = 0;}
        }
}

void process_wifi_file_input(FILE *file, unsigned long number_of_lines){
        char *ptr = 0;
        unsigned int line_count = 0;
        unsigned int filtered = 0;
        FILE *out_file = open_file(out_filename_ptr, "w+");
        fwrite(&Version, sizeof(int), 1, out_file);
        fwrite(&line_count, sizeof(int), 1, out_file);
        //fwrite(&zb_frame_len, sizeof(int), 1, out_file);
        fclose(out_file);

        printf("Started processing WiFi Input File!\n");
        //
        unsigned long file_content_line_slot = number_of_lines/100;
        if (file_content_line_slot == 0) file_content_line_slot = number_of_lines ; // less than a hundred, assign number of lines
        unsigned long next_read = 0;
        unsigned long slot = file_content_line_slot;
        unsigned long overall_filtered = 0;
        while(next_read < number_of_lines){
                ptr = extract_number_of_lines(file, next_read, next_read+slot);
                //ZigBee_Frame **zb_arr_ptr = process_zigbee_lines(ptr, slot, &filtered);
                WiFi_Frame **wifi_arr_ptr = process_wifi_lines(ptr, slot, &filtered);
                printf("Processed this number of lines: %u (%lu/%lu)\n", filtered, next_read, next_read+slot);
                write_out_frames_new((void*)wifi_arr_ptr, filtered, '.', line_count);
                next_read += file_content_line_slot;
                free(ptr);
                overall_filtered += filtered;
                if ((next_read+slot) > number_of_lines){ slot = number_of_lines-next_read; }
                if ((filtered != 0xFFFFFFFF) && (live_descriptor_write)){write_descriptor(&Enum_Start, "protocols.csv"); write_descriptor(&WiFi_Address, "addresses.csv"); live_descriptor_write = 0;}
        }
}
*/

