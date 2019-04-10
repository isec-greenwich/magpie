#include "enumerator.h"

void enum_init(Enum_Type *Enum){
        Enum->frame_type = 0x0000;
        Enum->name = 0x00;
        Enum->next = 0x00;
}

unsigned short enum_add(char *name, Enum_Type *Enum){
        unsigned int i = 0;
        unsigned short frame_type_id = 0;
        if(Enum->frame_type == 0){
                // first
                char *ptr = name;
                while(*ptr++ != '\0')i++;
                //ptr += name_length;
                Enum->name = (char*)calloc(i+1,sizeof(char));
                memcpy(Enum->name, name, sizeof(char)*i);
                Enum->frame_type = 1;
                frame_type_id = Enum->frame_type;
                Enum->next = (Enum_Type*)calloc(i,sizeof(Enum_Type));
        } else {
                // crawl to the top
                unsigned int free_frame_type = Enum->frame_type;
                Enum_Type *pkt_ptr = Enum->next;
                while(pkt_ptr->name != 0x00) {free_frame_type++; pkt_ptr = pkt_ptr->next;}
                char *ptr = name;
                while(*ptr++ != '\0')i++;
                //ptr += name_length;
                pkt_ptr->name = (char*)calloc(i+1,sizeof(char));
                memcpy(pkt_ptr->name, name, sizeof(char)*i);
                pkt_ptr->frame_type = free_frame_type+1;
                frame_type_id = pkt_ptr->frame_type;
                pkt_ptr->next = (Enum_Type*)calloc(i,sizeof(Enum_Type));
        }
        return frame_type_id;
}

unsigned short enum_add_num(char *name, Enum_Type *Enum, unsigned short frame_id){
        unsigned int i = 0;
        unsigned short frame_type_id = 0;

        if(Enum->frame_type == 0){
                // first
                char *ptr = name;
                while(*ptr++ != '\0')i++;
                Enum->name = (char*)calloc(i+1,sizeof(char));
                memcpy(Enum->name, name, sizeof(char)*i);
                Enum->frame_type = frame_id;
                Enum->next = (Enum_Type*)calloc(i, sizeof(Enum_Type));
        } else {
                // crawl to the top
                Enum_Type *pkt_ptr = Enum->next;
                while(pkt_ptr->name != 0x00) {pkt_ptr = pkt_ptr->next;}
                char *ptr = name;
                while(*ptr++ != '\0')i++;
                pkt_ptr->name = (char*)calloc(i+1,sizeof(char));
                memcpy(pkt_ptr->name, name, sizeof(char)*i);
                pkt_ptr->frame_type = frame_id;
                frame_type_id = pkt_ptr->frame_type;
                pkt_ptr->next = (Enum_Type*)calloc(i,sizeof(Enum_Type));
        }
        return frame_type_id;
}
char* enum_find_frame_name(unsigned short id, Enum_Type *Enum){
        Enum_Type *pkt_ptr = Enum;
        char *res = 0x0000;
        unsigned char length = 0;
        do{
                if (pkt_ptr->name != 0x00){
                        if(pkt_ptr->frame_type == id){ res = pkt_ptr->name; break;}
                        if(pkt_ptr->name != 0x00) pkt_ptr = pkt_ptr->next;
                }
        }while(pkt_ptr->name != 0x00);
        return res;
}

unsigned short enum_find_frame_type(char *name, Enum_Type *Enum){
        Enum_Type *pkt_ptr = Enum;
        unsigned short res = 0xFFFF;
        unsigned char length = 0;
        char *ptr = name;
        while(*ptr++ != '\0') length++;
        ptr = name;
        do{
                if (pkt_ptr->name != 0x00){
                        if(strcmp(name, pkt_ptr->name) == 0){ res = pkt_ptr->frame_type; break;}
                        if(pkt_ptr->name != 0x00) pkt_ptr = pkt_ptr->next;
                }
        }while(pkt_ptr->name != 0x00);
        return res;
}

unsigned short manage_enumerations(Enum_Type *Enum_Start, char *start_pointer, char* end_pointer){
        char *man_type = (char*)calloc((end_pointer-start_pointer)+1, sizeof(char));
        memcpy(man_type, start_pointer, sizeof(char)*(end_pointer-start_pointer));
        unsigned short ft = enum_find_frame_type(man_type, Enum_Start);
        if(ft == 0xFFFF){
                ft = enum_add(man_type, Enum_Start);
        }
        free(man_type);
        return ft;
}

unsigned char manage_comparison(char *start_pointer, char terminating_char, const char *value){
        char *t_ptr = start_pointer;
        unsigned char res = 0;
        while(*t_ptr++ != terminating_char){res++; if (*t_ptr == ','){start_pointer=t_ptr; res=0;}}
        char *man_type = (char*)calloc((t_ptr-start_pointer)+1, sizeof(char));
        memcpy(man_type, start_pointer, (t_ptr-start_pointer));
        if (strcmp(man_type, value) == 0){ free(man_type); return 1; }
        free(man_type);
        return 0;
}



