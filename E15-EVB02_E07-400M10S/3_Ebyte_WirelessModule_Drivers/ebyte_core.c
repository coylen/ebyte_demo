/**
  **********************************************************************************
  * @file      ebyte_core.c
  * @brief     EBYTE��������ϲ�API��װ�� ����ײ��߼�    
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

#include "ebyte_core.h"


/* ָ�� E07x �ײ��������� */
const Ebyte_RF_t Ebyte_RF =
{
  E07x_Init,
  E07x_GoTransmit,
  E07x_GoSleep,
  E07x_GoReceive,  
  E07x_TaskForPoll,
  E07x_TaskForIRQ,
  E07x_GetStatus,
  E07x_GetName,
  E07x_GetDriverVersion
};




