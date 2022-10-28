/**
  **********************************************************************************
  * @file      ebyte_e19x.c
  * @brief     EBYTE E19 (sx1278) ϵ������
  * @details   ������μ� https://www.ebyte.com/
  * @author    JiangHeng
  * @date      2021-06-16
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

#include "ebyte_e19x.h"

/// ʶ���룺ģ�����
#if defined(EBYTE_E19_433M20SC)
static uint8e_t E19x_NameString[] = "E19-433M20SC";
#endif

/// ʶ���룺��������汾��
#define EBYTE_E19_PROGRAM_TYPE 0x10

/// ģ���ڲ��շ���·ģʽ
#define ANTENNA_RXEN  0
#define ANTENNA_TXEN  1

/// Ƶ�ʸ����������
#define FREQ_STEP     61.03515625

/// �жϼĴ�����ʶ ֧�ֵ�����
#define IRQ_RXTIMEOUT                     0x80
#define IRQ_RXDONE                        0x40
#define IRQ_PAYLOADCRCERROR               0x20
#define IRQ_VALIDHEADER                   0x10
#define IRQ_TXDONE                        0x08
#define IRQ_CADDONE                       0x04
#define IRQ_FHSSCHANGEDCHANNEL            0x02
#define IRQ_CADDETECTED                   0x01

typedef enum
{
    GO_INIT          = 0x00,
    GO_BUSY          = 0x01,
    GO_STBY          = 0x02,
    GO_RECEIVE       = 0x03,
    GO_WAIT_RECEIVE  = 0x04,
    GO_TRANSMIT      = 0x05,
    GO_WAIT_TRANSMIT = 0x06,
    GO_SLEEP         = 0x07,
    GO_ERROR         = 0x08
} E19x_Status_t;

/// ״̬��ʶ
static E19x_Status_t E19x_Status = GO_INIT;

/// �������ݻ���
static uint8e_t E19x_ReceiveBuffer[255] ;



/*!
 * @brief �л�ģ���ڲ��շ���·
 *
 * @param mode �л���ָ��ģʽ
 *      @arg ANTENNA_RXEN: ����ģʽ
 *      @arg ANTENNA_TXEN: ����ģʽ
 *
 * @note �շ���·�л�����ȷ�ᵼ�·��书���½� �����޷������շ�����
 */
static void E19X_SetAntenna( uint8e_t mode )
{
    if( mode == ANTENNA_TXEN )
    {
        Ebyte_Port_RxenIoControl( 0 );
        Ebyte_Port_TxenIoControl( 1 );
    }
    else
    {
        Ebyte_Port_TxenIoControl( 0 );
        Ebyte_Port_RxenIoControl( 1 );
    }
}

/*!
 * @brief ����д��Ĵ���
 *
 * @param address �Ĵ�����ʼ��ַ
 * @param data    ��д��������ָ��
 * @param size    �Ĵ�������
 */
static void E19x_SetRegisters( uint8e_t address, uint8e_t* data, uint8e_t size )
{
    uint8e_t i;
    /* CSƬѡ �͵�ƽѡ�� */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* �Ĵ�����ʼ��ַ */
    Ebyte_Port_SpiTransmitAndReceivce( address | 0x80 ); //bit7=1 ��ʾ�������ݰ�Ϊд�� �� |0x80
    /* ����д�� */
    for( i = 0; i < size; i++ )
    {
        Ebyte_Port_SpiTransmitAndReceivce( data[i] );
    }
    /* CSƬѡ �ߵ�ƽ */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief ������ȡ�Ĵ���
 *
 * @param address �Ĵ�����ʼ��ַ
 * @param data    ָ���ȡ���ݴ�Ż�����
 * @param size    �Ĵ�������
 */
static void E19x_GetRegisters( uint8e_t address, uint8e_t* data, uint8e_t size )
{
    uint8e_t i;
    /* CSƬѡ �͵�ƽѡ�� */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* �Ĵ�����ʼ��ַ */
    Ebyte_Port_SpiTransmitAndReceivce( address & 0x7F ); //bit7=0 ��ʾ�������ݰ�Ϊ��ȡ �� &0x7F
    /* ������ȡ */
    for( i = 0; i < size; i++ )
    {
        data[i] = Ebyte_Port_SpiTransmitAndReceivce( 0 );
    }
    /* CSƬѡ �ߵ�ƽ */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief д�뵥���Ĵ���
 *
 * @param address �Ĵ�����ʼ��ַ
 * @param data    д������
 */
static void E19x_SetRegister( uint8e_t address, uint8e_t data )
{
    E19x_SetRegisters( address, &data, 1 );
}

/*!
 * @brief ��ȡ�����Ĵ���
 *
 * @param address �Ĵ�����ʼ��ַ
 * @param data    д������
 */
static uint8e_t E19x_GetRegister( uint8e_t address )
{
    uint8e_t data;
    E19x_GetRegisters( address, &data, 1 );
    return data;
}

/*!
 * @brief ����������д��ģ���ڲ����ݶ���
 *
 * @param data ָ����������ݰ�
 * @param size д�����ݳ���
 *
 * @note ģ���л�Ϊ����ģʽ�󣬻��Զ��Ӷ�������ȡ���ݽ������ߴ���
 */
static void E19x_SetFIFO( uint8e_t* data, uint8e_t size )
{
    E19x_SetRegisters( 0, data, size );
}

/*!
 * @brief ��ȡģ���ڲ����ݶ���
 *
 * @param data ָ���û��������ݻ���
 * @param size �������ݳ���
 *
 * @note ע��ʱ�򣬽���ģʽʱ��Ҫ�����ö�ȡ��ַ����λ�ȡʵ�ʽ��ճ��ȣ���ɺ��ٶ�ȡFIFO
 */
static void E19x_GetFIFO( uint8e_t* data, uint8e_t size )
{
    E19x_GetRegisters( 0, data, size );
}

/*!
 * @brief ģ�鸴λ
 *
 * @note �ڲ��Ĵ����������ỹԭΪĬ��ֵ
 */
static void E19x_Reset( void )
{
    Ebyte_Port_DelayMs( 10 );
    Ebyte_Port_RstIoControl( 0 );
    Ebyte_Port_DelayMs( 20 );
    Ebyte_Port_RstIoControl( 1 );
    Ebyte_Port_DelayMs( 20 );
}

/*!
 * @brief ����ģ��GPIO(DIO)�Ĺ���
 *
 * @param regDioMapping1  �Ĵ�����ַ:0x40
 * @param regDioMapping2  �Ĵ�����ַ:0x41
 *
 * @note һ������ָ��һ�������ڽ��ջ��߷������ʱ����MCU�ж�
 *       �����ܹ���Ӧ���ж������������Ƶ� �����SX1278�ֲ�(2.0.1 Digital IO Pin Mapping)
 */
void E19x_SetGPIO( uint8e_t regDioMapping1, uint8e_t regDioMapping2 )
{
    E19x_SetRegister( 0x40, regDioMapping1 );
    E19x_SetRegister( 0x41, regDioMapping2 );
}

/*!
 * @brief �ж�ģ���Ƿ����
 *
 * @note ͨ����ȡоƬ�汾����ʵ�ֵ�
 */
uint8e_t E19x_IsExist( void )
{
    uint8e_t result = 1;
    uint8e_t version;
    /* ��ȡоƬ�汾�� ����Ӧ����0x12 �Ĵ�����ַ:0x42   */
    version = E19x_GetRegister( 0x42 );
    if( version == 0x12 )
    {
        /* ���� */
        result = 0;
    }
    return result;
}

/*!
 * @brief �����ж�
 *
 * @param irq �ж����� ���궨�� ��λ��
 *      @arg IRQ_RXTIMEOUT
 *      @arg IRQ_RXDONE
 *      @arg IRQ_PAYLOADCRCERROR
 *      @arg IRQ_VALIDHEADER
 *      @arg IRQ_TXDONE
 *      @arg IRQ_CADDONE
 *      @arg IRQ_FHSSCHANGEDCHANNEL
 *      @arg IRQ_CADDETECTED
 */
void E19x_SetIRQ( uint8e_t irq )
{
    uint8e_t reg_value;
    /* �����жϵĻ���Ϊ��ӦΪ�� 0 */
    reg_value = ~irq;
    E19x_SetRegister( 0x11, reg_value );
}

/*!
 * @brief ��ȡ�жϱ�ʶ
 *
 * @return һ�㶼�Ƿ������/�������
 *       0x80:IRQ_RXTIMEOUT
 *       0x40:IRQ_RXDONE
 *       0x20:IRQ_PAYLOADCRCERROR
 *       0x10:IRQ_VALIDHEADER
 *       0x08:IRQ_TXDONE
 *       0x04:IRQ_CADDONE
 *       0x02:IRQ_FHSSCHANGEDCHANNEL
 *       0x01:IRQ_CADDETECTED
 */
uint8e_t E19x_GetIRQ( void )
{
    /* ��ȡ�ж�״̬ �Ĵ�����ַ:0x12 */
    return E19x_GetRegister( 0x12 );
}

/*!
 * @brief ��������жϱ�ʶ
 */
void E19x_ClearIRQ( void )
{
    uint8e_t reg_value = 0;
    /* ��ȡ�ж�״̬ �Ĵ�����ַ:0x12 */
    reg_value = E19x_GetRegister( 0x12 );
    /* �ж��������Ϊ��д��Ӧλ 1 */
    E19x_SetRegister( 0x12, reg_value );
}

/*!
 * @brief ��������ģʽ
 */
void E19x_SetSleep( void )
{
    uint8e_t result;
    /* ��ȡģʽ���ƼĴ��� �Ĵ�����ַ:0x01   */
    result = E19x_GetRegister( 0x01 );
    /* bit2��bit0 3λ��ʾģʽ 0����SLEEPģʽ  */
    result &= 0xF8;//bit����
    /* ��д */
    E19x_SetRegister( 0x01, result );
    /* ״̬��¼ */
    E19x_Status = GO_SLEEP;
}

/*!
 * @brief �����������ģʽ
 */
void E19x_SetStby( void )
{
    uint8e_t result;
    /* ��ȡģʽ���ƼĴ��� �Ĵ�����ַ:0x01   */
    result = E19x_GetRegister( 0x01 );
    /* bit2��bit0 3λ��ʾģʽ 0x01����STDBYģʽ  */
    result &= 0xF8;//bit����
    result |= 0x01;//bit��λ
    /* ��д */
    E19x_SetRegister( 0x01, result );
    /* ״̬��¼ */
    E19x_Status = GO_STBY;
}

/*!
 * @brief ���뷢��ģʽ
 */
void E19x_SetTransmit( void )
{
    uint8e_t result;
    /* ��ȡģʽ���ƼĴ��� �Ĵ�����ַ:0x01   */
    result = E19x_GetRegister( 0x01 );
    /* bit2��bit0 3λ��ʾģʽ 0x03����TXģʽ  */
    result &= 0xF8;//bit����
    result |= 0x03;//bit��λ
    /* ��д */
    E19x_SetRegister( 0x01, result );
    /* ״̬��¼  �Ѿ���ʼ���ͣ���������״̬Ϊ�ȴ�������� */
    E19x_Status = GO_WAIT_TRANSMIT;
}

/*!
 * @brief �������ģʽ
 */
void E19x_SetReceive( void )
{
    uint8e_t result;
    /* ��ȡģʽ���ƼĴ��� �Ĵ�����ַ:0x01   */
    result = E19x_GetRegister( 0x01 );
    /* bit2��bit0 3λ��ʾģʽ 0x05��������RXģʽ  */
    result &= 0xF8;//bit����
    result |= 0x05;//bit��λ
    /* ��д */
    E19x_SetRegister( 0x01, result );
    /* ״̬��¼  �Ѿ���ʼ���գ���������״̬Ϊ�ȴ��������״̬ */
    E19x_Status = GO_WAIT_RECEIVE;
}

/*!
 * @brief �����ŵ����ģʽ LBT��(Listen Before Talk)
 */
void E19x_SetCAD( void )
{
    uint8e_t result;
    /* ��ȡģʽ���ƼĴ��� �Ĵ�����ַ:0x01   */
    result = E19x_GetRegister( 0x01 );
    /* bit2��bit0 3λ��ʾģʽ 0x07��������CADģʽ  */
    result &= 0xF8;//bit����
    result |= 0x07;//bit��λ
    /* ��д */
    E19x_SetRegister( 0x01, result );
}

/*!
 * @brief LoRaģʽ��ʼ��
 *
 * @param frequency         ��λHz  �������� 433000000 ��ʾ433.0MHz
 * @param bandwidth         Ԥ����Ƶ��
 * @param spreading_factor  Ԥ������Ƶ����
 * @param coding_rate       Ԥ���������
 * @param crc               CRCУ�鿪��
 * @param preamble_length   ǰ���볤��
 * @param sync_word         ͬ����
 * @param output_power      ������� (����:1dBm)
 * @param low_datarate      �Ϳ����Ż�����
 *
 * @return 0:����  1:���ò�������
 *
 * @note Ԥ���������ebyte_e19x.h�еĺ궨�� ���� E19X_LORA_BANDWIDTH
 */
uint8e_t E19x_SetLoRaInit( uint32e_t frequency, uint8e_t bandwidth, uint8e_t spreading_factor, uint8e_t coding_rate,
                           uint8e_t crc, uint16e_t preamble_length, uint8e_t sync_word, int8e_t output_power, uint8e_t low_datarate )
{
    uint8e_t config[8];
    uint8e_t reg_value;
    uint32e_t freq;
    /* ֻ����Sleepģʽ�� �л����Ʒ�ʽ */
    E19x_SetSleep();
    /* �л�ΪLoRaģʽ �Ĵ�����ַ:0x01 bit7=1 */
    E19x_SetRegister( 0x01, 0x80 );
    /* �������� */
    E19x_SetStby();
    /* ����Ƶ�� */
    freq = ( uint32e_t )( ( double )frequency / ( double )FREQ_STEP );
    config[0] = ( uint8e_t )( ( freq >> 16 ) & 0xFF );
    config[1] = ( uint8e_t )( ( freq >> 8 ) & 0xFF );
    config[2] = ( uint8e_t )( freq & 0xFF );
    E19x_SetRegisters( 0x06, config, 3 );  //�Ĵ�����ʼ��ַ:0x06  ����3�ֽ�
    /* ������Ƶ���� SF
       SF = 6 �� LoRa ���ƽ��������ʵ�ֵ�����������ʴ����������������Ҫ�������� */
    if( spreading_factor < 6 || spreading_factor > 12 )
    {
        return 1;    //�������
    }
    if( spreading_factor == 6 )
    {
        reg_value = E19x_GetRegister( 0x31 );   //����DetectionOptimize �Ĵ�����ַ:0x31 bit2��bit0
        reg_value = ( reg_value & 0xF8 ) | 0x05;//0x05ר����SF6
        E19x_SetRegister( 0x31, reg_value );
    }
    else
    {
        reg_value = E19x_GetRegister( 0x31 );   //����DetectionOptimize �Ĵ�����ַ:0x31 bit2��bit0
        reg_value = ( reg_value & 0xF8 ) | 0x03;//0x03ר����SF7��SF12
        E19x_SetRegister( 0x31, reg_value );
    }
    reg_value = E19x_GetRegister( 0x1E );       //����SpreadingFactor �Ĵ�����ַ:0x1E bit7��bit4
    reg_value = ( reg_value & 0x0F ) | ( spreading_factor << 4 );
    E19x_SetRegister( 0x1E, reg_value );
    /* ���ñ����� CR */
    if( coding_rate < 1 || coding_rate > 4 )
    {
        return 1;    //�������
    }
    reg_value = E19x_GetRegister( 0x1D );       //����CodingRate �Ĵ�����ַ:0x1D bit3��bit1
    reg_value = ( reg_value & 0xF1 ) | ( coding_rate << 1 );
    E19x_SetRegister( 0x1D, reg_value );
    /* �������ݰ�У�� CRC */
    if( crc > 1 )
    {
        return 1;    //�������
    }
    reg_value = E19x_GetRegister( 0x1E );       //����RxPayloadCrcOn �Ĵ�����ַ:0x1E bit7��bit4
    reg_value = ( reg_value & 0xFB ) | ( crc << 2 );
    E19x_SetRegister( 0x1E, reg_value );
    /* ����Ƶ��  */
    if( bandwidth > 9 )
    {
        return 1;    //�������
    }
    reg_value = E19x_GetRegister( 0x1D );       //����Bw  �Ĵ�����ַ:0x1D bit7��bit4
    reg_value = ( reg_value & 0x0F ) | ( bandwidth << 4 );
    E19x_SetRegister( 0x1D, reg_value );
    /* ����ǰ���볤�� */
    config[0] = ( preamble_length >> 8 ) & 0x00FF;
    config[1] = preamble_length & 0xFF;
    E19x_SetRegisters( 0x20, config, 2 ); //�Ĵ�����ʼ��ַ:0x20  ����2�ֽ�
    /* ����ͬ���� */
    E19x_SetRegister( 0x39, sync_word ); //�Ĵ�����ʼ��ַ:0x39
    /* ���õͿ����Ż� */
    if( low_datarate > 1 )
    {
        return 1;    //�������
    }
    reg_value = E19x_GetRegister( 0x26 );       //����LowDataRateOptimize �Ĵ�����ַ:0x26 bit3
    reg_value = ( reg_value & 0xF7 ) | ( low_datarate << 3 );
    E19x_SetRegister( 0x26, reg_value );
    /* ����������� �͹�������ѡ��2-17Ϊ�� �������л�Ϊ17-20 */
    if( output_power < 2 || output_power > 20 )
    {
        return 1;    //�������
    }
    if( output_power >= 17 ) // 17-20 dBm
    {
        E19x_SetRegister( 0x4D, 0x87 );          //����RegPaDac �Ĵ�����ַ:0x4D  ģ��ɿ�������ʷ�Χ�л�Ϊ 5dBm��20dBm
        reg_value  = E19x_GetRegister( 0x09 );   //����PaSelect �Ĵ�����ַ:0x09
        reg_value |= 0x80 ;                      //bit7  PA_BOOST�������
        reg_value |= 0x70 ;                      //����MaxPower
        reg_value = ( reg_value & 0xF0 ) | ( ( uint8e_t )( ( uint16e_t )( output_power - 5 ) & 0x0F ) ); //����OutputPower
        E19x_SetRegister( 0x09, reg_value );
        E19x_SetRegister( 0x0B, 0x3B );          //����RegOcp   �Ĵ�����ַ:0x0B  ������� ������������
    }
    else                  // 2-17 dBm
    {
        E19x_SetRegister( 0x4D, 0x84 );          //����RegPaDac �Ĵ�����ַ:0x4D  ģ��ɿ�������ʷ�Χ�л�Ϊ 2dBm��17dBm
        reg_value  = E19x_GetRegister( 0x09 );   //����PaSelect �Ĵ�����ַ:0x09
        reg_value &= 0x7F ;                      //bit7  PA_BOOST�������
        reg_value |= 0x70 ;                      //����MaxPower
        reg_value = ( reg_value & 0xF0 ) | ( ( uint8e_t )( ( uint16e_t )( output_power - 2 ) & 0x0F ) ); //����OutputPower
        E19x_SetRegister( 0x09, reg_value );
    }
    return 0;
}

/*!
 * @brief ģ�鿪ʼ������׼����������
 *
 * @return 0
 * @note
 */
uint8e_t E19x_GoReceive( void )
{
    /* ģʽ�л�:���� */
    E19x_SetStby();
    /* �ر���Ƶ �Ĵ�����ַ:0x24 */
    E19x_SetRegister( 0x24, 0 );
    /* ����GPIOӳ�� ��������DIO0ӳ�䵽RxDone ��������������ж�
       �����̲�����ѯ��ȡģ���ڲ���־λ�ķ�ʽ���ж��Ƿ������ɡ����ԣ�IO������Ҳû�õ���
       ���迼���жϣ�����Ҫ��IO�жϺ����ж�ȡ���ݣ�����жϼ��� */
    E19x_SetGPIO( 0, 0 );
    /* FIFO: ���ý������ݴ����ʼ��ַ ��0��ʼ  �Ĵ�����ַ:0x0F */
    E19x_SetRegister( 0x0F, 0 );
    /* IO: �������յ�· */
    E19X_SetAntenna( ANTENNA_RXEN );
    /* ��������ж� */
    E19x_SetIRQ( IRQ_RXDONE );
    /* ����жϱ�ʶ */
    E19x_ClearIRQ();
    /* �л�Ϊ����ģʽ */
    E19x_SetReceive();
    return 0;
}

/*!
 * @brief ��ģ��д�����ݣ���ʼ���ߴ���
 *
 * @param data ָ����������ݰ�
 * @param size ���ݰ�����
 *
 * @return 0
 *
 * @note ���ݰ���󳤶�255
 */
uint8e_t E19x_GoTransmit( uint8e_t* data, uint8e_t size )
{
    uint8e_t irqStatus = 0;
    /* ģʽ�л�:���� */
    E19x_SetStby();
    /* �ر���Ƶ �Ĵ�����ַ:0x24 */
    E19x_SetRegister( 0x24, 0 );
    /* �������ݰ����� �Ĵ�����ַ:0x22 */
    E19x_SetRegister( 0x22, size );
    /* FIFO:������ ���͵�ַ��128��ʼ  �Ĵ�����ַ:0x0E */
    E19x_SetRegister( 0x0E, 0x80 );
    /* FIFO:������ SPIд���ַ��128��ʼ  �Ĵ�����ַ:0x0D */
    E19x_SetRegister( 0x0D, 0x80 );
    /* FIFO:д�뷢������ */
    E19x_SetFIFO( data, size );
    /* ����GPIOӳ�� ��������DIO0ӳ�䵽TxDone ��������������ж�
       �����̲�����ѯ��ȡģ���ڲ���־λ�ķ�ʽ���ж��Ƿ�����ɡ����ԣ�IO������Ҳû�õ���
       ���迼���жϣ���Ҫ���ú궨�� EBYTE_TRANSMIT_MODE_BLOCKED ��������IO�жϴ��� */
    E19x_SetGPIO( 0x40, 0x40 );
    /* IO: �������͵�· */
    E19X_SetAntenna( ANTENNA_TXEN );
    /* �������ж� */
    E19x_SetIRQ( IRQ_TXDONE );
    /* ����жϱ�ʶ */
    E19x_ClearIRQ();
    /* �л�Ϊ����ģʽ */
    E19x_SetTransmit();
#if (EBYTE_TRANSMIT_MODE_BLOCKED)
    /* ���� �ȴ�������� */
    do
    {
        Ebyte_Port_DelayMs( 1 );
        irqStatus = E19x_GetIRQ();
    }
    while( irqStatus == 0xFF || ( !( irqStatus & IRQ_TXDONE ) ) ); //���ڸ���ʱ,�и��ʶ���0xFF,������Ϊ�Ǵ�������
    /* ���˷������ ����ж� */
    E19x_ClearIRQ();
    /* ���� */
    E19x_SetStby();
    /* �ص��û������� */
    Ebyte_Port_TransmitCallback( 0x0001 );
#endif
    return 0;
}

/*!
 * @brief ģ���������(�͹���)
 *
 * @return 0
 */
uint8e_t E19x_GoSleep( void )
{
    E19x_SetSleep();
    return 0;
}


/*!
 * @brief BBYTE ����ģ���ʼ��
 */
uint8e_t E19x_Init( void )
{
    uint8e_t result;
    /* ��λ */
    E19x_Reset();
    /* ��飺ģ���Ƿ���� */
    result = E19x_IsExist();
    if( result != 0 )
    {
        return 1;
    }
    /* ��ʼ����LoRa �� FSK */
    switch( E19X_MODULATION_TYPE )
    {
        case 0:
            /* ִ��LoRaģʽ��ʼ�� ���ü��궨�� */
            E19x_SetLoRaInit( E19X_FREQUENCY_START,        //�ز�Ƶ��
                              E19X_LORA_BANDWIDTH,         //Ƶ��
                              E19X_LORA_SPREADING_FACTOR,  //��Ƶ����
                              E19X_LORA_CORING_RATE,       //������
                              E19X_LORA_CRC,               //CRC����
                              E19X_LORA_PREAMBLE_LENGTH,   //ǰ���볤��
                              E19X_LORA_SYNC_WORD,         //ͬ����
                              E19X_OUTPUT_POWER,           //�������
                              E19X_LORA_LOW_DATARATE );    //�Ϳ����Ż�
            break;
        case 1:
            //FSK
            break;
        default:
            break;
    }
    E19x_GoReceive();
    return 0;
}


/*!
 * @brief ����
 */
void E19x_TaskForIRQ( void )
{
}

/*!
 * @brief ��ѯ���� ��Ҫ������ѭ������
 *
 * @return 0
 * @note ��ȡ�ڲ���ʶλ���Ӷ�������ݽ���
 */
uint8e_t E19x_TaskForPoll( void )
{
    uint8e_t irqStatus;
    /* ��ȡģ���ڲ��жϱ�ʶ״̬ */
    irqStatus = E19x_GetIRQ();
    /* ��0��ʾĳһλ�ж���λ */
    if( irqStatus != 0 )
    {
        /* ����ǽ������ */
        if( irqStatus & IRQ_RXDONE )
        {
            /* ģʽ�л�:���� */
            E19x_SetStby();
            /* ��ȡ���½������ݴ����FIFO�е�λ��  �Ĵ�����ַ:0x10 */
            uint8e_t rxAddress = E19x_GetRegister( 0x10 );
            /* ��ȡ���½������ݵĳ���  �Ĵ�����ַ:0x13 */
            uint8e_t rxLength = E19x_GetRegister( 0x13 );
            /* ����SPI��ȡFIFO ��ʼ��ַ  �Ĵ�����ַ:0x0D */
            E19x_SetRegister( 0x0D, rxAddress );
            /* ��ȡFIFO���� */
            E19x_GetFIFO( E19x_ReceiveBuffer, 100 );
            /* �ص��û����� */
            Ebyte_Port_ReceiveCallback( 0x0001, E19x_ReceiveBuffer, rxLength );
        }
        /* ����ж� */
        E19x_ClearIRQ();
    }
    return 0;
}

/*!
 * @brief ��ȡģ������
 *
 * @return ָ�������ַ�����ָ��
 * @note ��׼�ַ��� ĩβ���н����� '\0'
 */
uint8e_t* E19x_GetName( void )
{
    return E19x_NameString;
}

/*!
 * @brief ��ȡģ�����汾
 *
 * @return 8λ�ı���
 * @note ����0x10 ����V1.0
 */
uint8e_t E19x_GetDriverVersion( void )
{
    return  EBYTE_E19_PROGRAM_TYPE;
}

/*!
 * @brief ��ȡģ��״̬
 *
 * @return ״̬����
 *        δ��ʼ��     GO_INIT          =0x00
 *        �����л�״̬ GO_BUSY          =0x01
 *        ����/����    GO_STBY          =0x02
 *        ׼�����ջ��� GO_RECEIVE       =0x03
 *        ���ڼ������� GO_WAIT_RECEIVE  =0x04
 *        ׼�����ͻ��� GO_TRANSMIT      =0x05
 *        �ȴ�������� GO_WAIT_TRANSMIT =0x06
 *        ����         GO_SLEEP         =0x07
 *        �ڲ�����     GO_ERROR         =0x08
 */
uint8e_t E19x_GetStatus( void )
{
    return ( uint8e_t )E19x_Status;;
}
