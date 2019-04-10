// Windowing function implementation
#include "windowing.h"

SLOT* slot_init(){
        SLOT *ptr = (SLOT*)calloc(1, sizeof(SLOT));
        return ptr;
}
void free_slot(SLOT *slot){
		FRAME *frame_ptr;
        FRAME *min_one;
        frame_ptr = slot->frame_array;
        min_one = slot->frame_array;
        while(slot->n--){
                while(frame_ptr->next != 0){min_one = frame_ptr; frame_ptr = frame_ptr->next;}
                free(frame_ptr->frame_ptr);
                free(frame_ptr);
                //frees++;
                min_one->next = 0;
                frame_ptr = slot->frame_array;
                //printf(".");
                fflush(stdout);
        }
        //printf("\n");
        slot->n = 0;
        slot->frame_array = 0;
}
void frame_add(SLOT *slot, void *object, unsigned char object_size, unsigned char type){
        // type 0 = wifi, 1 = zigbee, 2 = ip, 3 = sound
        FRAME *_frame = (FRAME*)0x0000;
        if (slot->frame_array != 0){
			_frame = slot->frame_array;
            while(_frame->next != 0) _frame = _frame->next;
            _frame->next = (FRAME *)calloc(1, sizeof(FRAME));
            _frame->next->frame_len = object_size;// zigbee frame length
            _frame->next->frame_ptr = (char*)calloc(1, _frame->next->frame_len);
            memcpy(_frame->next->frame_ptr, object, _frame->next->frame_len);
            slot->n++;
        } else {
                                _frame = (FRAME *)calloc(1, sizeof(FRAME));
                                _frame->frame_len = object_size;// zigbee frame length
                                _frame->frame_ptr = (char*)calloc(1, _frame->frame_len);
                                memcpy(_frame->frame_ptr, object, _frame->frame_len);
                                slot->frame_array = _frame;
                                slot->n++;
//                              printf("\n%d\n", ((ZigBee_Frame*)object)->packet_size);
       }
}
/*void frame_add(SLOT *slot, void *object, unsigned char object_size, unsigned char type){
        // type 0 = wifi, 1 = zigbee, 2 = ip, 3 = sound
        FRAME *_frame = (FRAME*)0x0000;
        switch(type){
                case 0:
                        //wifi

                        break;
                case 1:
                        //zigbee
                        if (slot->frame_array != 0){
                                _frame = slot->frame_array;
                                while(_frame->next != 0) _frame = _frame->next;
                                _frame->next = (FRAME *)calloc(1, sizeof(FRAME));
                                _frame->next->frame_len = object_size;// zigbee frame length
                                _frame->next->frame_ptr = (char*)calloc(1, _frame->next->frame_len);
                                memcpy(_frame->next->frame_ptr, object, _frame->next->frame_len);
                                slot->n++;
                        } else {
                                _frame = (FRAME *)calloc(1, sizeof(FRAME));
                                _frame->frame_len = object_size;// zigbee frame length
                                _frame->frame_ptr = (char*)calloc(1, _frame->frame_len);
                                memcpy(_frame->frame_ptr, object, _frame->frame_len);
                                slot->frame_array = _frame;
                                slot->n++;
//                              printf("\n%d\n", ((ZigBee_Frame*)object)->packet_size);
                        }
                        //mallocs++;
                        break;
                case 2:
                        //ip
                        break;
                case 3:
                        //sound
                        break;
                case 4:
						// spectrum
						
							
						break;
                default:
                        break;
        }
}*/
void update_stop_start_times(SLOT *slot, uint64_t global_start_time, unsigned int *multiplier, unsigned int interval){
        slot->slot_start_time = global_start_time + (interval*(*multiplier)++);
        slot->slot_stop_time = slot->slot_start_time+interval;
}

unsigned short get_unique_number_frames(SLOT *slot, unsigned char frame_type, GLOBAL_KNOWLEDGE *glob){
   // enumerate id's
        unsigned int i = 0;
        unsigned short unique_ft = 0;
        FRAME *_frame = slot->frame_array;
        unsigned short *ft_array = 0;
        while(i < slot->n){
                if (unique_ft == 0){
                        switch(frame_type){
                                case 0:         // Wifi
                                if (((WiFi_Frame*)_frame->frame_ptr) != 0){
                                        ft_array = (unsigned short*)calloc(1, sizeof(short));
                                        *ft_array = ((WiFi_Frame*)_frame->frame_ptr)->frame_type;
                                        unique_ft++;
                                }
                                        break;
                                case 1:         // ZigBee
                                if (((ZigBee_Frame*)_frame->frame_ptr) != 0){
                                        ft_array = (unsigned short*)calloc(1, sizeof(short));
                                        *ft_array = ((ZigBee_Frame*)_frame->frame_ptr)->frame_type;
                                        unique_ft++;
                                }
                                        break;
                                case 2:         // IP
                                if (((IP_Frame*)_frame->frame_ptr) != 0){
                                        ft_array = (unsigned short*)calloc(1, sizeof(short));
                                        *ft_array = ((IP_Frame*)_frame->frame_ptr)->protocol;
                                        unique_ft++;
                                }
                                        break;
                                case 3:         // Sound
                                        break;
                                default:
                                        break;
                        }
                } else {

                        unsigned short _t = 0;
                        unsigned short *_ptr = ft_array;
                        unsigned char found = 0;
                        unsigned short storage = 0;
                        while(_t < unique_ft){
                                switch(frame_type){
                                        case 0:
                                                storage = ((WiFi_Frame*)_frame->frame_ptr)->frame_type;
                                                break;
                                        case 1:
                                                storage = ((ZigBee_Frame*)_frame->frame_ptr)->frame_type;
                                                break;
                                        case 2:
                                                storage = ((IP_Frame*)_frame->frame_ptr)->protocol;
                                                break;
                                        case 3:
                                                break;
                                        default:
                                                break;
                                }
                                if (_ptr[_t] == storage){
                                        found = 1;
                                        break;
                                }

                                _t++;
                        }
                        if (found != 1){ // if src was not found, add
                                unique_ft++;   // this will be added
                                unsigned short *free_me = ft_array;
                                unsigned short *upd_ft_array = (unsigned short*)calloc(unique_ft, sizeof(short)); // allocate known unique +1
                                memcpy(upd_ft_array, free_me, sizeof(short)*(unique_ft-1));
                                free(free_me);
                                free_me = upd_ft_array+unique_ft-1;
                                *free_me = storage;
                                ft_array = upd_ft_array;
                        }
                }
                i++;
                _frame = _frame->next;

        }

        //if (src_array != 0) free(src_array);
        glob->Global_Frames->array = ft_array;
        glob->Global_Frames->n = unique_ft;
        return unique_ft;

}

unsigned short get_unique_number_wi(SLOT *slot, GLOBAL_KNOWLEDGE *glob){   // enumerate id's
        unsigned int i = 0;
        unsigned short unique_src = 0;
        unsigned short unique_dst = 0;
        FRAME *_frame = slot->frame_array;
        unsigned short *src_array = 0;
        unsigned short *dst_array = 0;
        while(i < slot->n){
                WiFi_Frame *frm = (WiFi_Frame*)_frame->frame_ptr;
                // do for sources
                if (unique_src == 0){
                        // save the first source
                        if (frm != 0){
                                src_array = (unsigned short*)calloc(1, sizeof(short));
                                *src_array = frm->src_id;
                                unique_src++;
                        } else {
                                printf("ERROR: Accessed get_unique_number [frm == 0] (src)\n");
                        }
                } else {
                        unsigned short _t = 0;
                        unsigned short *_ptr = src_array;
                        unsigned char found = 0;
                        while(_t < unique_src){
                                if (_ptr[_t] == frm->src_id){
                                        found = 1;
                                        break;
                                }
                                _t++;
                        }
                        if (found != 1){ // if src was not found, add
                                unique_src++;   // this will be added
                                unsigned short *free_me = src_array;
                                unsigned short *upd_src_array = (unsigned short*)calloc(unique_src, sizeof(short)); // allocate known unique +1
                                memcpy(upd_src_array, free_me, sizeof(short)*(unique_src-1));
                                free(free_me);
                                free_me = upd_src_array+unique_src-1;
                                *free_me = frm->src_id;
                                src_array = upd_src_array;
                        }
                }
                // do for destinations
                if (unique_dst == 0){
                        if (frm != 0){
                                dst_array = (unsigned short*)calloc(1, sizeof(short));
                                *dst_array= frm->dst_id;
                                unique_dst++;
                        } else {
                                printf("ERROR: Accessed get_unique_number [frm == 0] (dst)\n");
                        }
                } else {
                        unsigned short _t = 0;
                        unsigned short *_ptr = dst_array;
                        unsigned char found = 0;
                        while(_t < unique_dst){
                                if (_ptr[_t] == frm->dst_id){
                                        found = 1;
                                        break;
                                }
                                _t++;
                        }
                        if (found != 1){
                                unique_dst++;
                                unsigned short *free_me = dst_array;
                                unsigned short *upd_dst_array = (unsigned short*)calloc(unique_dst, sizeof(short));
                                memcpy(upd_dst_array, free_me, sizeof(short)*(unique_dst-1));
                                free(free_me);
                                free_me = upd_dst_array+unique_dst-1;
                                *free_me = frm->dst_id;
                                dst_array = upd_dst_array;
                        }
                }
                i++;
                _frame = _frame->next;
        }
        //if (src_array != 0) free(src_array);
        glob->Global_Sources->array = src_array;
        glob->Global_Destinations->array = dst_array;
        glob->Global_Destinations->n = unique_dst;
        glob->Global_Sources->n = unique_src;
        return unique_src;
}

unsigned short get_unique_number(SLOT *slot, GLOBAL_KNOWLEDGE *glob){   // enumerate id's
        unsigned int i = 0;
        unsigned short unique_src = 0;
        unsigned short unique_dst = 0;
        FRAME *_frame = slot->frame_array;
        unsigned short *src_array = 0;
        unsigned short *dst_array = 0;
        while(i < slot->n){
                ZigBee_Frame *frm = (ZigBee_Frame*)_frame->frame_ptr;
                // do for sources
                if (unique_src == 0){
                        // save the first source
                        if (frm != 0){
                                src_array = (unsigned short*)calloc(1, sizeof(short));
                                *src_array = frm->src_id;
                                unique_src++;
                        } else {
                                printf("ERROR: Accessed get_unique_number [frm == 0] (src)\n");
                        }
                } else {
                        unsigned short _t = 0;
                        unsigned short *_ptr = src_array;
                        unsigned char found = 0;
                        while(_t < unique_src){
                                if (_ptr[_t] == frm->src_id){
                                        found = 1;
                                        break;
                                }
                                _t++;
                        }
                        if (found != 1){ // if src was not found, add
                                unique_src++;   // this will be added
                                unsigned short *free_me = src_array;
                                unsigned short *upd_src_array = (unsigned short*)calloc(unique_src, sizeof(short)); // allocate known unique +1
                                memcpy(upd_src_array, free_me, sizeof(short)*(unique_src-1));
                                free(free_me);
                                free_me = upd_src_array+unique_src-1;
                                *free_me = frm->src_id;
                                src_array = upd_src_array;
                        }
                }
                // do for destinations
                if (unique_dst == 0){
                        if (frm != 0){
                                dst_array = (unsigned short*)calloc(1, sizeof(short));
                                *dst_array= frm->dst_id;
                                unique_dst++;
                        } else {
                                printf("ERROR: Accessed get_unique_number [frm == 0] (dst)\n");
                        }
                } else {
                        unsigned short _t = 0;
                        unsigned short *_ptr = dst_array;
                        unsigned char found = 0;
                        while(_t < unique_dst){
                                if (_ptr[_t] == frm->dst_id){
                                        found = 1;
                                        break;
                                }
                                _t++;
                        }
                        if (found != 1){
                                unique_dst++;
                                unsigned short *free_me = dst_array;
                                unsigned short *upd_dst_array = (unsigned short*)calloc(unique_dst, sizeof(short));
                                memcpy(upd_dst_array, free_me, sizeof(short)*(unique_dst-1));
                                free(free_me);
                                free_me = upd_dst_array+unique_dst-1;
                                *free_me = frm->dst_id;
                                dst_array = upd_dst_array;
                        }
                }
                i++;
                _frame = _frame->next;
        }
        //if (src_array != 0) free(src_array);
        glob->Global_Sources->array = src_array;
        glob->Global_Destinations->array = dst_array;
        glob->Global_Destinations->n = unique_dst;
        glob->Global_Sources->n = unique_src;
        return unique_src;
}


