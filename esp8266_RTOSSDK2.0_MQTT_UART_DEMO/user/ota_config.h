/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2017 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef ___OTA_CONFIG_H__
#define ___OTA_CONFIG_H__


#define OTA_TIMEOUT 120000//60000


//#define OTA_SERVER_ADDR "112.74.162.225"//慧享家测试环境

//#define Testing_environment		//定义为测试环境
#ifdef  Testing_environment
	#define OTA_SERVER_ADDR		"112.74.162.225"//测试环境IP地址
#else
	#define OTA_SERVER_ADDR		"112.74.141.193"//"iot.wisbetter.com"，生产环境地址
#endif

#define OTA_SERVER_PORT 9393

#define BIN_FILENAME "user.bin"


//#define MASTER_KEY "eaff337b3468470a12c0feea6604fa349853764c"
#define MASTER_KEY "NULL"
#define Download_request_url "GET /v1/device/rom/?action=download_rom&version=%s&filename=%s HTTP/1.0\r\n\
Host:%s:%d\r\n\
%s\r\n"

#define Query_version_url "GET /v1/device/rom/?is_format_simple=true HTTP/1.0\r\n\
Host:%s:%d\r\n\
%s\r\n"

#define pheadbuffer "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/60.0.3095.5 Safari/537.36\r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Charset: iso-8859-5\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n"


#define OTA_HEADER_BUF "Connection: keep-alive\r\n\
Cache-Control: no-cache\r\n\
User-Agent: Mozilla/5.0 (Windows NT 5.1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/30.0.1599.101 Safari/537.36 \r\n\
Accept: */*\r\n\
Authorization: token %s\r\n\
Accept-Encoding: gzip,deflate,sdch\r\n\
Accept-Language: zh-CN,zh;q=0.8\r\n\r\n"
#endif
