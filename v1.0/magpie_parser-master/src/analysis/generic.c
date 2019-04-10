// Generic function implementation

#include "generic.h"

GLOBAL_KNOWLEDGE *global_knowledge_init(){
	GLOBAL_KNOWLEDGE *_glob = (GLOBAL_KNOWLEDGE*)calloc(1, sizeof(GLOBAL_KNOWLEDGE));	// create structure with all 0's
	_glob->Global_Sources = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	_glob->Global_Destinations = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	_glob->Global_Frames = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	_glob->Global_Types = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	_glob->Global_SubTypes = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	_glob->Global_ExtTypes = (SOURCE_ENUM*)calloc(1, sizeof(SOURCE_ENUM));
	return _glob;
}

void global_knowledge_free(GLOBAL_KNOWLEDGE *_glob){
	free(_glob->Global_Sources);
	free(_glob->Global_Destinations);
	free(_glob->Global_Frames);
	free(_glob->Global_Types);
	free(_glob->Global_SubTypes);
	free(_glob->Global_ExtTypes);
	free(_glob);
}
