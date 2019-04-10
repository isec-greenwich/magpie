#include "argread.h"

unsigned short extract_flag(char *arg, unsigned short argument_flags, char *in_filename_ptr, char *out_filename_ptr, unsigned char *window_val, unsigned long *epoch){
        unsigned short args = argument_flags;
        if(strcmp(arg, "-l") == 0){
                args |= LIVE_FLAG;
        } else if (strcmp(arg, "-i") == 0){
                args |= IN_F_FLAG;
                args |= PARAMETERF;
//                in_filename_ptr = 0;
        } else if (strcmp(arg, "-o") == 0){
                args |= OU_F_FLAG;
                args |= PARAMETERF;
//                out_filename_ptr = 0;
        } else if (strcmp(arg, "-w") == 0){
                args |= WIFI_FLAG;
        } else if (strcmp(arg, "-z") == 0){
                args |= ZIGB_FLAG;
        } else if (strcmp(arg, "-t_cpu") == 0){
		args |= CPU_TIMES;
	} else if (strcmp(arg, "-s") == 0){
                args |= IP_SHOR_F;
        } else if (strcmp(arg, "-a") == 0){
                args |= AUDIO_FLA;
        } else if (strcmp(arg, "-t") == 0){
                args |= STDOUT_FL;
        } else if (strcmp(arg, "-p") == 0){
		args |= SPECT_FLA;
	} else if (strcmp(arg, "-d") == 0){
		args |= WINDO_FLA;
		args |= WINDOW_VA;
	} else if (strcmp(arg, "-e") == 0){
		args |= SYNCHRONI;
		args |= PARAMETERF;
	}else {
                if ((args & PARAMETERF) != 0){
                        if (((args & IN_F_FLAG) != 0)){
                                char *t = arg;
                                unsigned char i = 0;
                                while(*(t++) != 0)i++; // get length of the string
//                                in_filename_ptr = (char*)calloc(i+1, sizeof(char)); // clear the allocated memory
                                memcpy(in_filename_ptr, arg, i);
                                args ^= PARAMETERF; args ^= IN_F_FLAG; return args;
                        }
                        if (((args & OU_F_FLAG) != 0)){
                                char *t = arg;
                                unsigned char i = 0;
                                while(*(t++) != 0)i++; // get length of the string
//                                out_filename_ptr = (char*)calloc(i+1, sizeof(char)); // clear the allocated memory
                                memcpy(out_filename_ptr, arg, i);
                                args ^= PARAMETERF; args ^= OU_F_FLAG; return args;
                        }
			if (((args & SYNCHRONI) != 0)){
				unsigned long _epoch = atol(arg);
				memcpy((void*)epoch, (void*)&_epoch, sizeof(long));
				args ^= PARAMETERF; args ^= SYNCHRONI; return args;
			}
                } else if ((args & WINDOW_VA) != 0) {
			*window_val = atoi(arg);
			args ^= WINDOW_VA; args ^=WINDO_FLA; return args;
		}
        }
        return args;
}


unsigned short argument_flagger(int argc, char** argv, unsigned short argument_flags, char *in_file, char *out_file, unsigned char *window_val, unsigned long *epoch){
        unsigned char i = 1;
	unsigned short local_args = argument_flags;
        if (argc > 1){
                while(--argc){
                        local_args = extract_flag(argv[i], local_args, in_file, out_file, window_val, epoch);
                        i++;
                        if (local_args & HELP_FLAG) break; // if help is set then break the loop and show help
			argument_flags |= local_args;
                }
        }
        return argument_flags;
}

