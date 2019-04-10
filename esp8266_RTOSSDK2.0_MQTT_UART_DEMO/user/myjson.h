#ifndef __myjson__
#define __myjson__

#include "cJSON.h"

struct	version_struct_t
{
	char cmd[30];
	char version[30];
	char url[50];
};

struct	update_start_struct_t
{
	char cmd[30];
	char version[30];
	char url[50];
};



#endif

