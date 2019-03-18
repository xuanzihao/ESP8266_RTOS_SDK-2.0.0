/**
************************************************************
* @file         hal_key.c
* @brief        ��������
* 
* ����ģ����ö�ʱ��+GPIO״̬��ȡ���ƣ�GPIO������Ҫ����ESP8266������ֲ�������
* 
* ������֧�� 0 ~ 12 ��GPIO������չ��֧�ֿ�ƽ̨��ֲ��
* @author       Gizwits
* @date         2016-09-05
* @version      V03010201
* @copyright    Gizwits
* 
* @note         ������.ֻΪ����Ӳ������
*               Gizwits Smart Cloud  for Smart Products
*               ����|��ֵ|����|����|��ȫ|����|����|��̬
*               www.gizwits.com
*
***********************************************************/
#include "hal_key.h"
uint32 keyCountTime = 0; 
static uint8_t keyTotolNum = 0;

/**
* @brief Read the GPIO state
* @param [in] keys ��������ȫ�ֽṹ��ָ��
* @return uint16_t�͵�GPIO״ֵ̬
*/
static ICACHE_FLASH_ATTR uint16_t keyValueRead(keys_typedef_t * keys)
{
    uint8_t i = 0; 
    uint16_t read_key = 0;

    //GPIO Cyclic scan
    for(i = 0; i < keys->keyTotolNum; i++)
    {
        if(!GPIO_INPUT_GET(keys->singleKey[i]->gpio_id))
        {
            G_SET_BIT(read_key, keys->singleKey[i]->gpio_number);
        }
    }
    
    return read_key;
}

/**
* @brief Read the KEY value
* @param [in] keys ��������ȫ�ֽṹ��ָ��
* @return uint16_t�͵İ���״ֵ̬
*/
static uint16_t ICACHE_FLASH_ATTR keyStateRead(keys_typedef_t * keys)
{
    static uint8_t Key_Check = 0;
    static uint8_t Key_State = 0;
    static uint16_t Key_LongCheck = 0;
    uint16_t Key_press = 0; 
    uint16_t Key_return = 0;
    static uint16_t Key_Prev = 0;
    
    //�ۼӰ���ʱ��
    keyCountTime++;
        
    //��������30MS
    if(keyCountTime >= (DEBOUNCE_TIME / keys->key_timer_ms)) 
    {
        keyCountTime = 0; 
        Key_Check = 1;
    }
    
    if(Key_Check == 1)
    {
        Key_Check = 0;
        
        //��ȡ��ǰ��������ֵ
        Key_press = keyValueRead(keys); 
        
        switch (Key_State)
        {
            //"�״β�׽����"״̬
            case 0:
                if(Key_press != 0)
                {
                    Key_Prev = Key_press;
                    Key_State = 1;
                }
    
                break;
                
                //"��׽����Ч����"״̬
            case 1:
                if(Key_press == Key_Prev)
                {
                    Key_State = 2;
                    Key_return= Key_Prev | KEY_DOWN;
                }
                else
                {
                    //����̧��,�Ƕ���,����Ӧ����
                    Key_State = 0;
                }
                break;
                
                //"��׽������"״̬
            case 2:
    
                if(Key_press != Key_Prev)
                {
                    Key_State = 0;
                    Key_LongCheck = 0;
                    Key_return = Key_Prev | KEY_UP;
                    return Key_return;
                }
    
                if(Key_press == Key_Prev)
                {
                    Key_LongCheck++;
			//os_printf("\r\n...........\nkey_press = %x\n\n\n",Key_press);
			if((Key_press == 1)&&(Key_LongCheck/(PRESS_LONG_TIME / (DEBOUNCE_TIME*7)) >= 3)&&(Key_LongCheck%(PRESS_LONG_TIME / (DEBOUNCE_TIME*7)) == 0)){
			//	Play_Voice(VN_DI_SHORT);
			}
	                if(Key_LongCheck >= (PRESS_LONG_TIME / DEBOUNCE_TIME))    //����3S (����30MS * 100)
	                {
	                    Key_LongCheck = 0;
	                    Key_State = 3;
	                    Key_return= Key_press |  KEY_LONG;
	                    return Key_return;
	                }
                }
                break;
                
                //"��ԭ��ʼ"״̬    
            case 3:
                if(Key_press != Key_Prev)
                {
                    Key_State = 0;
                }
                break;
        }
    }

    return  NO_KEY;
}

/**
* @brief �����ص�����

* �ڸú�������ɰ���״̬������ö�Ӧ�Ļص�����
* @param [in] keys ��������ȫ�ֽṹ��ָ��
* @return none
*/
void ICACHE_FLASH_ATTR gokitKeyHandle(keys_typedef_t * keys)
{
    uint8_t i = 0;
    uint16_t key_value = 0;

    key_value = keyStateRead(keys); 
    
    if(!key_value) return;
    
    //Check short press button
    if(key_value & KEY_UP)
    {
        //Valid key is detected
        for(i = 0; i < keys->keyTotolNum; i++)
        {
            if(G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number)) 
            {
                //key callback function of short press
                if(keys->singleKey[i]->short_press) 
                {
                    keys->singleKey[i]->short_press(); 
					
                    //Stop_Voice();		//ֹͣ��������
                    os_printf("[zs] callback key: [%d][%d] \r\n", keys->singleKey[i]->gpio_id, keys->singleKey[i]->gpio_number); 
                }
            }
        }
    }

    //Check short long button
    if(key_value & KEY_LONG)
    {
        //Valid key is detected
        for(i = 0; i < keys->keyTotolNum; i++)
        {
            if(G_IS_BIT_SET(key_value, keys->singleKey[i]->gpio_number))
            {
                //key callback function of long press
                if(keys->singleKey[i]->long_press) 
                {
                    keys->singleKey[i]->long_press(); 
                    
                    os_printf("[zs] callback long key: [%d][%d] \r\n", keys->singleKey[i]->gpio_id, keys->singleKey[i]->gpio_number); 
                }
            }
        }
    }
}

/**
* @brief ��������ʼ��

* �ڸú�������ɵ��������ĳ�ʼ����������Ҫ���ESP8266 GPIO�Ĵ���˵���ĵ������ò���
* @param [in] gpio_id ESP8266 GPIO ���
* @param [in] gpio_name ESP8266 GPIO ����
* @param [in] gpio_func ESP8266 GPIO ����
* @param [in] long_press ����״̬�Ļص�������ַ
* @param [in] short_press �̰�״̬�Ļص�������ַ
* @return �������ṹ��ָ��
*/
key_typedef_t * ICACHE_FLASH_ATTR keyInitOne(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, gokit_key_function long_press, gokit_key_function short_press)
{
    static int8_t key_total = -1;

    key_typedef_t * singleKey = (key_typedef_t *)os_zalloc(sizeof(key_typedef_t));

    singleKey->gpio_number = ++key_total;
    
    //Platform-defined GPIO
    singleKey->gpio_id = gpio_id;
    singleKey->gpio_name = gpio_name;
    singleKey->gpio_func = gpio_func;
    
    //Button trigger callback type
    singleKey->long_press = long_press;
    singleKey->short_press = short_press;
    
    keyTotolNum++;    

    return singleKey;
}

/**
* @brief ����������ʼ��

* �ڸú�����������еİ���GPIO��ʼ����������һ����ʱ����ʼ����״̬���
* @param [in] keys ��������ȫ�ֽṹ��ָ��
* @return none
*/
void ICACHE_FLASH_ATTR keyParaInit(keys_typedef_t * keys)
{
    uint8 tem_i = 0; 
    
    if(NULL == keys)
    {
        return ;
    }
    
    //init key timer 
    keys->key_timer_ms = KEY_TIMER_MS; 
    os_timer_disarm(&keys->key_timer); 
    os_timer_setfn(&keys->key_timer, (os_timer_func_t *)gokitKeyHandle, keys); 
    
    keys->keyTotolNum = keyTotolNum;

    //Limit on the number keys (Allowable number: 0~12)
    if(KEY_MAX_NUMBER < keys->keyTotolNum) 
    {
        keys->keyTotolNum = KEY_MAX_NUMBER; 
    }
    
    //GPIO configured as a high level input mode
    for(tem_i = 0; tem_i < keys->keyTotolNum; tem_i++) 
    {
        PIN_FUNC_SELECT(keys->singleKey[tem_i]->gpio_name, keys->singleKey[tem_i]->gpio_func); 
        GPIO_OUTPUT_SET(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id), 1); 
        PIN_PULLUP_EN(keys->singleKey[tem_i]->gpio_name); 
        GPIO_DIS_OUTPUT(GPIO_ID_PIN(keys->singleKey[tem_i]->gpio_id)); 
        
        os_printf("gpio_name %d \r\n", keys->singleKey[tem_i]->gpio_id); 
    }
    
    //key timer start
    os_timer_arm(&keys->key_timer, keys->key_timer_ms, 1); 
}

/**
* @brief ������������

* �ú���ģ�����ⲿ�԰���ģ��ĳ�ʼ������

* ע���û���Ҫ������Ӧ�İ����ص�����(�� key1LongPress ...)
* @param none
* @return none
*/
void ICACHE_FLASH_ATTR keyTest(void)
{
#ifdef KEY_TEST
    //����GPIO�����궨��
    #define GPIO_KEY_NUM                            2                           ///< ���尴����Ա����
    #define KEY_0_IO_MUX                            PERIPHS_IO_MUX_GPIO0_U      ///< ESP8266 GPIO ����
    #define KEY_0_IO_NUM                            0                           ///< ESP8266 GPIO ���
    #define KEY_0_IO_FUNC                           FUNC_GPIO0                  ///< ESP8266 GPIO ����
    #define KEY_1_IO_MUX                            PERIPHS_IO_MUX_MTMS_U       ///< ESP8266 GPIO ����
    #define KEY_1_IO_NUM                            14                          ///< ESP8266 GPIO ���
    #define KEY_1_IO_FUNC                           FUNC_GPIO14                 ///< ESP8266 GPIO ����
    LOCAL key_typedef_t * singleKey[GPIO_KEY_NUM];                              ///< ���嵥��������Ա����ָ��
    LOCAL keys_typedef_t keys;                                                  ///< �����ܵİ���ģ��ṹ��ָ��    
    
    //ÿ��ʼ��һ����������һ��keyInitOne ,singleKey�����һ
    singleKey[0] = keyInitOne(KEY_0_IO_NUM, KEY_0_IO_MUX, KEY_0_IO_FUNC,
                                key1LongPress, key1ShortPress);
                                
    singleKey[1] = keyInitOne(KEY_1_IO_NUM, KEY_1_IO_MUX, KEY_1_IO_FUNC,
                                key2LongPress, key2ShortPress);
                                
    keys.key_timer_ms = KEY_TIMER_MS; //���ð�����ʱ������ ����10ms
    keys.singleKey = singleKey; //��ɰ�����Ա��ֵ
    keyParaInit(&keys); //����������ʼ��
#endif
}