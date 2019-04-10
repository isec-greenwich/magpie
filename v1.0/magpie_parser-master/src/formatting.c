#include "formatting.h"

char *extract_path(char *arg){
        char *ptr = arg;
        unsigned int len = 0;
        unsigned char slashes = 0;
        // find slashes (windows/unix) length, if full path provided
        while(*(ptr++) != 0){
                if ((*ptr == '\\') || (*ptr == '/')){ slashes++; }
                len++;
                //printf("+");
        }
        if (slashes == 0){ ptr = (char *)calloc(1, sizeof(char)); *ptr = '\0'; return ptr; }
        // extract path only
        ptr = arg+len;
        while(*(ptr--) != '\\'){
                if (*ptr == '/'){ len--; break; } // possibly len--
                len--;
                //printf("-");
        }
        ptr = (char *)calloc(len+2, sizeof(char));
        memcpy(ptr, arg, len+1);
        return ptr;
}

char *concat_path_and_filename(char *filename, char *path){
        char *ptr = 0;
        int len_p = 0;
        int len_f = 0;
        ptr = path;
        while(*(ptr++) != 0) len_p++;
        ptr = filename;
        while(*(ptr++) != 0) len_f++;
        ptr = (char*)calloc(len_p+len_f+1, sizeof(char));
        memcpy(ptr, path, len_p);
        memcpy(ptr+len_p, filename, len_f);
        return ptr;
        // free path
}
char *extract_line(char *lines, char **updated_offset){
        char *ptr = lines;
        int i = 0;
        int p = 0;
        if (*ptr == '\n'){ p++; ptr++; } // skip new line if exists
        while(*ptr == ' '){ p++; ptr++; } //skip padding
        while(*ptr != '\n'){ i++; ptr++; }
        updated_offset[0] = ptr;
        ptr = (char*)calloc(i+1, sizeof(char));
        memcpy(ptr, lines+p, sizeof(char)*i);
        return ptr;
}

char *return_me_not_char(char *ptr, char not_char){
        if (*ptr == not_char){
                while(*ptr++ == not_char);
                return (ptr-1);
        }
        return ptr;
}

