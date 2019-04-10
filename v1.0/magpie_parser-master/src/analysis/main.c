#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <limits.h>
//#include "structures.h"
#include "generic.h"
#include "math_func.h"
#include "windowing.h"
#include "processor.h"

#define INTERVAL 1000000 // second

unsigned char operation = 0;

unsigned char filter_address(void *object, unsigned char type, unsigned short exclude_addr){
	switch(type){
		case 1:
			//ZigBee_Frame *obj = (ZigBee_Frame*)object;
			if (((ZigBee_Frame*)object)->src_id == exclude_addr) return 0;
			if (((ZigBee_Frame*)object)->dst_id == exclude_addr) return 0;
			// parse ZigBee
			break;
		case 2:
			//IP_Frame *obj = (IP_Frame*)object;
			if (((IP_Frame*)object)->src_ip == exclude_addr) return 0;
			if (((IP_Frame*)object)->dst_ip == exclude_addr) return 0;
			// parse IP_Short
			break;
		case 3:
			//WiFi_Frame *obj = (WiFi_Frame*)object;
			if (((WiFi_Frame*)object)->src_id == exclude_addr) return 0;
			if (((WiFi_Frame*)object)->dst_id == exclude_addr) return 0;
			// parse wifi
			break;
		case 4:
			// parse audio
			break;
	}
	return 1;
}

int main(int argc, char **argv){ int mode = 1;
	FILE *file = fopen(argv[1], "rb");
	if (argc > 2){
		mode = atoi(argv[2]);
		operation = mode;
	}
	if (argc > 3){

	}
	int version = 0;
	int pkt_number = 0;
	fseek(file, 0, SEEK_SET);
	fread(&version, sizeof(int), 1, file);
	fread(&pkt_number,sizeof(int), 1,file);
//	printf("Version: %d Packets: %d\n", version, pkt_number);
	SLOT *slot;
	GLOBAL_KNOWLEDGE *glob = global_knowledge_init();
	uint64_t global_start_time = 0;
	unsigned int multiplier =0;
	slot = slot_init();
	while(pkt_number--){
		int t_len = 0;
		if (mode == 0){	// Wifi
			WiFi_Frame t_wf_frm;
			fread(&t_len, sizeof(char), 1, file);
			fread(&t_wf_frm, t_len, 1, file);
			if (slot->n == 0){
				if (global_start_time == 0) global_start_time = (t_wf_frm.timestamp/1000000)*1000000;
				update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
				frame_add(slot, &t_wf_frm, t_len, 1);
				continue;
			} else {
				if ((t_wf_frm.timestamp >= slot->slot_start_time)&&(t_wf_frm.timestamp <= slot->slot_stop_time)){
					frame_add(slot, &t_wf_frm, t_len, 1);
				} else {
					get_unique_number_wi(slot, glob);
					get_unique_number_frames(slot, 0, glob);
					// process
					pro_wifi(slot, glob);
					free_slot(slot);
					update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
					if ((t_wf_frm.timestamp > slot->slot_start_time) && (t_wf_frm.timestamp < slot->slot_stop_time)){
						 while(t_wf_frm.timestamp < slot->slot_start_time){
							// if 0's neede, can be padded here
							update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
						}
					}
					frame_add(slot, &t_wf_frm, t_len, 1);
					free(glob->Global_Sources->array);
					free(glob->Global_Destinations->array);
					free(glob->Global_Frames->array);
					continue;
				}
				continue;
			}
			printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n",
				t_wf_frm.timestamp, t_wf_frm.src_id, t_wf_frm.dst_id,
				t_wf_frm.bssid_len, t_wf_frm.essid_len, t_wf_frm.frame_type);
		} else if (mode == 1){	// ZigBee
			ZigBee_Frame t_zb_frm;
        	        fread(&t_len, sizeof(char), 1, file);
        	        fread(&t_zb_frm, t_len, 1, file);
			if (slot->n == 0){	// initial start, the rest will be done later in else
				if (global_start_time == 0) global_start_time = (t_zb_frm.timestamp/1000000)*1000000;
				update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
//				printf("*%"PRIu64"-%"PRIu64" Time: %"PRIu64"\n", slot->slot_start_time, slot->slot_stop_time, t_zb_frm.timestamp);
//				if (t_zb_frm.timestamp > slot->slot_stop_time){
//					printf("\nMoney\n");
//					process_zigbee(slot);
					// processing needs to be done here
					// free slot
					// identify whether current frame fits the slot
					// if not identify how many INTERVALS doesnt fit
						// output 0 until current frame fits and update slot
//				}
				//slot->frame_array = (FRAME*)calloc(1, sizeof(FRAME));
				//slot->frame_array->frame_len = t_len;
				//slot->frame_array->frame_ptr = (char*)calloc(1, t_len);
				//memcpy(slot->frame_array->frame_ptr, &t_zb_frm, t_len);
				//slot->n++;
				frame_add(slot, &t_zb_frm, t_len, 1);
				continue;
			} else {
				if ((t_zb_frm.timestamp >= slot->slot_start_time)&&(t_zb_frm.timestamp <= slot->slot_stop_time)){
				//	FRAME *frame = (FRAME*)calloc(1, sizeof(FRAME));
			//		FRAME *t_frame = slot->frame_array;
			//		while(t_frame->next != 0) t_frame = t_frame->next;
					// I have the last frame
			//		t_frame->next = frame;
			//		frame->frame_len = t_len;
			//		frame->frame_ptr = (char*)calloc(1, t_len);
			//		memcpy(frame->frame_ptr, &t_zb_frm, t_len);
			//		slot->n++;
					
//					printf(" %"PRIu64"-%"PRIu64" Time: %"PRIu64"\n", slot->slot_start_time, slot->slot_stop_time, t_zb_frm.timestamp);
					frame_add(slot, &t_zb_frm, t_len, 1);
				} else { // the timestamp is higher
					// do stat analysis
//					printf("%"PRIu64"-%"PRIu64" Time: %"PRIu64"\n", slot->slot_start_time, slot->slot_stop_time, t_zb_frm.timestamp);
					
					get_unique_number(slot, glob);
					get_unique_number_frames(slot, 1, glob);
					pro_zbee(slot, glob);
					//printf("\ngot a slot, pkt: %d\n", slot->n);
					if (t_zb_frm.timestamp > slot->slot_stop_time){
						pro_zbee(slot, glob);
					}
					free_slot(slot);
//					printf("\nMallocs: %d Frees: %d\n", mallocs, frees);
//					mallocs = 0; frees =0;
					update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
					if ((t_zb_frm.timestamp > slot->slot_start_time) && (t_zb_frm.timestamp < slot->slot_stop_time)){
						while(t_zb_frm.timestamp < slot->slot_start_time){
							// zeros can be padded here
							//for now catchup
							update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
						}
					}
					frame_add(slot, &t_zb_frm, t_len, 1); // add current handled frame
					free(glob->Global_Sources->array);
					free(glob->Global_Destinations->array);
					free(glob->Global_Frames->array);
//					free_slot(slot);
	//				return 0;
					// free slot

					// re-initialize
				}
				continue;
			}
	// if (t_zb_frm.frame_type == 0) continue;
	// printf("%04x\n",t_zb_frm.src_id);
	          //      printf("%"PRIu64",%04x,%04x,%d,%d,%d\n",
        	//                t_zb_frm.timestamp, t_zb_frm.src_id, t_zb_frm.dst_id,
               // 	        t_zb_frm.frame_type, t_zb_frm.packet_size, t_zb_frm.flags);
	// if (t_zb_frm.flags != 1) inc_relation(Start, t_zb_frm.src_id, t_zb_frm.dst_id);
		} else if (mode == 2){	// IP Short
			IP_Frame t_ips_frm;
        	        fread(&t_len, sizeof(char), 1, file);
	                fread(&t_ips_frm, sizeof(IP_Frame), 1, file);
			if (slot->n == 0){      // initial start, the rest will be done later in else
                                if (global_start_time == 0) global_start_time = (t_ips_frm.timestamp/1000000)*1000000;
                                update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
//                              printf("*%"PRIu64"-%"PRIu64" Time: %"PRIu64"\n", slot->slot_start_time, slot->slot_stop_time, t_zb_frm.timestamp);
//                              if (t_zb_frm.timestamp > slot->slot_stop_time){
//                                      printf("\nMoney\n");
//                                      process_zigbee(slot);
                                        // processing needs to be done here
                                        // free slot
                                        // identify whether current frame fits the slot
                                        // if not identify how many INTERVALS doesnt fit
                                                // output 0 until current frame fits and update slot
//                              }
                                //slot->frame_array = (FRAME*)calloc(1, sizeof(FRAME));
                                //slot->frame_array->frame_len = t_len;
                                //slot->frame_array->frame_ptr = (char*)calloc(1, t_len);
                                //memcpy(slot->frame_array->frame_ptr, &t_zb_frm, t_len);
                                //slot->n++;
                                frame_add(slot, &t_ips_frm, t_len, 1);
                                continue;
                        } else {
                                if ((t_ips_frm.timestamp >= slot->slot_start_time)&&(t_ips_frm.timestamp <= slot->slot_stop_time)){
					frame_add(slot, &t_ips_frm, t_len, 1);
                                } else { // the timestamp is higher
                                        // do stat analysis
//                                      printf("%"PRIu64"-%"PRIu64" Time: %"PRIu64"\n", slot->slot_start_time, slot->slot_stop_time, t_zb_frm.timestamp);

                                        get_unique_number(slot, glob);
					get_unique_number_frames(slot, 2, glob);
                                        pro_ip_short(slot, glob);
                                        //printf("\ngot a slot, pkt: %d\n", slot->n);
                                        if (t_ips_frm.timestamp > slot->slot_stop_time){
                                                pro_ip_short(slot, glob);
                                        }
                                        free_slot(slot);
//                                      printf("\nMallocs: %d Frees: %d\n", mallocs, frees);
//                                      mallocs = 0; frees =0;
                                        update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
                                        if ((t_ips_frm.timestamp > slot->slot_start_time) && (t_ips_frm.timestamp < slot->slot_stop_time)){
                                                while(t_ips_frm.timestamp < slot->slot_start_time){
                                                        // zeros can be padded here
                                                        //for now catchup
                                                        update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
                                                }
                                        }
                                        frame_add(slot, &t_ips_frm, t_len, 1); // add current handled frame
                                        free(glob->Global_Sources->array);
                                        free(glob->Global_Destinations->array);
					free(glob->Global_Frames->array);
//                                      free_slot(slot);
        //                              return 0;
                                        // free slot

                                        // re-initialize
                                }
                                continue;
                        }
	// if (t_zb_frm.frame_type == 0) continue;
	// printf("%04x\n",t_zb_frm.src_id);
//	                printf("%"PRIu64",%04x,%04x,%d,%d\n",
//	                        t_ips_frm.timestamp, t_ips_frm.src_id, t_ips_frm.dst_id,
//	                        t_ips_frm.protocol, t_ips_frm.packet_size);//, t_zb_frm.flags);
	// if (t_zb_frm.flags != 1) inc_relation(Start, t_zb_frm.src_id, t_zb_frm.dst_id);
		} else if (mode == 3){	// Sound else {
			Audio_Frame t_snd_frm;
                        fread(&t_len, sizeof(char), 1, file);
                        fread(&t_snd_frm, t_len, 1, file);
			if (slot->n == 0){      // initial start, the rest will be done later in else
                                if (global_start_time == 0) global_start_time = (t_snd_frm.timestamp/1000000)*1000000;
                                update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
				frame_add(slot, &t_snd_frm, t_len, 1);
			} else {
				if ((t_snd_frm.timestamp >= slot->slot_start_time)&&(t_snd_frm.timestamp <= slot->slot_stop_time)){
					frame_add(slot, &t_snd_frm, t_len, 1);
					continue;
				} else {
					pro_audio(slot);
					free_slot(slot);
					update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
                                        if ((t_snd_frm.timestamp > slot->slot_start_time) && (t_snd_frm.timestamp < slot->slot_stop_time)){
                                                while(t_snd_frm.timestamp < slot->slot_start_time){
                                                        // zeros can be padded here
                                                        //for now catchup
                                                        update_stop_start_times(slot, global_start_time, &multiplier, INTERVAL);
                                                }
                                        }
					frame_add(slot, &t_snd_frm, t_len, 1);
					continue;
				}
			}
			//printf("Mode not supported\n");
		}
	}
// Relation *ptr = Start; // while(ptr->next != 0){ // printf("Relation: %04x -> %04x. Hits: %lu\n", ptr->src_id, ptr->dst_id, ptr->counter); // ptr = ptr->next; //	} //	printf("Debug break");
	return 0;
}
