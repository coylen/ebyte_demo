/**
  **********************************************************************************
  * @file      ebyte_e19x.h
  * @brief     E19 (sx1278) ϵ�� ������
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
 * �ز�����Ƶ��
 *
 * @note �й�-��ҵ����Ϣ����
 *
 * ���л����񹲺͹����ߵ�Ƶ�ʻ��ֹ涨��(��ҵ����Ϣ�������46���� 2018��7��1����ʩ��)
 *  �涨�������ر���׼ISMӦ��Ƶ��( Sub-1G��Χ ):
 *          6.765��6.795MHz ������Ƶ��6.780MHz��
 *         13.553��13.567MHz (����Ƶ��13.560MHz)
 *         26.957��27.283MHz (����Ƶ��27.120MHz)
 *          40.66��40.7MHz   (����Ƶ��40.68MHz��
 *
 * ��΢����(�̾���)���ߵ��豸�ļ���Ҫ�󡷣��Ų��ޡ�2005��423�ţ�(��ҵ����Ϣ������52���� 2019��)
 *  �涨��΢�������ߵ��豸Ӧ��Ƶ��( Sub-1G��Χ ):
 *  A��      9KHz��190KHz
 *  B��    1.7MHz��2.1MHz
 *         2.2MHz��3.0MHz
 *         3.1MHz��4.1MHz
 *         4.2MHz��5.6MHz
 *         5.7MHz��6.2MHz
 *         7.3MHz��8.3MHz
 *         8.4MHz��9.9MHz
 *  C��  6.765MHz��6.795MHz
 *      13.553MHz��13.567MHz
 *      26.957MHz��27.283MHz
 *  D��    315kHz��30MHz
 *  E��  40.66MHz��40.7MHz
 *  -------------------------
 *      26.975MHz��27.255MHz    ģ�͡����: ����/��ģ
 *       40.61MHz��40.75MHz     ģ�͡����: ����/��ģ
 *       40.77MHz��40.85MHz     ģ�͡����: ��ģ
 *          45MHz��45.475MHz    ģ��ʽ�绰��: ������
 *          48MHz��48.475MHz    ģ��ʽ�绰��: �ֻ���
 *       72.13MHz��72.84MHz     ģ�͡����: ��ģ
 *        75.4MHz��76.0MHz      �������估�����Ǳ�
 *          84MHz��87MHz        �������估�����Ǳ�
 *          87MHz��108MHz       �������估�����Ǳ�
 *         174MHz��216MHz       ����ҽѧ�豸
 *       189.9MHz��223.0MHz     �������估�����Ǳ�
 *         223MHz��224MHz       ���ӵ���ר��
 *         224MHz��231MHz       �������ݴ����豸
 *         314MHz��316MHz       �����豸����
 *         407MHz��425MHz       ����ҽѧ�豸
 *      409.75MHz��409.9875MHz  ���ڶԽ���
 *     419.950MHz��419.275MHz   ��ҵ������ң���豸
 *         430MHz��432MHz       �����豸����
 *         433MHz��434.79MHz    �����豸����
 *    450.0125MHz��450.2125MHz  �����豸����
 *         470MHz��510MHz       ���ü����豸
 *         470MHz��566MHz       ͨ������ң���豸
 *         608MHz��630MHz       ����ҽѧ�豸
 *         614MHz��787MHz       ͨ������ң���豸
 *
 * @note ���Ϲ�-���ʵ�������-���ߵ�ͨ�Ų���(ITU-R)
 *
 * ��Report ITU-R SM.2180: Impact of Industrial, Scientific and Medical (ISM) Equipment on Radiocommunication Services��(2011��1��)
 *  SM.2180����ָ�������ر���׼��ISMӦ��Ƶ��( Sub-1G��Χ ):
 *          6.765��6.795MHz ������Ƶ��6.780MHz��
 *         13.553��13.567MHz (����Ƶ��13.560MHz)
 *         26.957��27.283MHz (����Ƶ��27.120MHz)
 *          40.66��40.7MHz   (����Ƶ��40.68MHz��
 *         433.05��434.79MHz (����Ƶ��433.92MHz��   ������ITU 1��ʹ�� (ŷ�޺ͷ����Լ��ɹš�ԭ�����Ա��ĵ�����ŷ�ޡ�����������ԭ�����������������)
 *            902��928MHz    (����Ƶ��915MHz��      ������ITU 2��ʹ�� (�ϡ������޺�������)
 *  �й���ITU 3��(���ޡ������޺������������������)
 *
 * @note ŷ��
 *  ��COMMISSION DECISION of 9 November 2006 - on harmonisation of the radio spectrum for use by short-range devices��(2006/771/EC)
 *   ŷ�����ߵ翪��Ƶ���еģ������ر���׼Ƶ�λ������У�
 *          868.0��869.25MHz
 *
 *   ����CE��֤�����˽�ŷ�������豸ָ��RED 2014/53/EU
 *
 * @note ����
 *  ��47 CFR Part 18 - INDUSTRIAL, SCIENTIFIC, AND MEDICAL EQUIPMENT��
 *   FCC��18���ֹ���涨��ISMƵ��
 *
 *   ����FCC��֤�����˽� 47 CFR Part 15
 */

/* EBYTE_E19_433M20SC ģ�����ͨ��Ƶ��  */
#if defined(EBYTE_E19_433M20SC)
#define   E19X_FREQUENCY_START                          434000000 // Hz ��ʼƵ��


#endif


#define   E19X_MODULATION_TYPE         0   //���Ʒ�ʽ 0:LoRa   
#define   E19X_OUTPUT_POWER           20   //������ 20dBm

#define   E19X_LORA_BANDWIDTH          7   //LoRa���Ʋ��� Ƶ��BW      [ 0: 7.8 kHz, 1: 10.4 kHz, 2: 15.6 kHz, 3: 20.8 kHz, 4: 31.2 kHz 5: 41.6 kHz, 6: 62.5 kHz, 7: 125 kHz, 8: 250 kHz, 9: 500 kHz ]
#define   E19X_LORA_SPREADING_FACTOR   7   //LoRa���Ʋ��� ��Ƶ����SF  [ 6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips]
#define   E19X_LORA_CORING_RATE        1   //LoRa���Ʋ��� ������CR    [ 1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8 ]
#define   E19X_LORA_CRC                1   //CRC����                  [ 0: OFF, 1: ON]
#define   E19X_LORA_LOW_DATARATE       1   //�Ϳ����Ż�����            [ 0: OFF, 1: ON]
#define   E19X_LORA_PREAMBLE_LENGTH   20   //LoRaǰ���볤��  
#define   E19X_LORA_SYNC_WORD       0x12   //LoRaͬ����       ������ǲ���LoRaWAN �����޸�Ϊ����ͬ����: 0x34

/* ��μ��ģ�鷢�������  0:�첽�ж�֪ͨ  1:ͬ������ֱ��������� */
#define EBYTE_TRANSMIT_MODE_BLOCKED   1

uint8e_t E19x_Init( void );
uint8e_t E19x_GoReceive( void );
uint8e_t E19x_GoTransmit( uint8e_t* data, uint8e_t size );
uint8e_t E19x_GoSleep( void );
uint8e_t E19x_TaskForPoll( void );
uint8e_t* E19x_GetName(void);
uint8e_t E19x_GetDriverVersion( void );
uint8e_t E19x_GetStatus( void );
void E19x_TaskForIRQ( void );