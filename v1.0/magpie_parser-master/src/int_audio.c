#include "int_audio.h"
#include "formatting.h"
#include "fileutils.h"

void pro_audio_int(char *line, audio_struct_internal *audio_frm){
	// 1529699190,3.86
	// [1533593349.383641, 3.1]
	char *ptr = &line[0];
	int i = 0;
	while (ptr[i] != ',') i++;
	ptr[i] = '\0';
	audio_frm->timestamp = (uint64_t)(atof(ptr)*1000000); i++;
	audio_frm->value = atof(&ptr[i]);
}
Audio_Frame **process_audio_frame_lines(char *ptr, unsigned long lines, unsigned int *filtered){
        char *line = ptr;
        char **t_upd = (char**)calloc(1, sizeof(t_upd));
        Audio_Frame **audio_arr = (Audio_Frame**)calloc(lines, sizeof(audio_arr));
        int audio_arr_i = 0;
        unsigned int l_count = 0;
        while(l_count < lines){
                line = extract_line(line, t_upd);
                // the format is csv 0.16003552981111932,2017-11-24,15:14:03
                // value,date,time
                audio_arr[audio_arr_i] = (Audio_Frame*)calloc(1, sizeof(Audio_Frame));
                char *start = line;
                char *t_ptr = 0;
                char *tt_ptr= 0;
                char *test=0;
                int i = 0;
                int z = 0;
                while(*(start++) != ',')i++;
                start = (char*)calloc((i+1), sizeof(char));
                memcpy(start, line, i); // -1 removes the comma
//              audio_arr[audio_arr_i]->db = atof(start);
                double abc_test = atof(start);
                abc_test *= 1000000.0;
                audio_arr[audio_arr_i]->timestamp = (atof(start)*(double)1000000.0);
                free(start);
                start = line+i+1; i = 0; // +1 because currently pointing at ,
                while(*(start++) != ',')i++;
                t_ptr = (char*)calloc(i+1, sizeof(char));
                memcpy(t_ptr, (start-(1+i)), i);
                audio_arr[audio_arr_i]->db = atof(t_ptr);
                //start is currently at the last bit
                tt_ptr = start;
                i=0;
                while(*(tt_ptr++) != ','){i++; if(*tt_ptr == '\r' || *tt_ptr == '\n')break; } // got the last bit
                test=(char*)calloc(i+1, sizeof(char));
                memcpy(test, start, i);
                // now we have to merge them together
                // t_ptr = date, z ?= date length
                // test = time, i = time length
                tt_ptr = t_ptr;
                while(*tt_ptr++ != '\0') z++;
                start = (char*)calloc(z+i+1+12, sizeof(char)); // Why 3? Why not I would say ))
//              sprintf(start, "1 %s %s.000000  a",t_ptr, test);
//              audio_arr[audio_arr_i]->timestamp = convert_date_to_epoch(start);
//              memcpy(start, t_ptr, z);
//              memcpy(start+z, " ", 1); // add space
//              memcpy(start+z+1, test, i);
                free(start);
                free(test);
                free(t_ptr);
                audio_arr_i++;
                l_count++;
        }
        *filtered = --audio_arr_i;
        free(t_upd);
        return audio_arr;
}

void process_audio_input_live(unsigned char live_mode, char *line_buffer, char *args, unsigned short argument_flags, char *out_filename_ptr){
        unsigned int line_count = 0;
        unsigned int filtered = 0;
        Audio_Frame **audio_arr_ptr = process_audio_frame_lines(line_buffer, 1, &filtered);
        if (filtered != 0xFFFFFFFF) write_out_frames_new((void*)audio_arr_ptr, 1, '\0', line_count, out_filename_ptr, argument_flags, args);
}

