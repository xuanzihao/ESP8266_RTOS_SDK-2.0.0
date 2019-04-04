/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
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

#ifndef __MYSEBSERVER_H__
#define __MYSEBSERVER_H__
#include "esp_common.h"

#define SERVER_PORT 80
#define SERVER_SSL_PORT 443

#define URLSize 10

typedef enum Result_Resp {
    RespFail = 0,
    RespSuc,
} Result_Resp;

typedef enum ProtocolType {
    GET = 0,
    POST,
} ProtocolType;

typedef enum _ParmType {
    SWITCH_STATUS = 0,
    INFOMATION,
    WIFI,
    SCAN,
	REBOOT,
    DEEP_SLEEP,
    LIGHT_STATUS,
    CONNECT_STATUS,
    USER_BIN
} ParmType;

typedef struct URL_Frame {
    enum ProtocolType Type;
    char pSelect[URLSize];
    char pCommand[URLSize];
    char pFilename[URLSize];
} URL_Frame;

typedef struct _rst_parm {
    ParmType parmtype;
    struct espconn *pespconn;
} rst_parm;

struct apInfo
{
	uint8_t ssid[32];
	uint8_t channel;
	char rssi;
	AUTH_MODE authmode;
	CIPHER_TYPE pairwise_cipher;
};

struct apInfoFormat
{
	uint8_t ssid[32];
	uint8_t channel;
	char rssi;
	char authmode[20];
	char pairwise_cipher[20];
};



void wifi_scand_start();



#define WEB_INDEX "<!DOCTYPE html>\
<html>\
    <head>\
        <meta charset=\"UTF-8\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" name=\"viewport\"/>\
        <title>\
            XZH的WEB配网\
        </title>\
    </head>\
    <body>\
        <div align=\"center\">\
            <font>\
                Code By XZH\
            </font>\
            <br/>\
            <font>\
                E-mail:503482366@qq.com\
            </font>\
            <br/>\
            <br/>\
            <p>\
            </p>\
            <a href=\"WebConfig.html\" text-decoration=\"none\">\
                <button formtarget=\"_self\" style=\"display:block;margin:0 auto\">\
                    开始配网\
                </button>\
            </a>\
        </div>\
    </body>\
</html>"

#define WEB_CONFIG "<!DOCTYPE html>\
<html>\
    <head>\
        <meta charset=\"UTF-8\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" name=\"viewport\"/>\
        <title>\
            XZH的WEB配网\
        </title>\
    </head>\
    <body>\
        <div align=\"center\">\
            <font>\
                Code By XZH\
            </font>\
            <br/>\
            <font>\
                E-mail: 530482366@qq.com\
            </font>\
        </div>\
        <form action=\"WiFiConfig.html\" enctype=\"application/x-www-form-urlencoded\" method=\"post\">\
            <table align=\"center\" border=\"0\" cellspacing=\"10\">\
                <tr>\
                    <td>\
                        Wi-Fi名称:\
                        <input name=\"SSID\" placeholder=\"在这里输入Wi-Fi名称\" type=\"text\"/>\
                    </td>\
                </tr>\
                <tr>\
                    <td>\
                         Wi-Fi密码:\
                        <input name=\"PASSWORD\" placeholder=\"在这里输入Wi-Fi密码\" type=\"password\"/>\
                    </td>\
                </tr>\
            </table>\
            <button style=\"display:block;margin:0 auto\" type=\"submit\" value=\"Submit\">\
                确认提交\
            </button>\
        </form>\
    </body>\
</html>\
"

#define WEB_CONFIG_HEAD "<!DOCTYPE html>\
<html>\
    <head>\
        <meta charset=\"UTF-8\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" name=\"viewport\"/>\
        <title>\
            XZH的WEB配网\
        </title>\
    </head>\
    <body>\
        <div align=\"center\">\
            <font>\
                Code By XZH\
            </font>\
            <br/>\
            <font>\
                E-mail: 530482366@qq.com\
            </font>\
        </div>\
        <form action=\"WiFiConfig.html\" enctype=\"application/x-www-form-urlencoded\" method=\"post\">\
            <table align=\"center\" border=\"0\" cellspacing=\"10\">\
                <tr>\
                    <td>\
                        Wi-Fi名称:\
                        <input name=\"SSID\" placeholder=\"在这里输入Wi-Fi名称\" type=\"text\"/>\
                    </td>\
                </tr>\
                <tr>\
                    <td>\
                         Wi-Fi密码:\
                        <input name=\"PASSWORD\" placeholder=\"在这里输入Wi-Fi密码\" type=\"password\"/>\
                    </td>\
                </tr>\
            </table>\
            <button style=\"display:block;margin:0 auto\" type=\"submit\" value=\"Submit\">\
                确认提交\
            </button>\
        </form>\
"

#define WEB_CONFIG_TAIL "\
    </body>\
</html>\
"



#define WEB_WIFI_CONFIG "<!DOCTYPE html>\
<html>\
    <head>\
        <meta charset=\"UTF-8\" content=\"width=device-width,initial-scale=1.0, minimum-scale=1.0, maximum-scale=1.0, user-scalable=no\" name=\"viewport\"/>\
        <title>\
            XZH的WEB配网\
        </title>\
    </head>\
    <body>\
        <div align=\"center\">\
            <font>\
                Code By XZH\
            </font>\
            <br/>\
            <font>\
                E-mail: 530482366@qq.com\
            </font>\
            <br/>\
            <br/>\
            <font>\
                 正在连接Wi-Fi，LED灯闪烁三次后连接完成！\
                <br/>\
                LED熄灭表示连接失败请重新输入！\
                <br/>\
                <a href=\"WebConfig.html\" text-decoration=\"none\">\
                    <button formtarget=\"_self\" style=\"display:block;margin:0 auto\">\
                        重新配网\
                    </button>\
                </a>\
                <br/>\
                <a href=\"/\">\
                    返回首页\
                </a>\
            </font>\
        </div>\
    </body>\
</html>\
"


void softAP_init(void);
void web_server_start();



#endif

