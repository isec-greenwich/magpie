// Processor function implementation

#include "processor.h"

#define ZBEE_TYPE 1
#define WIFI_TYPE 2
#define IPSH_TYPE 3
#define AUDI_TYPE 4

void pro_audio(SLOT *slot){
        unsigned int i = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        FRAME *_frame = slot->frame_array;
        double *val_array = (double*)calloc(slot->n, sizeof(double));
        while(i < slot->n){
                Audio_Frame *frm = (Audio_Frame*)_frame->frame_ptr;
                val_array[i] = frm->db;
                avg += frm->db;
                _frame = _frame->next;
                i++;
        }
        if (i != 0){
                avg = _math_average_dbl(val_array, i);
                stdev = _math_stdev(_math_variance_dbl(val_array, avg, i));
                avg_dev = _math_avg_dev_dbl(val_array, i);
                if (isnan(avg_dev)) avg_dev=0.0;
                if (isnan(stdev)) stdev = 0.0;
                double min = 0.0;
                double max = 0.0;
                int freq = _math_count_threshold(val_array, i, 0.9);
                _math_minmax_dbl(val_array, i, &min, &max);
                printf("%" PRIu64 ",%f,%f,%f,%f,%d\n", slot->slot_stop_time, avg, min, max, stdev, freq);
                avg = 0.0;
                stdev = 0.0;
                avg_dev = 0.0;
                free(val_array);
        }
}

void pro_zbee(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned int proceed = 0;
//      unsigned char flags = 0;
        FRAME *_frame = slot->frame_array;
        while (f < glob->Global_Frames->n){
        while (k < glob->Global_Sources->n){   // for every source
                while(j < glob->Global_Destinations->n){ // for every destination
                        _frame = slot->frame_array;
                        unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
			uint64_t *dif_array = (uint64_t*)calloc(slot->n, sizeof(uint64_t)); // latenc
                        while(i < slot->n){ // for each packet
                                ZigBee_Frame *frm = (ZigBee_Frame*)_frame->frame_ptr;
                                if (frm->frame_type == (*(glob->Global_Frames->array+f))) proceed = 1;
                                if (proceed && (*(glob->Global_Sources->array+k) == frm->src_id) && (*(glob->Global_Destinations->array+j) == frm->dst_id)){
					dif_array[freq] = frm->timestamp;	// ~ latec
                                        val_array[freq] = frm->packet_size;
                                        freq++;
                                        avg += frm->packet_size;
//                                        if (frm->flags == 1) printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n", slot->slot_stop_time, (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+j)),(*(glob->Global_Frames->array+f)), frm->packet_size, frm->flags);
//                                        if (frm->flags == 0) printf("%" PRIu64 ",%04x,%04x,%d,%d,%d\n", slot->slot_stop_time, (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+j)),(*(glob->Global_Frames->array+f)), frm->packet_size, frm->flags);
                                }
                                // assuming there is only 1 packet in the slot
                        //      flags = frm->flags;
                                _frame = _frame->next;
                                proceed = 0;
                                i++;
                        }
			unsigned int *latency_array = _math_generate_latency_array(dif_array, freq);//(unsigned int*)calloc(freq, sizeof(int)); // lat
			double avg_latency = _math_average(latency_array, freq-1); // lat
			double std_latency = _math_stdev(_math_variance(latency_array, avg, freq-1)); // lat
			unsigned int min_latency, max_latency; // late
			_math_minmax(latency_array, freq-1, &min_latency, &max_latency); // lat
                        avg = _math_average(val_array, freq);
                        stdev = _math_stdev(_math_variance(val_array, avg, freq));
                        avg_dev = _math_avg_dev(val_array, freq);
                        if (isnan(avg_dev)) avg_dev=0.0;
                        if (isnan(stdev)) stdev = 0.0;
                        unsigned int min = 0;
                        unsigned int max = 0;
                        _math_minmax(val_array, freq, &min, &max);
                        freq = 0;
                        avg = 0.0;
                        i = 0;
			free(dif_array);
			free(latency_array);
                        free(val_array);
                        j++;
                }
                j = 0;
                k++;
        }
        k = 0; j = 0;
        f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}

int cmp(const void *a, const void *b){
	return ( *(int*)a - *(int*)b );
}

void cpu_wifi_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator){
	unsigned int i = 0;
	unsigned int k = 0;
	unsigned int j = 0;
	unsigned int l = 0;
	unsigned int x = 0;

	unsigned char flag = 0;
	unsigned int freq = 0;
	double avg = 0.0;
	double std = 0.0;
	unsigned int min = 0;
	unsigned int max = 0;
	double avg_dev = 0.0;
	double avg_rssi= 0.0;
	double avg_lat = 0.0;
	double std_lat = 0.0;
	double std_rssi= 0.0;
	unsigned int  min_lat = 0.0;
	unsigned int  max_lat = 0.0;

	while(i < glob->Global_SubTypes->n){	// for each subtype
		k = 0;
		while(k < glob->Global_Types->n){ // for each type
			j = 0;
			while(j < glob->Global_Sources->n){ // for every source
				l = 0;
				while(l < glob->Global_Destinations->n){ // for every destination
					FRAME *_frame = slot->frame_array;
					unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
					uint64_t *dif_array = (uint64_t*)calloc(slot->n, sizeof(uint64_t));
					int *rssi_array= (int*)calloc(slot->n, sizeof(int));
					x = 0;
					while(x < slot->n){ // for every packet in slot
						wifi_struct_internal *frm = (wifi_struct_internal*)_frame->frame_ptr;
						if (frm->subtype == (glob->Global_SubTypes->array[i])) flag = 1;
						if (flag == 1){
							if ( (glob->Global_Types->array[k] == frm->type) && (glob->Global_Sources->array[j] == frm->src_mac) && (glob->Global_Destinations->array[l] == frm->dst_mac) ){
								dif_array[freq] = frm->timestamp;
								val_array[freq] = frm->len;
								rssi_array[freq]= frm->rssi;
								freq++;
							}
						}

						_frame = _frame->next;
						flag = 0;
						x++;
					}
					unsigned int *latency_array = 0;
					if (freq > 1){
						if (freq > 1) latency_array = _math_generate_latency_array(dif_array, freq);
						if (latency_array != 0){
							qsort(latency_array, freq-1, sizeof(int), cmp);
							avg_lat = _math_average(latency_array, freq-1);
							std_lat = _math_stdev(_math_variance(latency_array, avg, freq-1));
//							_math_minmax(latency_array, freq-1, &min_lat, &max_lat);
						}
						
						avg = _math_average(val_array, freq);
						avg_rssi = _math_average_i(rssi_array, freq);
						std_rssi = _math_stdev(_math_variance_i(rssi_array, avg_rssi, freq));
						std = _math_stdev(_math_variance(val_array, avg, freq));
						avg_dev = _math_avg_dev(val_array, freq);

						if (isnan(avg_dev)) avg_dev= 0.0;
						if (isnan(std)) std = 0.0;
						if (isnan(std_lat)) std_lat = 0.0;
						if (isnan(std_rssi)) std_rssi= 0.0;
						if (isnan(avg_rssi)) avg_rssi= 0.0;
						if (isinf(avg_rssi)) avg_rssi= 0.0;
						if (isinf(std_rssi)) std_rssi= 0.0;
						if (isinf(avg_dev)) avg_dev= 0.0;
						if (isinf(std)) std = 0.0;
						if (isinf(std_lat)) std_lat = 0.0;
						
//						_math_minmax(val_array, freq, &min, &max);
						// output
						char *src_mac = enum_find_frame_name(glob->Global_Sources->array[j], Enumerator);
						char *dst_mac = enum_find_frame_name(glob->Global_Destinations->array[l], Enumerator);
						printf("%" PRIu64 ",%s,%s,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,wifi,%d\n", slot->slot_stop_time, src_mac, dst_mac, freq, glob->Global_Types->array[k], glob->Global_SubTypes->array[i], avg,std, avg_rssi, std_rssi, avg_lat, std_lat, slot->tag);
						// clear variables
						freq = 0; avg = 0; std = 0; flag = 0; min = 0; max = 0; avg_lat = 0; std_lat = 0; min_lat = 0; max_lat = 0; avg_dev = 0; x = 0; avg_rssi = 0.0; std_rssi = 0.0;
//						free(dif_array);
//						free(rssi_array);
//						free(latency_array);
//						free(val_array);
					} else if (freq == 1){
						char *src_mac = enum_find_frame_name(glob->Global_Sources->array[j], Enumerator);
						char *dst_mac = enum_find_frame_name(glob->Global_Destinations->array[l], Enumerator);
						avg = val_array[0]; std = 0; flag = 0; min = 0; max = 0; avg_lat = 0;
						 std_lat = 0; min_lat = 0; max_lat = 0; avg_dev = 0; x = 0; 
						avg_rssi = rssi_array[0]; std_rssi = 0.0;
						printf("%" PRIu64 ",%s,%s,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,wifi,%d\n", slot->slot_stop_time, src_mac, dst_mac, freq, glob->Global_Types->array[k], glob->Global_SubTypes->array[i], avg, std, avg_rssi, std_rssi, avg_lat, std_lat, slot->tag);
						freq = 0; avg = 0; std = 0; flag = 0; min = 0; max = 0; avg_lat = 0; std_lat = 0; min_lat = 0; max_lat = 0; avg_dev = 0; x = 0; avg_rssi = 0.0; std_rssi = 0.0;
//						free(dif_array);
//						free(val_array);
						//continue;
					}
					free(dif_array);
                                        free(rssi_array);
                                        free(latency_array);
                                        free(val_array);
						 /*else if (freq == 0){
						char src_mac[] = "0";
						char dst_mac[] = "0";
						freq = 0; avg = 0; std = 0; flag = 0; min = 0; max = 0; avg_lat = 0; std_lat = 0; min_lat = 0; max_lat = 0; avg_dev = 0; x = 0;
						printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
						free(dif_array);
						free(val_array);
						//continue;
					}*/
					l++;
				}
				j++;
			}
			k++;
		}
		i++;
	}
	if (slot->n == 0) printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,wifi,%d\n", slot->slot_stop_time, slot->tag);
}

void cpu_zbee_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator){
	unsigned int i = 0;
	unsigned int k = 0;
	unsigned int j = 0;
	unsigned int l = 0;
	unsigned int x = 0;
	unsigned int p = 0;

	unsigned char flag = 0;
	unsigned int freq = 0;
	double avg_sz = 0.0;
	double std_sz = 0.0;
	double avg_ttl= 0.0;
	double std_ttl= 0.0;
	unsigned int min_sz = 0;
	unsigned int max_sz = 0;
	unsigned int min_ttl= 0;
	unsigned int max_ttl= 0;
	double avg_dev = 0.0;
	double avg_lat = 0.0;
	double std_lat = 0.0;
	unsigned int  min_lat = 0;
	unsigned int  max_lat = 0;

	freq = 0; avg_sz = 0.0; avg_ttl = 0.0; avg_dev = 0.0; avg_lat = 0.0; std_sz = 0.0; std_ttl = 0.0; std_lat = 0.0;
	min_sz = 0; min_ttl = 0; min_lat = 0; max_sz = 0; max_ttl = 0; max_lat = 0;
//	printf("In the zigbee, the number in window %d, the glob_sources: %d\n", slot->n, glob->Global_Sources->n);
	while(i < glob->Global_Sources->n){	// for each source addr
		k = 0;
		while(k < glob->Global_Destinations->n){ // for each destination addr
			j = 0;
			while(j < glob->Global_Types->n){	// for each frame type
				FRAME *_frame = slot->frame_array;
				unsigned int *len_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
				uint64_t *time_dif_array = (uint64_t*)calloc(slot->n, sizeof(uint64_t));
				p =0;
				while(p < slot->n){ // for each packet
					zbee_struct_internal *frm = (zbee_struct_internal*)_frame->frame_ptr;
					
					if ( (glob->Global_Types->array[j] == frm->pkt_type) && (glob->Global_Sources->array[i] == frm->src_id) &&
					     (glob->Global_Destinations->array[k] == frm->dst_id)){
							time_dif_array[freq] = frm->timestamp;
							len_array[freq] = frm->len;
							freq++;
					}
					_frame = _frame->next;
					p++;
				}	
				if (freq > 1){
					// calculate latency
					unsigned int *latency_array = 0;
					latency_array = _math_generate_latency_array(time_dif_array, freq);
					if (latency_array != 0){
						qsort(latency_array, freq-1, sizeof(int), cmp);
						avg_lat = _math_average(latency_array, freq-1);
						std_lat = _math_stdev(_math_variance(latency_array, avg_lat, freq-1));
//						_math_minmax(latency_array, freq-1, &min_lat, &max_lat);
						if (isnan(std_lat)) std_lat = 0.0;
					}
					avg_sz = _math_average(len_array, freq);
					std_sz = _math_stdev(_math_variance(len_array, avg_sz, freq));
					avg_dev = _math_avg_dev(len_array, freq);
					if (isnan(avg_dev)) avg_dev= 0.0;
					if (isnan(std_sz)) std_sz = 0.0;
					if (isinf(avg_lat)) avg_lat = 0.0;
					if (isinf(std_lat)) std_lat = 0.0;
					if (isinf(std_sz)) std_lat = 0.0;
//					_math_minmax(len_array, freq, &min_sz, &max_sz);
					
					//char *src_name = enum_find_frame_name(glob->Global_Sources->array[j], Enumerator);
					//char *dst_name = enum_find_frame_name(glob->Global_Destinations->array[k], Enumerator);
					
					printf("%" PRIu64 ",%04x,%04x,%d,%d,%.2f,%.2f,%.2f,%.2f,zigbee,%d\n",
					slot->slot_stop_time, glob->Global_Sources->array[i], glob->Global_Destinations->array[k],
					// protocol
					glob->Global_Types->array[j],
					// freq
					freq,
					// size
					avg_sz, std_sz,//min_sz, max_sz, std_sz,
					// latency
					avg_lat, std_lat, slot->tag//min_lat, max_lat, std_lat
					 );
				} else if (freq == 1){
					_math_minmax(len_array, freq, &min_sz, &max_sz); avg_sz = min_sz; std_sz = 0; // one packet: avg = min = max, std = inf
					avg_lat = 0.0; std_lat = 0.0; min_lat = 0; max_lat = 0;

					printf("%" PRIu64 ",%04x,%04x,%d,%d,%.2f,%.2f,%.2f,%.2f,zigbee,%d\n",
					slot->slot_stop_time, glob->Global_Sources->array[i], glob->Global_Destinations->array[k],
					// protocol
					glob->Global_Types->array[j],
					// freq
					freq,
					// size
					avg_sz, std_sz,
					// latency
					avg_lat, std_lat, slot->tag
					 );
				} else if (freq == 0){
					//printf("0");
					//printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time); // possibly needs to move in to an if statement
				} else {
					printf("invalid case in processor.h freq is negative !!!");		
				}
				freq = 0; avg_sz = 0.0; avg_dev = 0.0; avg_lat = 0.0; std_sz = 0.0; std_lat = 0.0;
				min_sz = 0; min_lat = 0; max_sz = 0;  max_lat = 0; 
				j++;
			}
			k++;
		}
		i++;
	}
	if (slot->n == 0){
		// no packets were sent
		printf("%" PRIu64 ",0,0,0,0,0,0,0,0,zigbee,%d\n", slot->slot_stop_time, slot->tag); // possibly needs to move in to an if statement
	}
}

void cpu_ip_out(SLOT *slot, GLOBAL_KNOWLEDGE *glob, Enum_Type *Enumerator){
	unsigned int i = 0;
	unsigned int k = 0;
	unsigned int j = 0;
	unsigned int l = 0;
	unsigned int x = 0;
	unsigned int p = 0;

	unsigned char flag = 0;
	unsigned int freq = 0;
	double avg_sz = 0.0;
	double std_sz = 0.0;
	double avg_ttl= 0.0;
	double std_ttl= 0.0;
	unsigned int min_sz = 0;
	unsigned int max_sz = 0;
	unsigned int min_ttl= 0;
	unsigned int max_ttl= 0;
	double avg_dev = 0.0;
	double avg_lat = 0.0;
	double std_lat = 0.0;
	unsigned int  min_lat = 0;
	unsigned int  max_lat = 0;

	freq = 0; avg_sz = 0.0; avg_ttl = 0.0; avg_dev = 0.0; avg_lat = 0.0; std_sz = 0.0; std_ttl = 0.0; std_lat = 0.0;
	min_sz = 0; min_ttl = 0; min_lat = 0; max_sz = 0; max_ttl = 0; max_lat = 0;

	while(i < glob->Global_Sources->n){	// for each source ip
		k = 0;
		while(k < glob->Global_Destinations->n){ // for each destination ip
			j = 0;
			while(j < glob->Global_Types->n){	// for each protocol
				l = 0;
				while(l < glob->Global_SubTypes->n){ // for each src port
					x = 0;
					while(x < glob->Global_ExtTypes->n){ // for each dst port
						FRAME *_frame = slot->frame_array;
						unsigned int *len_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
						uint64_t *time_dif_array = (uint64_t*)calloc(slot->n, sizeof(uint64_t));
						unsigned int *ttl_array = (unsigned int*)calloc(slot->n, sizeof(int));
						p =0;
						while(p < slot->n){ // for each packet
							ip_struct_internal *frm = (ip_struct_internal*)_frame->frame_ptr;
							
							if ( (glob->Global_Types->array[j] == frm->protocol) && (glob->Global_Sources->array[i] == frm->src_ip) &&
							     (glob->Global_Destinations->array[k] == frm->dst_ip) && (glob->Global_SubTypes->array[l] == frm->src_port) && 
							     (glob->Global_ExtTypes->array[x] == frm->dst_port)){

								time_dif_array[freq] = frm->timestamp;
								len_array[freq] = frm->len;
								ttl_array[freq] = frm->ttl;
								freq++;
							}
							_frame = _frame->next;
							p++;
						}	
						if (freq > 1){
							// calculate latency
							unsigned int *latency_array = 0;
							latency_array = _math_generate_latency_array(time_dif_array, freq);
							if (latency_array != 0){
								qsort(latency_array, freq-1, sizeof(int), cmp);
								avg_lat = _math_average(latency_array, freq-1);
								std_lat = _math_stdev(_math_variance(latency_array, avg_lat, freq-1));
//								_math_minmax(latency_array, freq-1, &min_lat, &max_lat);
								if (isnan(std_lat)) std_lat = 0.0;
							}
							avg_sz = _math_average(len_array, freq);
							avg_ttl= _math_average(ttl_array, freq);

							std_sz = _math_stdev(_math_variance(len_array, avg_sz, freq));
							std_ttl= _math_stdev(_math_variance(ttl_array, avg_ttl, freq));
							avg_dev = _math_avg_dev(len_array, freq);

							if (isnan(avg_dev)) avg_dev= 0.0;
							if (isnan(std_sz)) std_sz = 0.0;
							if (isnan(std_ttl))std_ttl= 0.0;
//							_math_minmax(len_array, freq, &min_sz, &max_sz);
//							_math_minmax(ttl_array, freq, &min_ttl, &max_ttl);

							char *src_name = enum_find_frame_name(glob->Global_Sources->array[i], Enumerator);
							char *dst_name = enum_find_frame_name(glob->Global_Destinations->array[k], Enumerator);
							
							//printf("%" PRIu64 ",\e[32m%s\e[0m,\e[33m%s\e[0m,%d,%d,%d,%d,%.2f,%d,%d,%.2f,%.2f,%d,%d,%.2f,%.2f,%d,%d,%.2f\n",
							printf("%" PRIu64 ",%s,%s,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,ip,%d\n",
							slot->slot_stop_time, src_name, dst_name,
							// protocol
							glob->Global_Types->array[j],
							// src port
							glob->Global_SubTypes->array[l],
							// dst port
							glob->Global_ExtTypes->array[x],
							// freq
							freq,
							// ttl
							avg_ttl, std_ttl, //min_ttl, max_ttl, std_ttl,
							// size
							avg_sz,  std_sz, //min_sz, max_sz, std_sz,
							// latency
							avg_lat, std_lat, slot->tag //min_lat, max_lat, std_lat
							 );
						} else if (freq == 1){
							char *src_name = enum_find_frame_name(glob->Global_Sources->array[i], Enumerator);
							char *dst_name = enum_find_frame_name(glob->Global_Destinations->array[k], Enumerator);

							_math_minmax(len_array, freq, &min_sz, &max_sz); avg_sz = min_sz; std_sz = 0; // one packet: avg = min = max, std = inf
							_math_minmax(ttl_array, freq, &min_ttl, &max_ttl); avg_ttl = min_ttl; std_ttl = 0;
							avg_lat = 0.0; std_lat = 0.0; min_lat = 0; max_lat = 0;

							printf("%" PRIu64 ",%s,%s,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,ip,%d\n",
							slot->slot_stop_time, src_name, dst_name,
							// protocol
							glob->Global_Types->array[j],
							// src port
							glob->Global_SubTypes->array[l],
							// dst port
							glob->Global_ExtTypes->array[x],
							// freq
							freq,
							// ttl
							avg_ttl, std_ttl,
							// size
							avg_sz, std_sz,
							// latency
							avg_lat, std_lat, slot->tag
							 );
						} else if (freq == 0){
							//printf("0");
							//printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time); // possibly needs to move in to an if statement
						} else {
							printf("invalid case in processor.h freq is negative !!!");		
						}
						freq = 0; avg_sz = 0.0; avg_ttl = 0.0; avg_dev = 0.0; avg_lat = 0.0; std_sz = 0.0; std_ttl = 0.0; std_lat = 0.0;
						min_sz = 0; min_ttl = 0; min_lat = 0; max_sz = 0; max_ttl = 0; max_lat = 0; 
						x++;
						free(len_array);
						free(time_dif_array);
						free(ttl_array);
					}
					l++;
				}
				j++;
			}
			k++;
		}
		i++;
	}
	if (slot->n == 0){
		// no packets were sent
		printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,ip,%d\n", slot->slot_stop_time, slot->tag); // possibly needs to move in to an if statement
	}
}

void pro_wifi(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned proceed = 0;
        FRAME *_frame = slot->frame_array;
        while(f < glob->Global_Frames->n){
        while(k < glob->Global_Sources->n){
                while(j < glob->Global_Destinations->n){
                        _frame = slot->frame_array;
                        unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst case scenario
                        while(i < slot->n){
                                WiFi_Frame *frm = (WiFi_Frame*)_frame->frame_ptr;
                                if (frm->frame_type == (*(glob->Global_Frames->array+f))) proceed = 1;
                                if (proceed && (*(glob->Global_Sources->array+k) == frm->src_id) && (*(glob->Global_Destinations->array+j) == frm->dst_id)){
                                        val_array[freq] = frm->frame_length;
                                        freq++;
                                        avg += frm->frame_length;
                                }
                                _frame = _frame->next;
                                proceed = 0;
                                i++;
                        }
                        avg = _math_average(val_array, freq);
                        stdev = _math_stdev(_math_variance(val_array, avg, freq));
                        avg_dev = _math_avg_dev(val_array, freq);
                        if (isnan(avg_dev)) avg_dev = 0.0;
                        if (isnan(stdev)) stdev = 0.0;
                        unsigned int min = 0;
                        unsigned int max = 0;
                        _math_minmax(val_array, freq, &min, &max);
                        if (freq != 0)printf("%" PRIu64 ",%d,%d,%d,%d,%f,%f,%f,%d,%d\n", slot->slot_stop_time,
                                (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)), (*(glob->Global_Frames->array+f)), freq, avg, stdev,avg_dev, min, max);
                        freq = 0;
                        avg = 0.0;
                        i = 0;
                        free(val_array);
                        j++;
                }
                j = 0;
                k++;
        }
        k = 0; j = 0;
        f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}

void pro_ip_short(SLOT *slot, GLOBAL_KNOWLEDGE *glob){
        unsigned int i = 0;
        unsigned int k = 0;
        unsigned int j = 0;
        unsigned int freq = 0;
        unsigned int f = 0;
        double avg = 0.0;
        double stdev = 0.0;
        double avg_dev = 0.0;
        unsigned char proceed = 0;
        FRAME *_frame = slot->frame_array;
        while (f < glob->Global_Frames->n){
                while (k < glob->Global_Sources->n){   // for every source
                        while(j < glob->Global_Destinations->n){ // for every destination
                                _frame = slot->frame_array;
                                unsigned int *val_array = (unsigned int*)calloc(slot->n, sizeof(int)); // worst$
                                while(i < slot->n){ // for each packet
                                        IP_Frame *frm = (IP_Frame*)_frame->frame_ptr;
                                        if (frm->protocol == (*(glob->Global_Frames->array+f))) proceed = 1;
                                        if (proceed && (*(glob->Global_Sources->array+k) == frm->src_ip) && (*(glob->Global_Destinations->array+j) == frm->dst_ip)){
                                                val_array[freq] = frm->packet_size;
                                                freq++;
                                                avg += frm->packet_size;
                                        }
                                        _frame = _frame->next;
                                        proceed = 0;
                                        i++;
                                }
                                avg = _math_average(val_array, freq);
                                stdev = _math_stdev(_math_variance(val_array, avg, freq));
                                avg_dev = _math_avg_dev(val_array, freq);
                                if (isnan(avg_dev)) avg_dev=0.0;
                                if (isnan(stdev)) stdev = 0.0;
                                unsigned int min = 0;
                                unsigned int max = 0;
                                _math_minmax(val_array, freq, &min, &max);
                                if (freq != 0)printf("%" PRIu64 ",%d,%d,%d,%d,%f,%f,%f,%d,%d\n", slot->slot_stop_time,
                                         (*(glob->Global_Sources->array+k)), (*(glob->Global_Destinations->array+j)),(*(glob->Global_Frames->array+f)), freq, avg, stdev,avg_dev, min, max);
                                freq = 0;
                                avg = 0.0;
                                i = 0;
                                free(val_array);
                                j++;
                        }
                        j = 0;
                        k++;
                }
                k = 0; j = 0;
                f++;
        }
        glob->Global_Sources->n = 0;
        glob->Global_Destinations->n = 0;
        glob->Global_Frames->n = 0;
        i = 0;
}


void process_slot(SLOT *slot, GLOBAL_KNOWLEDGE *glob, unsigned char type){

	switch(type){
		case(ZBEE_TYPE):
				pro_zbee(slot, glob);
			break;
		case(WIFI_TYPE):
				pro_wifi(slot, glob);
			break;
		case(IPSH_TYPE):
				pro_ip_short(slot, glob);
			break;
		case(AUDI_TYPE):
				pro_audio(slot);
			break;
		default:

			break;
	}

}
