/**
  **********************************************************************************
  * @file      ebyte_port.h
  * @brief     EBYTE�������Ӳ���ӿڲ� �ɿͻ��Լ��ṩIO�ӿ�
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
#include "ebyte_port.h"


/*= !!!����Ŀ��Ӳ��ƽ̨ͷ�ļ� =======================================*/
#include "board.h"  //E15-EVB02 ������
/*==================================================================*/

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨SPI�ӿ��շ�����
 *
 * @param send EBYTE�������ϲ������Ҫ��������� 1 Byte
 * @return SPI���յ����� 1 Byte
 */
uint8e_t Ebyte_Port_SpiTransmitAndReceivce( uint8e_t send )
{
    uint8e_t result = 0;
    /*!�����ṩ: SPI�ӿ� */
    result = Ebyte_BSP_SpiTransAndRecv( send );//ʾ��
    return result;
}

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨SPI_CS(NSS)����
 *
 * @param cmd EBYTE�������ϲ������Ŀ���ָ��
 *            @arg 0: ����CS(NSS)��������͵�ƽ�߼��ź� EBYTEģ��SPI�ӿ�Ϊ�͵�ƽѡ��
 *            @arg 1: ����CS(NSS)��������ߵ�ƽ�߼��ź�
 */
void Ebyte_Port_SpiCsIoControl( uint8e_t cmd )
{
    if( cmd == 1 )
    {
        /* SPI CS���� �ߵ�ƽδѡ�� */
        Ebyte_BSP_RfSpiUnselected();//ʾ��
    }
    else
    {
        /* SPI CS���� �͵�ƽѡ�� */
        Ebyte_BSP_RfSpiSelected( );//ʾ��
    }
}

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨��λ���� NRST ���ƺ���
 *
 * @param cmd EBYTE�������ϲ������Ŀ���ָ��
 *            @arg 0: ����NRST ��������͵�ƽ�߼��ź�
 *            @arg 1: ����NRST ��������ߵ�ƽ�߼��ź�
 * @note EBYTEģ��NRSTΪ�͵�ƽ��λ
 */
void Ebyte_Port_RstIoControl( uint8e_t cmd )
{
    if( cmd == 1 )
    {
        /*!�����ṩ: NRST����IO ����ߵ�ƽ */
        Ebyte_BSP_E19ResetIoHigh();//ʾ��
    }
    else
    {
        /*!�����ṩ: NRST����IO ����͵�ƽ */
        Ebyte_BSP_E19ResetIoLow();//ʾ��
    }
}

/*!
 * @brief (��ѡ)����Ŀ��Ӳ��ƽ̨����ʹ������ TXEN ���ƺ���
 *
 * @param cmd EBYTE�������ϲ������Ŀ���ָ��
 *            @arg 0: ����TXEN ��������͵�ƽ�߼��ź�
 *            @arg 1: ����TXEN ��������ߵ�ƽ�߼��ź�
 */
void Ebyte_Port_TxenIoControl( uint8e_t cmd )
{
    if( cmd == 1 )
    {
        /*!��ѡ: Ŀ��Ӳ����·��ӦTXEN����IO ����ߵ�ƽ */
        Ebyte_BSP_E19TxIoEnable();//ʾ��
    }
    else
    {
        /*!��ѡ: Ŀ��Ӳ����·��ӦTXEN����IO ����͵�ƽ */
        Ebyte_BSP_E19TxIoDisable();//ʾ��
    }
}

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨����ʹ������ RXEN ���ƺ���
 *
 * @param cmd EBYTE�������ϲ������Ŀ���ָ��
 *            @arg 0: ����RXEN��������͵�ƽ�߼��ź�
 *            @arg 1: ����RXEN��������ߵ�ƽ�߼��ź�
 * @note EBYTEģ��RXENΪ�ߵ�ƽ��Ч
 */
void Ebyte_Port_RxenIoControl( uint8e_t cmd )
{
    if( cmd == 1 )
    {
        /*!�����ṩ: Ŀ��Ӳ����·��ӦRXEN����IO ����ߵ�ƽ */
        Ebyte_BSP_E19RxIoEnable();//ʾ��
    }
    else
    {
        /*!�����ṩ: Ŀ��Ӳ����·��ӦRXEN����IO ����͵�ƽ */
        Ebyte_BSP_E19RxIoDisable();//ʾ��
    }
}

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨ ͬ������DIO0 ��ȡ����
 */
uint8e_t Ebyte_Port_Dio0IoRead( void )
{
    uint8e_t result = 0 ;
    /*!�����ṩ: Ŀ��Ӳ����·��ӦDIO0 IO ״̬��ȡ */
    result = Ebyte_BSP_E19Dio0Read();//ʾ��
    return result;
}

/*!
 * @brief ����Ŀ��Ӳ��ƽ̨��ʱ����
 *
 * @param time ������ʱ����
 */
void Ebyte_Port_DelayMs( uint16e_t time )
{
    /*!�����ṩ: ��ʱ���� */
    uint16e_t i, n;
    while( time-- ) //����ʾ����STM8L 16Mʱ��ǰ���µĴ��������ʱ
    {
        for( i = 900; i > 0; i-- )
        {
            for( n = 1 ; n > 0 ; n-- )
            {
                asm( "nop" );
                asm( "nop" );
                asm( "nop" );
                asm( "nop" );
            }
        }
    }
}



