// Generic functions header file 
#ifndef _ANALYSIS_GENERIC_
#include <stdlib.h>

typedef struct SOURCE_ENUM{
        unsigned short *array;
        unsigned short n;
}SOURCE_ENUM;

typedef struct GLOBAL_KNOWLEDGE{
	SOURCE_ENUM	*Global_Sources;
	SOURCE_ENUM	*Global_Destinations;
	SOURCE_ENUM	*Global_Frames;	// discontinued, remove later
	SOURCE_ENUM	*Global_Types;
	SOURCE_ENUM	*Global_SubTypes;
	SOURCE_ENUM 	*Global_ExtTypes;
}GLOBAL_KNOWLEDGE;

GLOBAL_KNOWLEDGE *global_knowledge_init();
void global_knowledge_free(GLOBAL_KNOWLEDGE *_glob);

#define _ANALYSIS_GENERIC_
#endif
