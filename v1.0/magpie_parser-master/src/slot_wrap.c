#include "slot_wrap.h" 
#include "pthread.h"

void free_slot_frame_type(SLOT *slot, unsigned char type){
	// wifi_struct_internal = 1
	unsigned int i = 0;
	unsigned int k = 0;
	FRAME *ptr = slot->frame_array;
	switch(type){
		case 1:
			while(i < slot->n){
				ptr = slot->frame_array;
				while(k < i){ ptr = ptr->next; k++; }
				k = 0;
				if (ptr == 0){ printf("free_slot_frame_type func failed, abnormal condition\n"); break;}
//				free( ((wifi_struct_internal*)(ptr->frame_ptr))->src_mac );
//				free( ((wifi_struct_internal*)(ptr->frame_ptr))->dst_mac );
				i++;
			}
			break;
	}
}
unsigned char validate_object(SLOT *slot, void *object, unsigned char type){
	if (object == 0){
		switch(type){
			case 1:	//
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				return 0;
				break;
			case 2:
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				return 0;
				break;
			case 3:
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time); // possibly needs to move in to an if statement
				return 0;
				break;
			case 4:
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				return 0;
				break;
			case 5:
				printf("%" PRIu64 ",0,0,0,0,0\n", slot->slot_stop_time);
				return 0;
				break;
			default:
				break;
		}
	}
	return 1;
}
void process_audio_test(SLOT *slot){
	if (slot->n > 0){
        	double *dbl_array = (double*)calloc(slot->n, sizeof(double));
                FRAME *spec_frm = slot->frame_array;
                unsigned int rms_count = 0;
                unsigned int freq = 0;
                double min_rms = 0; double max_rms=0; double std_rms = 0.0; double avg_rms = 0.0;
                int i = 0;
                while(i < slot->n){
                	if (((audio_struct_internal*)(spec_frm->frame_ptr))->value > 0.9){
                        	rms_count++;
                                dbl_array[i] = ((audio_struct_internal*)(spec_frm->frame_ptr))->value;
                        }
                        spec_frm = spec_frm->next;
                        i++;
		}
                if (rms_count != 0){
                	avg_rms = _math_average_dbl(dbl_array, rms_count);
                        std_rms = _math_stdev(_math_variance_dbl(dbl_array, avg_rms, rms_count));
                        if (isnan(std_rms)) std_rms = 0;
                        if (isnan(avg_rms)) avg_rms = 0;
                        if (isinf(std_rms)) std_rms = 0;
                        if (isinf(avg_rms)) avg_rms = 0;
//                        _math_minmax_dbl(dbl_array, rms_count, &min_rms, &max_rms);
		} else {
			rms_count = 0;
			avg_rms = 0.0;
			std_rms = 0.0;
		}
                free(dbl_array);
                printf("%" PRIu64 ",%d,%.2f,%.2f,audio,%d\n", slot->slot_stop_time, rms_count, avg_rms,std_rms, slot->tag);
        	fflush(stdout);
	}
	if (slot->n == 0) {
		printf("%" PRIu64 ",0,0.00,0.00,audio,%d\n", slot->slot_stop_time, slot->tag);
		fflush(stdout);
	}
}
void *analyse_thread_IP(void *Some_Structure){
		PT_GLOB *test = (PT_GLOB*)Some_Structure;
//		sem_wait(test->semaphore);
		// if Some_Structure->busy_flag == 1
		// sem_wait();
		// process
		GLOBAL_KNOWLEDGE *_glob = 0;
		_glob = perform_global_features(test->slot, test->type);
//		if (test->slot->n == 0){
//			uint64_t time;
//			if (test->slot->frame_array->frame_ptr == NULL) time = 0;
//			test->slot->slot_start_time = time; 
//			test->slot->slot_stop_time = time+(1000000*test->window); 
//		}//ms
		if (test->slot->slot_start_time == 0){
			if (test->slot->n != 0){
				test->slot->slot_start_time = *(uint64_t*)(test->slot->frame_array->frame_ptr);
				test->slot->slot_stop_time = (test->slot->slot_start_time)+(1000000*test->window);
			} else { fflush(stdout); }
		}
		switch(test->type){
			case 1:
				cpu_wifi_out(test->slot, _glob, test->Addresses);
				break;
			case 2:
				cpu_ip_out(test->slot, _glob, test->Addresses);
				break;
			case 3:
				cpu_zbee_out(test->slot, _glob, test->Addresses);
				break;
			case 4:
				break;
			case 5:
				process_audio_test(test->slot);
				break;
			default:
				break;
		}
		// sem_post();
//		sem_post(test->semaphore);
		free(_glob);
		free_slot(test->slot);
		test->slot->slot_start_time = test->slot->slot_stop_time;
                test->slot->slot_stop_time = test->slot->slot_start_time +(1000000*test->window);
		return 0;
}
void analyse_slot_add(SLOT *slot, void *object, unsigned char object_size, unsigned char type, Enum_Type *Enumerator, unsigned char *process_flag, unsigned char window_size, LOCAL_SOCKET *sock){
	/*if (validate_object(slot, object, type) == 0){
		// update slot times
		slot->slot_start_time = slot->slot_stop_time; slot->slot_stop_time += (1000000*window_size);
		return;
	}*/
	uint64_t time = *((uint64_t*)object);
	if (slot->n == 0){ slot->slot_start_time = time; slot->slot_stop_time = time+(1000000*window_size); }//ms
	
	if ((time >= slot->slot_stop_time && (*process_flag == 5)) || (*process_flag == 1) || ( (time >= slot->slot_stop_time) && ( type == 4) )){ // if current time of an object is higher then stop time i.e. exceeds
		// process current slotn
		GLOBAL_KNOWLEDGE *_glob = 0;// = perform_global_features(slot, 1); // 1 wifi
		switch(type){ // || ( (time >= slot->slot_stop_time) && ( type == 4) )
			case 1: //wifi
				_glob = perform_global_features(slot, type);
				cpu_wifi_out(slot, _glob, Enumerator);

				break;
			case 2: // ip
				_glob = perform_global_features(slot, type);
				cpu_ip_out(slot, _glob, Enumerator);
				
				break;
			case 3: // zigbee
				_glob = perform_global_features(slot, type);
				cpu_zbee_out(slot, _glob, Enumerator);
				break;
			case 4: // ...............................................................................................
				if (slot->n >= 47*window_size){
						PDU pdu;
						pdu.timestamp = 0;
						pdu.command = 2;
						write_data_to_socket(sock, (char*)&pdu, sizeof(pdu));
						read_data_from_socket(sock, (char*)&pdu, sizeof(pdu));
						int timestamp = pdu.timestamp;
						if (slot->n <= 47*2){ // if less than 3 seconds, no point in processing avg, std etc...
							FRAME *spec_frm = slot->frame_array;
							((spec_struct_internal*)(spec_frm->frame_ptr))->timestamp =  timestamp;
							printf("% " PRIu64 "", ((spec_struct_internal*)(spec_frm->frame_ptr))->timestamp);
							int i = 0;
							while(i < slot->n){
								printf(",%.2f", ((spec_struct_internal*)(spec_frm->frame_ptr))->array[0]);
								spec_frm = spec_frm->next;
								i++;
							}
							printf("\n");
							fflush(stdout);
							free_slot(slot);
							frame_add(slot, object, object_size, 1); // type needs to be 1, can be removed later
							slot->slot_start_time = slot->slot_stop_time; slot->slot_stop_time += (1000000*window_size);
						} else { // 3 or higher
							FRAME *spec_frm = slot->frame_array;
							((spec_struct_internal*)(spec_frm->frame_ptr))->timestamp =  timestamp;
							printf("% " PRIu64 "", ((spec_struct_internal*)(spec_frm->frame_ptr))->timestamp);
                                                        int i = 0;
							int k = 0;
							double value_array[47][window_size];
							while(i < window_size){
								while(k < 47){
									value_array[k][i] = ((spec_struct_internal*)(spec_frm->frame_ptr))->array[0];
									spec_frm = spec_frm->next;
									k++;
								}
								k = 0;
								i++;
							}
							i = 0;
							while(i < 47){
								double avg = _math_average_dbl(value_array[i], window_size);
								double std = _math_stdev(_math_variance_dbl(value_array[i], avg, window_size));
								printf(",%.2f,%.2f", avg, std);
								i++;
							}
							printf(",rf,%d\n", slot->tag);
							if (slot->tag == 0){ slot->tag = 1; } else { slot->tag = 0; }
							fflush(stdout);
							free_slot(slot);
							frame_add(slot, object, object_size, 1);
							slot->slot_start_time = slot->slot_stop_time; slot->slot_stop_time += (1000000*window_size);
						}
						return;
				}
				break;
			case 5: // audio
				if (slot->n > 0){
						double *dbl_array = (double*)calloc(slot->n, sizeof(double));
						FRAME *spec_frm = slot->frame_array;
						unsigned int rms_count = 0;
						unsigned int freq = 0;
						double min_rms = 0; double max_rms=0; double std_rms = 0.0; double avg_rms = 0.0;
						int i = 0;
						while(i < slot->n){
							if (((audio_struct_internal*)(spec_frm->frame_ptr))->value > 0.9){
								 rms_count++;
								 dbl_array[i] = ((audio_struct_internal*)(spec_frm->frame_ptr))->value;
							}
							spec_frm = spec_frm->next;
							i++;
						}
						if (rms_count != 0){
							avg_rms = _math_average_dbl(dbl_array, rms_count);
							std_rms = _math_stdev(_math_variance_dbl(dbl_array, avg_rms, rms_count));
							if (isnan(std_rms)) std_rms = 0;
							if (isnan(avg_rms)) avg_rms = 0;
							if (isinf(std_rms)) std_rms = 0;
							if (isinf(avg_rms)) avg_rms = 0;
							_math_minmax_dbl(dbl_array, rms_count, &min_rms, &max_rms);
						}	
						free(dbl_array);
						printf("%" PRIu64 ",%d,%.2f,%.2f,%.2f,%.2f\n", slot->slot_stop_time, rms_count, avg_rms, min_rms, max_rms,std_rms);
						fflush(stdout);
				}
				break;
			default:
				printf("Defaulted in analyse slot_add (slot_wrap.c)");
				break;
		}
		
		if (_glob != 0) global_knowledge_free(_glob);
		// free the slot, taking into account types
//		free_slot_frame_type(slot, type);
		free_slot(slot);

		// set current object as 
		frame_add(slot, object, object_size, 1); // type needs to be 1, can be removed later
		slot->slot_start_time = slot->slot_stop_time;
		slot->slot_stop_time = slot->slot_start_time +(1000000*window_size);
		if (*process_flag == 1) *process_flag = 2;
	} else if (((time >= slot->slot_start_time) && (*process_flag == 5)) || (*process_flag == 0) || (*process_flag == 4)){ // within the range start <-> stop times, good
		frame_add(slot, object, object_size, 1);
	} else {	// something went wrong for sure
		printf("analyse_slot_add func failed, abnormal if condition! \n");
	}

}
unsigned short *add_short_to_array(unsigned short *array, unsigned short val, unsigned int index){
	unsigned short *_temp_array = array;
	if (_temp_array == 0){ _temp_array = (unsigned short*)calloc(index, sizeof(short)); _temp_array[0] = val; return _temp_array; }
        unsigned short *_new_array = (unsigned short*)calloc(index, sizeof(short));
        memcpy(_new_array, _temp_array, sizeof(short)*(index-1));
        free(_temp_array);
        _new_array[index-1] = val;
	return _new_array;
}


GLOBAL_KNOWLEDGE *perform_global_features(SLOT *slot, unsigned char type){
	GLOBAL_KNOWLEDGE *_glob = global_knowledge_init();
	unsigned int i = 0;
	unsigned int unique_type = 0;
	unsigned int unique_subtype = 0;
	unsigned int unique_exttype = 0;
	unsigned int unique_src = 0;
	unsigned int unique_dst = 0;

	unsigned short temp = 0;
	unsigned short *type_array;
	unsigned short *subtype_array;
	unsigned short *exttype_array;
	unsigned short *src_array;
	unsigned short *dst_array;

	FRAME *_frame = slot->frame_array;

	while(i < slot->n){ // go through all frames

		switch(type){
			case 1:		// wifi struct internal
				if (_frame != 0){
					if (unique_type == 0){	// first value
						type_array = (unsigned short*)calloc(1, sizeof(short));
						*type_array = ((wifi_struct_internal*)_frame->frame_ptr)->type;
						unique_type++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_type){	// cycle through all unique types
							if(type_array[k] == ((wifi_struct_internal*)_frame->frame_ptr)->type){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_type++;
							type_array = add_short_to_array(type_array, ((wifi_struct_internal*)_frame->frame_ptr)->type, unique_type);
						}
					}
					if (unique_subtype == 0){
						subtype_array = (unsigned short*)calloc(1, sizeof(short));
						*subtype_array = ((wifi_struct_internal*)_frame->frame_ptr)->subtype;
						unique_subtype++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_subtype){	// cycle through all unique types
							if(subtype_array[k] == ((wifi_struct_internal*)_frame->frame_ptr)->subtype){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_subtype++;
							subtype_array = add_short_to_array(subtype_array, ((wifi_struct_internal*)_frame->frame_ptr)->subtype, unique_subtype);
						}
					}
					if (unique_src == 0){
						src_array = (unsigned short*)calloc(1,sizeof(short));
						*src_array = ((wifi_struct_internal*)_frame->frame_ptr)->src_mac;
						unique_src++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_src){	// cycle through all unique types
							if(src_array[k] == ((wifi_struct_internal*)_frame->frame_ptr)->src_mac){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_src++;
							src_array = add_short_to_array(src_array, ((wifi_struct_internal*)_frame->frame_ptr)->src_mac, unique_src);
						}
					}
					if (unique_dst == 0){
						dst_array = (unsigned short*)calloc(1,sizeof(short));
						*dst_array= ((wifi_struct_internal*)_frame->frame_ptr)->dst_mac;
						unique_dst++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_dst){	// cycle through all unique types
							if(dst_array[k] == ((wifi_struct_internal*)_frame->frame_ptr)->dst_mac){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_dst++;
							dst_array = add_short_to_array(dst_array, ((wifi_struct_internal*)_frame->frame_ptr)->dst_mac, unique_dst); //_new_array;
						}
					}
				}
				break;
			case 2:		// IP struct internal
				if (_frame != 0){
					if (unique_type == 0){	// first value	// proto
						type_array = (unsigned short*)calloc(1, sizeof(short));
						*type_array = ((ip_struct_internal*)_frame->frame_ptr)->protocol;
						unique_type++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_type){	// cycle through all unique types
							if(type_array[k] == ((ip_struct_internal*)_frame->frame_ptr)->protocol){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_type++;
							type_array = add_short_to_array(type_array, ((ip_struct_internal*)_frame->frame_ptr)->protocol, unique_type);
						}
					}
					if (unique_subtype == 0){	// src_port
						subtype_array = (unsigned short*)calloc(1,sizeof(short));
						*subtype_array= ((ip_struct_internal*)_frame->frame_ptr)->src_port;
						unique_subtype++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_subtype){	// cycle through all unique types
							if(subtype_array[k] == ((ip_struct_internal*)_frame->frame_ptr)->src_port){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_subtype++;
							subtype_array = add_short_to_array(subtype_array, ((ip_struct_internal*)_frame->frame_ptr)->src_port, unique_subtype); //_new_array;
						}
					}
					if (unique_exttype == 0){ 	// dst port
						exttype_array = (unsigned short*)calloc(1,sizeof(short));
						*exttype_array= ((ip_struct_internal*)_frame->frame_ptr)->dst_port;
						unique_exttype++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_exttype){	// cycle through all unique types
							if(exttype_array[k] == ((ip_struct_internal*)_frame->frame_ptr)->dst_port){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_exttype++;
							exttype_array = add_short_to_array(exttype_array, ((ip_struct_internal*)_frame->frame_ptr)->dst_port, unique_exttype); //_new_array;
						}
					}
					if (unique_src == 0){
						src_array = (unsigned short*)calloc(1,sizeof(short));
						*src_array = ((ip_struct_internal*)_frame->frame_ptr)->src_ip;
						unique_src++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_src){	// cycle through all unique types
							if(src_array[k] == ((ip_struct_internal*)_frame->frame_ptr)->src_ip){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_src++;
							src_array = add_short_to_array(src_array, ((ip_struct_internal*)_frame->frame_ptr)->src_ip, unique_src);
						}
					}
					if (unique_dst == 0){
						dst_array = (unsigned short*)calloc(1,sizeof(short));
						*dst_array= ((ip_struct_internal*)_frame->frame_ptr)->dst_ip;
						unique_dst++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_dst){	// cycle through all unique types
							if(dst_array[k] == ((ip_struct_internal*)_frame->frame_ptr)->dst_ip){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_dst++;
							dst_array = add_short_to_array(dst_array, ((ip_struct_internal*)_frame->frame_ptr)->dst_ip, unique_dst); //_new_array;
						}
					}
				}
				
				break;
			case 3:		// ZBee
				if (_frame != 0){
					if (unique_type == 0){	// first value
						type_array = (unsigned short*)calloc(1, sizeof(short));
						*type_array = ((zbee_struct_internal*)_frame->frame_ptr)->pkt_type;
						unique_type++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_type){	// cycle through all unique types
							if(type_array[k] == ((zbee_struct_internal*)_frame->frame_ptr)->pkt_type){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_type++;
							type_array = add_short_to_array(type_array, ((zbee_struct_internal*)_frame->frame_ptr)->pkt_type, unique_type);
						}
					}
					if (unique_src == 0){
						src_array = (unsigned short*)calloc(1,sizeof(short));
						*src_array = ((zbee_struct_internal*)_frame->frame_ptr)->src_id;
						unique_src++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_src){	// cycle through all unique types
							if(src_array[k] == ((zbee_struct_internal*)_frame->frame_ptr)->src_id){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_src++;
							src_array = add_short_to_array(src_array, ((zbee_struct_internal*)_frame->frame_ptr)->src_id, unique_src);
						}
					}
					if (unique_dst == 0){
						dst_array = (unsigned short*)calloc(1,sizeof(short));
						*dst_array= ((zbee_struct_internal*)_frame->frame_ptr)->dst_id;
						unique_dst++;
					} else {
						unsigned int k = 0;
						unsigned char found = 0;
						while(k < unique_dst){	// cycle through all unique types
							if(dst_array[k] == ((zbee_struct_internal*)_frame->frame_ptr)->dst_id){
								found = 1;
								break;
							}
							k++;
						}
						if (found != 1){	// if not found then add
							unique_dst++;
							dst_array = add_short_to_array(dst_array, ((zbee_struct_internal*)_frame->frame_ptr)->dst_id, unique_dst); //_new_array;
						}
					}
				}
				break;
			default:
			
				break;
		}

		_frame = _frame->next;
		i++;
	}
	_glob->Global_Types->n = unique_type;
	_glob->Global_SubTypes->n = unique_subtype;
	_glob->Global_ExtTypes->n = unique_exttype;
	_glob->Global_Sources->n = unique_src;
	_glob->Global_Destinations->n = unique_dst;

        _glob->Global_Types->array = type_array;
        _glob->Global_SubTypes->array = subtype_array;
	_glob->Global_ExtTypes->array = exttype_array;
        _glob->Global_Sources->array = src_array;
        _glob->Global_Destinations->array = dst_array;

	return _glob;
}
