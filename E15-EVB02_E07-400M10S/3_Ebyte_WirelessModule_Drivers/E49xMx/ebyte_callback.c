/**
  **********************************************************************************
  * @file      ebyte_callback.h
  * @brief     EBYTE��������շ���ɻص����� �ɿͻ�ʵ���Լ����߼����� 
  * @details   ������μ� https://www.ebyte.com/       
  * @author    JiangHeng     
  * @date      2021-05-13     
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

/*= !!!����Ŀ��Ӳ��ƽ̨ͷ�ļ� =======================================*/
#include "board.h"  //E15-EVB02 ������
#include "ebyte_debug.h"  //E15-EVB02 ������
/*= !!!����Ŀ��ƽ̨�ص�����      =======================================*/
extern void UserTransmitDoneCallback(void);
extern void UserReceiveDoneCallback( uint8_t *buffer, uint8_t length);
/*==================================================================*/

/* !
 * @brief ������ɻص��ӿ� �ɿͻ�ʵ���Լ��ķ�������߼�
 * 
 * @param state �ϲ�ص��ṩ��״̬�� �ͻ������ʾ��ע���ҵ���Ӧ����
 *  
 * @note E49x ģ����ṩ��״̬��
 *         IRQ_TX_DONE                             = 0x0800
 */
void Ebyte_Port_TransmitCallback( uint16e_t state )
{       
    /* ����: ������� */
    if( state &= 0x0800 )
    {
        //To-do ʵ�����Լ����߼�
        UserTransmitDoneCallback();
        
    }
    /* ����: δ֪���� */
    else
    {
        /* ����״̬��ʶ����ȷ��������Ӳ��  
           �������� 1:SPIͨ�Ų���ȷ 2:ģ�鹩�粻�� */
        while(1);
    }
}

/* !
 * @brief ������ɻص��ӿ� �ɿͻ�ʵ���Լ��ķ�������߼�
 * 
 * @param state �ϲ�ص��ṩ��״̬�� �ͻ������ʾ��ע���ҵ���Ӧ����
 * 
 * @note E49x ģ����ṩ��״̬��
 *         GET_IRQ_PKT_OK                             = 0x0001
 */
void Ebyte_Port_ReceiveCallback(  uint16e_t state, uint8e_t *buffer, uint8e_t length )
{
    /* ����: ���� */
    if( state &= 0x0001 )
    {
        //To-do ʵ�����Լ����߼�
        UserReceiveDoneCallback( buffer , length);

    }
    /* ����: δ֪���� */
    else
    {
        /* һ�㶼�ǿ����˶�����жϱ�ʶ ��Ҫ�����жϴ��� */
        while(1);
    }
}