/**
  **********************************************************************************
  * @file      ebyte_callback.h
  * @brief     EBYTE��������շ���ɻص����� �ɿͻ�ʵ���Լ����߼����� 
  * @details   ������μ� https://www.ebyte.com/       
  * @author    JiangHeng     
  * @date      2021-05-26     
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
/*= !!!����Ŀ��Ӳ������       =======================================*/
extern void UserTransmitDoneCallback(void);
extern void UserReceiveDoneCallback( uint8_t *buffer, uint8_t length);
/*==================================================================*/

/*!
 * @brief ������ɻص��ӿ� �ɿͻ�ʵ���Լ��ķ�������߼�
 * 
 * @param state �ϲ�ص��ṩ��״̬�� �ͻ������ʾ��ע���ҵ���Ӧ����
 *  
 */
void Ebyte_Port_TransmitCallback( uint16e_t state )
{       
    /* ����: ������� */
    if( state &= 0x0001 )
    {
        //To-do ʵ���Լ����߼� 
        UserTransmitDoneCallback();
    }
    /* ����: ������� */
    else 
    {
        //To-do ʵ���Լ����߼� 
    }
}

/*!
 * @brief ������ɻص��ӿ� �ɿͻ�ʵ���Լ��ķ�������߼�
 * 
 * @param state �ϲ�ص��ṩ��״̬�� �ͻ������ʾ��ע���ҵ���Ӧ����
 */
void Ebyte_Port_ReceiveCallback(  uint16e_t state, uint8e_t *buffer, uint8e_t length )
{
    /* ����: ���� */
    if( state &= 0x0001 )
    {
        //To-do ʵ���Լ����߼�     
        UserReceiveDoneCallback( buffer , length);//ʾ��

    }
    /* ����: ������� */
    else 
    {
        //To-do ʵ���Լ����߼� 
    }

}