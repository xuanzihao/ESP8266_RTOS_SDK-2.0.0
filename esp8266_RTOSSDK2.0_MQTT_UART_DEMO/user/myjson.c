#include "myjson.h"

struct version_struct_t  ICACHE_FLASH_ATTR version_scr_json_parase(char * pMsg)
{
	version_struct_t vsersion_struct;
	memset(vsersion_struct,0,sizeof(vsersion_struct));
	if(NULL == pMsg)
	{
		os_printf("\r\npMsg is NULL!!!\r\n");
		return vsersion_struct;
	}

	
	cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson)                                                                                         
	{
		os_printf("\r\n parse faild !!!\r\n");
		return vsersion_struct;
	}

	cJSON *  pSub = cJSON_GetObjectItem(pJson, "cmd");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget cmd false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("cmd :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.cmd,pSub->valuestring);
	
	pSub = cJSON_GetObjectItem(pJson, "version");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget version false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("version :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.version,pSub->valuestring);

	pSub = cJSON_GetObjectItem(pJson, "url");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget url false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("url :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.url,pSub->valuestring);	
	return vsersion_struct;

}

char*  ICACHE_FLASH_ATTR version_sca_json(char * data,char* status,char* version)
{
	cJSON * pJsonRoot = cJSON_CreateObject();
	if(NULL == pJsonRoot)
	{
		//error happend here
		return NULL;
	}
	cJSON_AddStringToObject(pJsonRoot, "data", data);
	cJSON_AddStringToObject(pJsonRoot, "status", status);
	cJSON_AddStringToObject(pJsonRoot, "version", version);
	
	char * p = cJSON_Print(pJsonRoot);
	if(NULL == p)
	{
		//convert json list to string faild, exit
		//because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
		cJSON_Delete(pJsonRoot);
		return NULL;
	}
	cJSON_Delete(pJsonRoot);
	return p;
}

struct version_struct_t  ICACHE_FLASH_ATTR version_scr_json_parase(char * pMsg)
{
	version_struct_t vsersion_struct;
	memset(vsersion_struct,0,sizeof(vsersion_struct));
	if(NULL == pMsg)
	{
		os_printf("\r\npMsg is NULL!!!\r\n");
		return vsersion_struct;
	}

	
	cJSON * pJson = cJSON_Parse(pMsg);
	if(NULL == pJson)                                                                                         
	{
		os_printf("\r\n parse faild !!!\r\n");
		return vsersion_struct;
	}

	cJSON *  pSub = cJSON_GetObjectItem(pJson, "cmd");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget cmd false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("cmd :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.cmd,pSub->valuestring);
	
	pSub = cJSON_GetObjectItem(pJson, "version");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget version false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("version :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.version,pSub->valuestring);

	pSub = cJSON_GetObjectItem(pJson, "url");
	if(NULL==pSub)
	{
		cJSON_Delete(pJson);
		os_printf("\r\nget url false!!!\r\n");	
		return vsersion_struct;
	}
	os_printf("url :%s\r\n",pSub->valuestring);
	memcpy(vsersion_struct.url,pSub->valuestring);	
	return vsersion_struct;

}

char*  ICACHE_FLASH_ATTR version_sca_json(char * data,char* status,char* version)
{
	cJSON * pJsonRoot = cJSON_CreateObject();
	if(NULL == pJsonRoot)
	{
		//error happend here
		return NULL;
	}
	cJSON_AddStringToObject(pJsonRoot, "data", data);
	cJSON_AddStringToObject(pJsonRoot, "status", status);
	cJSON_AddStringToObject(pJsonRoot, "version", version);
	
	char * p = cJSON_Print(pJsonRoot);
	if(NULL == p)
	{
		//convert json list to string faild, exit
		//because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
		cJSON_Delete(pJsonRoot);
		return NULL;
	}
	cJSON_Delete(pJsonRoot);
	return p;
}