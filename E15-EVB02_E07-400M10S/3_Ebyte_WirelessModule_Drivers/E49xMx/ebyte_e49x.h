/**
  **********************************************************************************
  * @file      board_ebyte_E22xx.h
  * @brief     E49 ϵ�� ������     
  * @details   ������μ� https://www.ebyte.com/       
  * @author    JiangHeng     
  * @date      2021-05-08     
  * @version   1.0.0     
  **********************************************************************************
  * @copyright BSD License 
  *            �ɶ��ڰ��ص��ӿƼ����޹�˾  
  *   ______   ____   __     __  _______   ______ 
  *  |  ____| |  _ \  \ \   / / |__   __| |  ____|
  *  | |__    | |_) |  \ \_/ /     | |    | |__   
  *  |  __|   |  _ <    \   /      | |    |  __|  
  *  | |____  | |_) |    | |       | |    | |____ 
  *  |______| |____/     |_|       |_|    |______| 
  *
  **********************************************************************************
  */
#include "ebyte_callback.h"

/**
 * @note ���� E49-400M20S Ƶ��
 *       �� 410.000 MHz��Ϊ��ʼƵ��
 */
#if defined(EBYTE_E49_400M20S )
   #define EBYTE_E49x_FREQUENCY_STEP 200   //ͨ������ 2.5KHz*200 = 500KHz = 0.5MHz
   #define EBYTE_E49x_FREQUENCY_CHANNEL 46 //ͨ�������ۻ�  ��ô����Ƶ��Ϊ 410.0MHz + 0.5MHz*46  = 433.0MHz
   
/**
 * @note ���� E49-900M20S Ƶ��
 *       �� 850.000 MHz��Ϊ��ʼƵ��
 */
#elif defined(EBYTE_E49_900M20S)
   #define EBYTE_E49x_FREQUENCY_STEP 200   //ͨ������ 2.5KHz*200 = 500KHz = 0.5MHz
   #define EBYTE_E49x_FREQUENCY_CHANNEL 0 //ͨ�������ۻ�  ��ô����Ƶ��Ϊ 850.0MHz + 0.5MHz*36  = 868.0MHz
#endif

/* ��μ��ģ�鷢�������  0:�첽�ж�֪ͨ  1:ͬ������ֱ��������� */
#define EBYTE_TRANSMIT_MODE_BLOCKED   0     
   
/* ���÷������ʱ �����жϵ�GPIO */
#define TRANSMIT_IRQ_GPIO  2  //1:GPIO1  2:GPIO2  3:GPIO3
   
/* ���ý������ʱ �����жϵ�GPIO */
#define RECEIVE_IRQ_GPIO   2  //1:GPIO1  2:GPIO2  3:GPIO3 
   
 
   
uint8e_t E49x_Init(void);
uint8e_t E49x_GoTransmit( uint8e_t *data, uint8e_t size );
uint8e_t E49x_GoReceive(void);    
uint8e_t E49x_GoSleep(void);
uint8e_t E49x_TaskForPoll(void);
uint8e_t E49x_GetStatus(void);
uint8e_t E49x_GetDriverVersion(void);
uint8e_t* E49x_GetName(void);
void E49x_TaskForIRQ(void);