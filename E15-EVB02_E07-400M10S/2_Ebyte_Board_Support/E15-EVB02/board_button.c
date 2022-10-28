/**
  **********************************************************************************
  * @file      board_button.c
  * @brief     ͨ�� ��������������         
  * @author    JiangHeng     
  * @date      2021-05-06     
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

#include "board.h"

/* !
 * @brief �������г�ʼ��
 * 
 * @param fifo ָ�򰴼��¼����нṹ���ָ��
 */
void Ebyte_BTN_FIFO_Init(BSP_BTN_FIFO_t *fifo)
{
    fifo->fifoLen   = 0;
    fifo->fifoRead  = 0;
    fifo->fifoWrite = 0;
}

/* !
 * @brief ����������� 
 * 
 * @param  fifo  ָ�򰴼��¼����нṹ���ָ��
 * @param  event �������¼�  ��ҪΪ�������Ķ̰�/����
 * @return 0:����  1:�������
 * @note   ���λ�ÿ�ѭ�������Ḳ����������ݣ�����ʱ�ᵼ�����ݶ�ʧ��
 */
uint8_t Ebyte_BTN_FIFO_Push(BSP_BTN_FIFO_t *fifo, BSP_BTN_EVENT_t event)
{
    /* ������ݰ�����Ԥ������ */
    fifo->fifoLen++;

    /* �����ӳ��ȴ������趨���� */
    if(fifo->fifoLen > BSP_BTN_FIFO_LENGTH)
    {
        fifo->fifoLen = BSP_BTN_FIFO_LENGTH;//��ӳ��Ȳ�������
        return 1; 
    }
    
    /* ������� */
    fifo->buff[fifo->fifoWrite] = event;
    
    /* ������λ���Ѿ����˶�β */
    if(++fifo->fifoWrite >= BSP_BTN_FIFO_LENGTH)
    {
        fifo->fifoWrite = 0; //��ô��һ��������ݽ��ص����׿�ʼ���
    }

    return 0;

}

/* !
 * @brief �������г���
 * 
 * @param fifo  ָ�򰴼��¼����нṹ���ָ��
 * @param event �������¼�  ��ҪΪ�������Ķ̰�/����
 * @return 0:����  1:����Ϊ��
 */
uint8_t Ebyte_BTN_FIFO_Pop(BSP_BTN_FIFO_t *fifo, BSP_BTN_EVENT_t *event)
{
    /* �����ӳ���Ϊ0 ���ն��� */
    if(fifo->fifoLen == 0)
    {
        return 1;
    }
    
    /* ������ݳ����Լ� */
    fifo->fifoLen--;

    /* �������� */
    *event =  (BSP_BTN_EVENT_t )(fifo->buff[fifo->fifoRead]);

    /* �������λ���Ѿ����˶�β */
    if(++fifo->fifoRead >= BSP_BTN_FIFO_LENGTH)
    {
        fifo->fifoRead = 0;//��ô��һ�ν��Ӷ��׿�ʼ����
    }

    return 0;
}
