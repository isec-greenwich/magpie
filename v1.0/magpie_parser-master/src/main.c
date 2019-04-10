#include "signal.h"
#include "argread.h"
#include "int_audio.h"
#include "int_ipshort.h"
#include "int_wifi.h"
#include "int_zbee.h"
#include "int_spectrum.h"
#include "slot_wrap.h"
#include "pthread.h"
#include "semaphore.h"

#define FILENAME_BUFFER 128
char *interface = (char*)calloc(40, sizeof(char));
char *descriptor_filename_start;

struct ZigBee_Frame Global_ZB_Pkt;
struct Enum_Type Enum_Start;
struct Enum_Type WiFi_Address;
struct Enum_Type IP_Address;
struct Enum_Type Proto_Address;

unsigned int Enum_Index = 0;
unsigned char live_descriptor_write = 1;
unsigned int Version = 1;
unsigned long line_offset_increments = 1;

char cpu_time_file[64];

unsigned long mlk_alloc = 0;
unsigned long mlk_free = 0;
unsigned char eapol_flag = 0;

unsigned char process_flag = 5;
unsigned char window_seconds = 0;
unsigned char busy_processing = 0;
unsigned char synchronized = 0;
unsigned long start_proc_epoch = 0;
Enum_Type *Enumerator_Addr = (Enum_Type *)calloc(1, sizeof(Enum_Type));
Enum_Type *Enumerator_Proto = (Enum_Type *)calloc(1, sizeof(Enum_Type));
SLOT *slot = slot_init();
PT_GLOB pt_struct;
LOCAL_SOCKET *cli_sock = generate_free_socket();
PDU pdu;
unsigned short argument_flags = 0;
int synchro_seconds = 0;

sem_t semaphore;

void *thread_sleep(void *num){
	int *window = (int*)num;
	while(1){
//		usleep(1000000*(*window));
//		char origin[25];
//		if ((argument_flags & ZIGB_FLAG) != 0)sprintf(origin, "Zigbee");
//		if ((argument_flags & WIFI_FLAG) != 0)sprintf(origin, "Wifi");
//		if ((argument_flags & IP_SHOR_F) != 0)sprintf(origin, "IP Short");
//		if ((argument_flags & AUDIO_FLA) != 0)sprintf(origin, "Audio");
		read_data_from_socket(cli_sock, (char*)&pdu, sizeof(pdu));
		fprintf(stderr, "\nD - Synchro thread has received input. [%s]\n", interface);
//		printf("%s Received: cmd: %d time: %d!\n", origin, pdu.command, pdu.timestamp);
		fflush(stdout);
		pt_struct.slot->slot_start_time = pdu.timestamp;
		pt_struct.slot->slot_stop_time = pdu.timestamp;
		if (pdu.command == 1) busy_processing = 1;
//		if ((busy_processing == 0) && (synchronized == 1)) busy_processing = 1;
	}
	return 0;
}

void *thread_process(void *structure){
	while(1){
		if (busy_processing == 1){
			sem_wait(&semaphore);
//			fprintf(stderr, "\nD - Process thread has started. [%s]\n", interface);
//			usleep(400000);
			clock_t t;
			t = clock();
			analyse_thread_IP(structure);
			if (((PT_GLOB*)structure)->slot->tag == 0){ ((PT_GLOB*)structure)->slot->tag = 1; } else { ((PT_GLOB*)structure)->slot->tag = 0; }
			t = clock() - t;
			if ((argument_flags & CPU_TIMES) != 0){
				FILE *_file = fopen(cpu_time_file, "a");
				fprintf(_file, "%.6f\n", ((double)t)/CLOCKS_PER_SEC);
				fclose(_file);
			}
//			printf("~\n"); fflush(stdout);
			fflush(stdout);
//			fprintf(stderr, "\nD - Process thread released. [%s]\n", interface);
			sem_post(&semaphore);
			busy_processing = 0;
		}
		usleep(30000); // solving full cpu load
	}
	return 0;
}
void *thread_synchronize(void *s){
	while(synchronized == 0){
		if (synchronized == 0){
			fprintf(stderr, "\nD - Synchro Old Thread.\n");
			unsigned long epoch = (unsigned long)time(NULL);
			if (epoch > start_proc_epoch) synchronized = 1;
//			printf("%lu / %lu\n", epoch, start_proc_epoch);
//			fflush(stdout);
			usleep(10000);
		}
	}
	return 0;
}
void *thread_conv_line_slot(void *pt_struct){
	SLOT *_t_slot = ((PT_GLOB*)pt_struct)->slot;
	char buffer[2048];
	while((fgets(buffer, 2048, stdin) != NULL) && (synchronized == 0)){
		fprintf(stderr, "\nD - Convert thread is dropping [%s].\n", interface);
		if (synchronized == 1) break;
	}
	while((fgets(buffer, 2048, stdin) != NULL)){
		sem_wait(&semaphore);
//		if (argument_flags & AUDIO_FLA) fprintf(stderr, "\nD - Convert thread has started. [%s]\n", interface);
		if (busy_processing != 1){
			void *structure = 0;
			unsigned char obj_size = 0;
			switch(((PT_GLOB*)pt_struct)->type){
				case 1:
					structure = calloc(1,sizeof(wifi_struct_internal));
					pro_wifi_int(buffer, (wifi_struct_internal*)structure, Enumerator_Addr);
					obj_size = sizeof(wifi_struct_internal);
					break;
				case 2:
					structure = calloc(1,sizeof(ip_struct_internal));
					pro_short_int(buffer, (ip_struct_internal*)structure, Enumerator_Addr);
					obj_size = sizeof(ip_struct_internal);
					break;
				case 3:
					structure = calloc(1,sizeof(zbee_struct_internal));
					pro_zbee_int(buffer, (zbee_struct_internal*)structure, Enumerator_Addr);
					obj_size = sizeof(zbee_struct_internal);
					break;
				case 4:
					// spec
					break;
				case 5:
					// audio
					structure = calloc(1, sizeof(audio_struct_internal));
					pro_audio_int(buffer, (audio_struct_internal*)structure);
					obj_size = sizeof(audio_struct_internal);
					break;

				default:
					printf("Default case in thread, Abort!\n");
					exit(0);
			}
			frame_add(_t_slot, structure, obj_size, 1);
			//fprintf(stderr, "\nD - Buffer: %s.\n", buffer);

//			ip_struct_internal *test_ip = (ip_struct_internal*)calloc(1,sizeof(ip_struct_internal));
//			pro_short_int(buffer, test_ip, Enumerator_Addr);
//			frame_add(_t_slot, test_ip, sizeof(ip_struct_internal), 1);
//			printf(".");
		}
//		if (argument_flags & AUDIO_FLA) fprintf(stderr, "\nD - Convert thread released. [%s]\n", interface);
		sem_post(&semaphore);
	}
	return 0;
}

void test_main(unsigned char type){
	SLOT *t_slot = slot_init();
//	PT_GLOB pt_struct;
	pt_struct.slot = t_slot;
	pt_struct.Addresses = Enumerator_Addr;
	pt_struct.semaphore = &semaphore;
	pt_struct.type = type; // for IP test
	pt_struct.window = window_seconds;
	pthread_t process_lines, timer, process, synchro;
//	printf("Stage 0: Synchro\n");
	pthread_create(&synchro, NULL, thread_synchronize, NULL);
	pthread_create(&process_lines, NULL, thread_conv_line_slot, (void*)&pt_struct);
	pthread_create(&timer, NULL, thread_sleep, (void*)&pt_struct.window);
	pthread_create(&process, NULL, thread_process,(void*)&pt_struct);
	pthread_join(synchro, NULL);
	pthread_join(process_lines, NULL);
	pthread_join(timer,NULL);
	pthread_join(process, NULL);
	sem_destroy(&semaphore);
	exit(0);
}


void showHelpMessage(){
	printf("Usage: C_Parser -[io] (filename) -[lwzhsa] \n");
	printf(" -i\tInput file parameter followed by the filename \n");
	printf(" -o\tOutput file parameter followed by the filename \n");
	printf(" -l\tLive mode (pipe with tshark output)\n");
	printf(" -z\tZigBee input\n");
	printf(" -w\tWifi input\n");
	printf(" -s\tIP Short input\n");
	printf(" -a\tAudio Input\n");
	printf(" -t\tOutput to stdout");
	printf(" -h\tThis help menu\n");
}

void middleware_handler(char *line_buffer){
	
}
unsigned int identify_arg(){
	if ((argument_flags & ZIGB_FLAG) == ZIGB_FLAG) return 1;
	if ((argument_flags & WIFI_FLAG) == WIFI_FLAG) return 2;
	if ((argument_flags & AUDIO_FLA) == AUDIO_FLA) return 3;
	if ((argument_flags & SPECT_FLA) == SPECT_FLA) return 4;
	if ((argument_flags & IP_SHOR_F) == IP_SHOR_F) return 5;
	return 0;
}
void int_sigalarm(int sig){
	//printf("\n!!! INTERRUPT !!!\n");
	process_flag = 1;
	if (slot->n < 1){
		switch(identify_arg()){
			case 1:	// wifi
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				break;
			case 2:	// ip
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				break;
			case 3:
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time); // possibly needs to move in to an if statement
				break;
			case 4: // spect
				printf("%" PRIu64 ",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n", slot->slot_stop_time);
				break;
			case 5:
				printf("%" PRIu64 ",0,0,0,0,0\n", slot->slot_stop_time);
				break;
			default:
				break;
		}
		slot->slot_stop_time = slot->slot_start_time+(1000000*window_seconds); slot->slot_start_time = slot->slot_stop_time; 
	}
	alarm(window_seconds);
}

void prep_cpu_time_writeout(char *file, unsigned short flags){
	unsigned short _flags = (flags & WIFI_FLAG) | (flags & ZIGB_FLAG) | (flags & IP_SHOR_F) | (flags & AUDIO_FLA) | (flags & SPECT_FLA);
	switch(_flags){
		case WIFI_FLAG:
			sprintf(file, "wifi_t_cpu.csv");
			break;
		case ZIGB_FLAG:
			sprintf(file, "zbee_t_cpu.csv");
			break;
		case IP_SHOR_F:
			sprintf(file, "ip_s_t_cpu.csv");
			break;
		case AUDIO_FLA:
			sprintf(file, "audi_t_cpu.csv");
			break;
		case SPECT_FLA:
			sprintf(file, "spec_t_cpu.csv");
			break;
		default:
			sprintf(file, "unhandled.csv");
			break;
	}
}
int main(int argc, char *argv[])
{
	//printf("Hello World, %s\n", argv[0]);
	char *path = extract_path(argv[0]);
//	usleep(1000000); // delay to allow to initially
	//Enum_Type *Enumerator_Addr = (Enum_Type *)calloc(1, sizeof(Enum_Type));
	//Enum_Type *Enumerator_Proto = (Enum_Type *)calloc(1, sizeof(Enum_Type));
//	enum_init(&Enum_Start);
//	enum_init(&WiFi_Address);
//	enum_init(&IP_Address);
//	enum_init(&Proto_Address);
	// Initialization of required variables
	uint64_t *line_offsets = 0;
	uint64_t line_offset_index = 0;
	char *in_filename_ptr = (char*)calloc(FILENAME_BUFFER, sizeof(char));
	char *out_filename_ptr = (char*)calloc(FILENAME_BUFFER, sizeof(char));

	// process arguments
	
	argument_flags = argument_flagger(argc, argv, argument_flags, in_filename_ptr, out_filename_ptr, &window_seconds, &start_proc_epoch);
	prep_cpu_time_writeout(cpu_time_file, argument_flags);
	if ((argument_flags & SYNCHRONI) == 0) synchronized = 1;
	synchronized = 1;
//	printf("debug: %lu\n", start_proc_epoch);
	//load_maps();
	load_maps(argument_flags, Enumerator_Addr, argv[0]);
	unsigned char active_slot = 0;
	if (argument_flags & ZIGB_FLAG) memcpy(interface, "Zigbee", 6);
	if (argument_flags & WIFI_FLAG) memcpy(interface, "WIFI", 4);
	if (argument_flags & AUDIO_FLA) memcpy(interface, "Audio", 5);
	if (argument_flags & IP_SHOR_F) memcpy(interface, "IP", 2);
	if (window_seconds != 0 ){
		if ((argument_flags & SPECT_FLA) != SPECT_FLA){
//			signal(SIGALRM, int_sigalarm);
//			alarm(window_seconds); process_flag = 4;
		}
	} else {
		window_seconds = 1;
	}
	if ((argument_flags & STDOUT_FL) == STDOUT_FL){
		in_filename_ptr = (char*) calloc(1, sizeof(char));
		out_filename_ptr= (char*) calloc(1, sizeof(char));
	}
	if (((argument_flags & HELP_FLAG) != 0)){ showHelpMessage(); return 0; }
	if (((argument_flags & ZIGB_FLAG) != 0) &&	// if zigbee flag & input flag & output flag
		((argument_flags & IN_F_FLAG) != 0) && 
		((argument_flags & OU_F_FLAG) != 0)){
			FILE *in_file = open_file(in_filename_ptr, "r", argv[0]);
			if (in_file != NULL){
				// Initial line offsets 100k
				line_offsets = (uint64_t*)calloc(LINE_INIT+1, sizeof(uint64_t));
				*line_offsets = 0; // first starts at offset zero
				line_offset_index++;
				uint64_t lines = get_number_of_lines(in_file, line_offsets, line_offset_index);
				fseek(in_file, 0, SEEK_SET);
				//char *test = extract_n_lines(in_file, line_p_session, 0);
				//process_zigbee_file_input(in_file, lines);
				//process_zigbee_file_input_live(LIVE_FLAG, line_buffer, out_filename_ptr, argument_flags, argv, Enumerator_Addr);
				printf("Was able to open file!\nAllocs: %lu Freed: %lu\n", mlk_alloc, mlk_free);
				fclose(in_file);
			}

	} else if (
	((argument_flags & LIVE_FLAG) != 0) &&	// if live flag & (output or stdout) & not(zigbee or wifi)
				(((argument_flags & OU_F_FLAG) != 0)
				 || ((argument_flags & STDOUT_FL) != 0)) &&
				(((argument_flags & ZIGB_FLAG) == 0) || ((argument_flags & WIFI_FLAG) == 0)) || ((argument_flags & SPECT_FLA) == SPECT_FLA)
				){

				char *line_buffer = (char*)calloc(1024, sizeof(char));
				unsigned int line_count = 0;
				if ((argument_flags & STDOUT_FL) != STDOUT_FL){
					FILE *out_file = open_file(out_filename_ptr, "w+", path);
					fwrite(&Version, sizeof(int), 1, out_file);
					fwrite(&line_count, sizeof(int), 1, out_file);
					fclose(out_file);
				}
				sem_init(&semaphore, 0, 1);
//				printf("Stage -1: Synchro\n");
//				char *test_local = "localhost\0";
				char test_local[] = "localhost\0";
				create_tcp_client_connection(cli_sock, test_local, 31337);
				PDU pdu_buffer;
				read_data_from_socket(cli_sock, (char*)&pdu_buffer, sizeof(pdu_buffer));
				if (pdu_buffer.command == 16) synchro_seconds = pdu_buffer.timestamp;

				while((fgets(line_buffer, 1024, stdin) != NULL)){
					if (((argument_flags & ZIGB_FLAG) == ZIGB_FLAG)){
						//printf("\nEn - %s\n", line_buffer);
						test_main(3);
//						zbee_struct_internal *test_zbee = (zbee_struct_internal*)calloc(1, sizeof(zbee_struct_internal));
//						pro_zbee_int(line_buffer, test_zbee, Enumerator_Addr);
//***						analyse_slot_add(slot, (void*)test_zbee, sizeof(zbee_struct_internal), 3, Enumerator_Addr, &process_flag, window_seconds);
//						if (process_flag == 2){ process_flag = 0; }
						//process_zigbee_file_input_live(LIVE_FLAG, line_buffer, out_filename_ptr, argument_flags, argv[0], Enumerator_Addr);

					} else if (((argument_flags & WIFI_FLAG) == WIFI_FLAG)){
						test_main(1);
						//process_wifi_file_input_live(LIVE_FLAG, line_buffer, out_filename_ptr, argument_flags, argv[0], Enumerator_Addr, Enumerator_Proto);
//							wifi_struct_internal *test_wifi = (wifi_struct_internal*)calloc(1, sizeof(wifi_struct_internal));
//							pro_wifi_int(line_buffer, test_wifi, Enumerator_Addr);
//***							analyse_slot_add(slot, (void*)test_wifi, sizeof(wifi_struct_internal), 1, Enumerator_Addr, &process_flag, window_seconds);
//							if (process_flag == 2){ process_flag = 0; }
					} else if (((argument_flags & IP_SHOR_F) == IP_SHOR_F)){
						test_main(2); // 2 - IP type
//						ip_struct_internal *test_ip = (ip_struct_internal*)calloc(1,sizeof(ip_struct_internal));
//						pro_short_int(line_buffer, test_ip, Enumerator_Addr);
//						analyse_slot_add(slot, (void*)test_ip, sizeof(ip_struct_internal), 2, Enumerator_Addr, &process_flag, window_seconds);
//						if (process_flag == 2){ process_flag = 0;  }
						//process_ip_short_input_live(LIVE_FLAG, line_buffer, out_filename_ptr, argument_flags, argv[0], Enumerator_Addr, Enumerator_Proto);
					} else if (((argument_flags & AUDIO_FLA) == AUDIO_FLA)){
						test_main(5);
//						printf("\n%d", sizeof(Audio_Frame));
//						audio_struct_internal *test_aud = (audio_struct_internal*)calloc(1,sizeof(audio_struct_internal));
//						pro_audio_int(line_buffer, test_aud);
//						analyse_slot_add(slot, (void*)test_aud, sizeof(audio_struct_internal), 5, Enumerator_Addr, &process_flag, window_seconds);
//						if (process_flag == 2){ process_flag = 0; }
						//process_audio_input_live(LIVE_FLAG, line_buffer, argv[0], argument_flags, out_filename_ptr);
						//if (process_flag == 2){ process_flag = 0; alarm(window_seconds); }
					} else if (((argument_flags & SPECT_FLA) == SPECT_FLA)){
						if (synchronized == 1){
							spec_struct_internal *test_spec = (spec_struct_internal*)calloc(1,sizeof(spec_struct_internal));
							process_rf_output(line_buffer, test_spec);
							if (test_spec->n != 0){
								FILE *_file;
								clock_t time;
								unsigned char proc = 0;
								if (test_spec->n >= 47*window_seconds) proc = 1;
								if (proc == 1){ _file = fopen(cpu_time_file, "a"); time = clock(); }
								 analyse_slot_add(slot, (void*)test_spec, sizeof(spec_struct_internal), 4, Enumerator_Addr, &process_flag, window_seconds, cli_sock);
								if (proc == 1){ time = clock() - time; fprintf(_file, "%.6f\n", ((double)time)/CLOCKS_PER_SEC); fclose(_file); proc =0; }
							}
						}
						// process_spectrum_input
					}
					line_count++;
				}

	} else if (((argument_flags & WIFI_FLAG) != 0) &&     // if wifi flag & input & output
                	((argument_flags & IN_F_FLAG) != 0) &&
                	((argument_flags & OU_F_FLAG) != 0)){

			FILE *in_file = open_file(in_filename_ptr, "r", path);
                        if (in_file != NULL){
                                // Initial line offsets 100k
                                line_offsets = (uint64_t*)calloc(LINE_INIT+1, sizeof(uint64_t));
                                *line_offsets = 0; // first starts at offset zero
                                line_offset_index++;
                                uint64_t lines = get_number_of_lines(in_file, line_offsets, line_offset_index);
                                fseek(in_file, 0, SEEK_SET);
                                //char *test = extract_n_lines(in_file, line_p_session, 0);
//                                process_wifi_file_input(in_file, lines);
                                printf("Was able to open file!\nAllocs: %lu Freed: %lu\n", mlk_alloc, mlk_free);
                                fclose(in_file);
                        }
	} else {
		showHelpMessage();
	}
//	cin.get();
	return 0;
}

