#include "ebyte_e10x.h"
#include "radio_config_434.h"

/* ����ʶ��ģ��Ƶ�� */
#if defined(EBYTE_E10_400M20S)
static uint8e_t E10x_NameString[] = "E10-400M20S";
#elif defined(EBYTE_E10_900M20S)
static uint8e_t E10x_NameString[] = "E10-900M20S";
#endif

/* ����ʶ����������汾�� */
#define EBYTE_E10_PROGRAM_TYPE 0x10

///ָ���ڲ�FIFO
#define  FIFO_READ           0x00
#define  FIFO_WRITE          0x01

///����ģʽ�л�
#define  E10X_NOCHANGE            0x00
#define  E10X_SLEEP               0x01
#define  E10X_SPI_ACTIVE          0x02
#define  E10X_READY               0x03
#define  E10X_TX_TUNE             0x05
#define  E10X_RX_TUNE             0x06
#define  E10X_TX                  0x07
#define  E10X_RX                  0x08


typedef enum 
{
    GO_INIT          =0x00,        
    GO_BUSY          =0x01, 
    GO_STBY          =0x02,
    GO_RECEIVE       =0x03,
    GO_WAIT_RECEIVE  =0x04,       
    GO_TRANSMIT      =0x05,
    GO_WAIT_TRANSMIT =0x06,
    GO_SLEEP         =0x07,
    GO_ERROR         =0x08
}E10x_Status_t;

/// ״̬��ʶ
static E10x_Status_t E10x_Status = GO_INIT;

static uint8e_t E10x_RxBuffer[64];

///WDS���ɵ������ļ�
const uint8e_t E10x_Config_WDS[] = RADIO_CONFIGURATION_DATA_ARRAY;

/*!
 * @brief �ж�ģ���Ƿ����׼��
 *
 * @return 0:���� 1:��ʱ�쳣
 * @note  ͨ��ָ��READ_CMD_BUFF(0x44)ʵ�֣�ָ����ȡCTS�ź�(0xFF)
 *        CTS��Ϊ Clear To Send,��ʾ�Ѿ�׼������Ӧ����
 *        �����ʱ: ����Ӳ�������Ƿ���ȷ
 *                  ����SPI���ͨ�������Ƿ���ȷ
 */
static uint8e_t E10x_IsReady( void )
{
    uint8e_t cts;
    uint8e_t result = 0;
    uint8e_t counter_1ms = 0;
    do
    {
        /* SPI CS ѡ�� */
        Ebyte_Port_SpiCsIoControl( 0 );
        /* 0x44ָ��  READ_CMD_BUFF */
        Ebyte_Port_SpiTransmitAndReceivce( 0x44 );
        /* ��㷢������ ������Ӧ���� ����Ӧ��ȡ��CTS(0xFF) */
        cts = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
        /* SPI CS ���� */
        Ebyte_Port_SpiCsIoControl( 1 );
        /* ��ʱ���� 1������� */
        counter_1ms++;
        Ebyte_Port_DelayMs( 1 );
    }
    while( ( cts != 0xFF ) && ( counter_1ms < 200 ) ); //��ȡ��CTS(0xFF)���� 200���볬ʱ ���˳�ѭ��
    /* ����ֵ�ж� */
    if( counter_1ms >= 200 )
    {
        /* ��ʱ�� */
        result = 1;
    }
    return result;
}

/*!
 * @brief ��ģ��д�빹����ָ��֡
 *
 * @param data  ָ��ָ������
 * @param size ָ�����鳤��
 * @return 0:���� 1:��ʱ
 */
static uint8e_t E10x_SendCommand( uint8e_t* data, uint8e_t size )
{
    uint8e_t result = 0;
    /* ���:ģ���Ƿ�׼����� */
    result = E10x_IsReady();
    if( result != 0 )
    {
        return 1;
    }
    /* SPI CS  */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* д�� */
    while( size-- )
    {
        Ebyte_Port_SpiTransmitAndReceivce( *data++ );
    }
    /* SPI CS  */
    Ebyte_Port_SpiCsIoControl( 1 );
    return result;
}

/*!
 * @brief ��ģ���ȡָ��֡��Ӧ
 *
 * @param data  ָ��洢������
 * @param size ��ȡ����
 * @return 0:���� 1:��ʱ
 */
static uint8e_t E10x_GetResponse( uint8e_t* data, uint8e_t size )
{
    uint8e_t result = 0;
    /* ���:ģ���Ƿ�׼����� */
    result = E10x_IsReady();
    if( result != 0 )
    {
        return 1;
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ָ��:READ_CMD_BUFF(0x44)  */
    Ebyte_Port_SpiTransmitAndReceivce( 0x44 );
    /* ��ȡ */
    while( size-- )
    {
        *data++ = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
    return result;
}

/*!
 * @brief ���ù�����
 *
 * @param address ���Եı���
 * @param data    ����ֵ
 * @return 0:���� 1:��ʱ
 */
static uint8e_t E10x_SetProperty( uint16e_t address, uint8e_t data )
{
    uint8e_t result = 0;
    uint8e_t cmd[5];
    /* ָ��:SET_PROPERTY(0x11) ��ʾ�������� */
    cmd[0] = 0x11;
    cmd[1] = ( uint8e_t )( address >> 8 ); //�����    GROUP
    cmd[2] = 1;                      //д�볤��  NUM_PROPS
    cmd[3] = ( uint8e_t )address;    //��ʼ��    START_PROP
    cmd[4] = data;                   //д��ֵ
    /* д�� */
    result = E10x_SendCommand( cmd, 5 );
    return result;
}


/*!
 * @brief �л�ģ��Ĺ���ģʽ
 *
 * @param state ����ģʽ����  ����ο�EZRadio_RevC2A API ��CHANGE_STATE�½�
 *        @arg  0:E10X_NOCHANGE
 *        @arg  1:E10X_SLEEP
 *        @arg  2:E10X_SPI_ACTIVE
 *        @arg  3:E10X_READY
 *
 *        @arg  5:E10X_TX_TUNE
 *        @arg  6:E10X_RX_TUNE
 *        @arg  7:E10X_TX
 *        @arg  8:E10X_RX
 *
 * @return 0:���� 1:�л�ģʽʧ��
 */
static uint8e_t E10x_SetWorkMode( uint8e_t state )
{
    uint8e_t cmd[3];
    uint8e_t i;
    uint8e_t result = 0;
    /* ָ��:CHANGE_STATE(0x34) �л�����ģʽ */
    cmd[0] = 0x34;
    cmd[1] = state; //ģʽ����
    /* д�� */
    result = E10x_SendCommand( cmd, 2 );
    if( result != 0 )
    {
        return 1;
    }
    /* ѭ��ȷ��ģʽ��� */
    for( i = 0 ; i < 5; i++ )
    {
        /* ָ��:REQUEST_DEVICE_STATE(0x33) ����ǰ�豸״̬��ͨ�� */
        cmd[0] = 0x33;
        /* д�� */
        E10x_SendCommand( cmd, 1 );
        /* ��ȡָ����Ӧ */
        E10x_GetResponse( cmd, 3 );
        /* �ȶ�ģʽ���� */
        if( ( cmd[1] & 0x0F ) == state )
        {
            break;    //�˳�ѭ��
        }
        Ebyte_Port_DelayMs( 1 );
    }
    /* ����ֵ���� */
    if( i >= 5 )
    {
        /* ģʽ���ʧ�� */
        result = 1;
    }
    return result;
}

/*!
 * @brief ���FIFO����
 * @param mode ģʽ
 *           @arg FIFO_WRITE ��ָ������FIFO
 *           @arg FIFO_READ  ��ָ������FIFO
 *
 * @return 0:���� 1:��ʱ
 */
static uint8e_t E10x_ClearFIFO( uint8e_t mode )
{
    uint8e_t cmd[2];
    uint8e_t result;
    if( mode == FIFO_WRITE )
    {
        /* ָ��:FIFO_INFO(0x15) �鿴FIFO����������FIFO */
        cmd[0] = 0x15;
        cmd[1] = 0x01;//���÷���FIFO
        result = E10x_SendCommand( cmd, 2 );
        if( result != 0 )
        {
            return 1;
        }
    }
    else
    {
        /* ָ��:FIFO_INFO(0x15) �鿴FIFO����������FIFO */
        cmd[0] = 0x15;
        cmd[1] = 0x02;//���ý���FIFO
        result = E10x_SendCommand( cmd, 2 );
        if( result != 0 )
        {
            return 1;
        }
    }
    return result;
}

/*!
 * @brief ������������д���ڲ����ݶ���
 *
 * @param data ָ��д������
 * @param size ���ݳ���
 *
 * @note Ĭ�Ͻ���/����FIFO���Զ�����д��������󳤶�Ϊ64�ֽ� (���ȿ���ռ��һλ)
 *       ���GLOBAL_CONFIG:FIFO_MODE�����ã���ôFIFO�ϲ��ҹ���д��������󳤶�Ϊ129�ֽ�
 */
static void E10x_SetFIFO( uint8e_t* data, uint8e_t size )
{
    /* ���:ģ���Ƿ�׼����� */
    E10x_IsReady();
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ָ��:WRITE_TX_FIFO(0x66) дFIFO */
    Ebyte_Port_SpiTransmitAndReceivce( 0x66 );
    /* ʹ�������ݰ��䳤ģʽ ��һ�ֽ���д�����ݳ��� */
    Ebyte_Port_SpiTransmitAndReceivce( size );
    /* ѭ��д������ */
    while( size -- )
    {
        Ebyte_Port_SpiTransmitAndReceivce( *data++ );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief ���ڲ����ݶ��ж�ȡ���յ�����
 *
 * @param data ָ��洢������
 * @param size ָ���ȡ�ĳ���
 *
 * @note Ĭ�Ͻ���/����FIFO���Զ�������ȡ������󳤶�Ϊ64�ֽ� (���ȿ���ռ��һλ)
 *       ���GLOBAL_CONFIG:FIFO_MODE�����ã���ôFIFO�ϲ��ҹ�����ȡ������󳤶�Ϊ129�ֽ�
 */
static uint8e_t E10x_GetFIFO( uint8e_t* data, uint8e_t* size )
{
    uint8e_t length;
    uint8e_t result ;
    /* ���ģ���Ƿ�׼����� */
    result = E10x_IsReady();
    if( result != 0 )
    {
        return 1;
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ָ��:READ_RX_FIFO(0x77) ��FIFO  */
    Ebyte_Port_SpiTransmitAndReceivce( 0x77 );
    /* �ɱ䳤���ݰ� �������ݵ�һ�ֽ�Ϊ������ */
    length = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    *size = length;
    /* ѭ����ȡ */
    while( length-- )
    {
        *data++ = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
    return result;
}

/*!
 * @brief ��ȡ�ڲ��жϱ�ʶ
 *
 * @return �ж����ͱ���
 *        @arg 0x80:FILTER_MATCH_PEND
 *        @arg 0x40:FILTER_MISS_PEND
 *        @arg 0x20:PACKET_SENT_PEND   �������
 *        @arg 0x10:PACKET_RX_PEND     �������
 *        @arg 0x08:CRC_ERROR_PEND     CRCУ�����
 *        @arg 0x04:ALT_CRC_ERROR_PEND
 *        @arg 0x02:FIFO_ALMOST_EMPTY_PEND
 *        @arg 0x01:FIFO_ALMOST_FULL_PEND
 *
 */
uint8e_t E10x_GetIRQ( void )
{
    uint8e_t buffer[10];
    uint8e_t cmd[4];
    /* ָ��:GET_INT_STATUS(0x20) ��ȡ�ж�״ָ̬�� */
    cmd[0] = 0x20;
    cmd[1] = 0;  //PH_CLR_PEND    ��������
    cmd[2] = 0;  //MODEM_CLR_PEND ��������
    cmd[3] = 0;  //CHIP_CLR_PEND  ��������
    /* д�� */
    E10x_SendCommand( cmd, 4 );
    /* ����ָ����Ӧ */
    E10x_GetResponse( buffer, 9 );
    /* ����ֻ��ȡ�� PH_PEND �ֽ��е��ж�״̬
       ���Բ��� EZRadio_revC2A_API ��GET_INT_STATUS�½� ��ȡ�����ж�״̬*/
    return  buffer[3];
}

/*!
 * @brief �ж�ģ���Ƿ����
 *
 * @return 0:���� 1:�쳣
 */
static uint8e_t E10x_IsExist( void )
{
    uint8e_t cmd[4];
    static uint8e_t buffer[10];    
    /* ָ��:PART_INFO(0x01) ��ȡӲ��������Ϣ */
    cmd[0] = 0x01;
    /* д�� */
    E10x_SendCommand( cmd, 1 );
    /* ����ָ����Ӧ */
    E10x_GetResponse( buffer, 9 ); 
    /* ��������� ���������е�PART��Ϣ��0x4438  */
    if( (buffer[2] == 0x44) && (buffer[3] == 0x38) ) return 0;//��������
    
    return 1;
}

/*!
 * @brief ��λ
 * @note ģ���ڲ������õĲ����ᶪʧ
 */
void E10x_Reset( void )
{
    /* SDN�����ø�����ģ�鸴λ */
    Ebyte_Port_SdnIoControl( 1 );
    /* ��ʱ */
    Ebyte_Port_DelayMs( 20 );
    /* SDN �ָ� */
    Ebyte_Port_SdnIoControl( 0 );
    /* CS �ָ�*/
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief ����ģ�����
 * @note ��Ҫ��ԭ��������� WDS ����
 *       ע��E10ϵ��Ϊ26MHz����
 *       ����������������� ֱ���滻radio_config.h ����
 */
static void E10x_Config( void )
{
    uint8e_t i;
    uint16e_t j = 0;
    /* ����WDS�Զ����ɵĲ�����  */
    const uint8e_t*  ptr = E10x_Config_WDS;
    /* д��WDS������ */
    while( ( i = *( ptr + j ) ) != 0 )
    {
        j += 1;
        E10x_SendCommand( ( uint8e_t* )ptr + j, i );
        j += i;
    }
    /* �������� �ɱ䳤���ݰ� */
    E10x_SetProperty( 0x1208, 0x2A ); //���ģʽ 1Bbyte������ֵ�����FIFO�� ��2�������ݰ����ȿɱ�
    E10x_SetProperty( 0x1209, 0x01 ); //���ݰ����ȷ��ڵ�1��
    E10x_SetProperty( 0x120D, 0x00 ); //��һ���򳤶� ���ֽ�
    E10x_SetProperty( 0x120E, 0x01 ); //��һ���򳤶� ���ֽ� 0x01:����Ϊһ���ֽ�
    E10x_SetProperty( 0x1211, 0x00 );
    E10x_SetProperty( 0x1212, 0x10 );
    /* �����������
     * ���뷶Χ:0x00��0x7F   Ĭ��ֵ:0x7F
     * ע��;���ʲ�������������
     * ���ӽ����ֵʱ����������С��0.1db
     * ������ƫ��ʱ���������ܴ���0.1db  */
    E10x_SetProperty( 0x2201, 0x7F );
}

/*!
 * @brief ��ʼ��������
 *
 * @return 0:���� 1:�쳣
 */
uint8e_t E10x_GoReceive( void )
{
    uint8e_t result = 0;
    uint8e_t cmd[8];
    /* FIFO ��λ */
    result = E10x_ClearFIFO( FIFO_READ );
    if( result != 0 )
    {
        return 1;
    }
    E10x_ClearFIFO( FIFO_WRITE );
    /* �л�ģʽ:  SPI_ACTIVE
     * ��ο�  datasheet �еĹ���ģʽ�л�����ͼ
     * ����ģʽ����Ϊ: Sleep��Stby->SPI active->Ready->RxTune->Rx */
    result = E10x_SetWorkMode( E10X_SPI_ACTIVE );
    if( result != 0 )
    {
        return 1;
    }
    /* �л�ģʽ:  RX_TUNE  ����Ԥ��״̬ */
    result = E10x_SetWorkMode( E10X_RX_TUNE );
    if( result != 0 )
    {
        return 1;
    }
    /* ��ʼ����(����) */
    cmd[0] = 0x32;      //ָ��:START_RX (0x32) ����Rx����ģʽ��ʼ����Ƶ������
    cmd[1] = 0;         //�ŵ�����:   ������Ƶ���л�
    cmd[2] = 0;         //��������:   ��������ʱ�����������̬�����ȴ����Ѷ�ʱ��
    cmd[3] = 0;         //���ճ���:   ��Byte
    cmd[4] = 0;         //���ճ���:   ��Byte
    cmd[5] = E10X_RX;   //���ճ�ʱ��: �л�Ϊ����״̬
    cmd[6] = E10X_RX;   //���ճɹ���: �л�Ϊ����״̬
    cmd[7] = E10X_RX;   //����ʧ�ܺ�: �л�Ϊ����״̬
    E10x_SendCommand( cmd, 8 );
    
    /* ״̬��¼ */
    E10x_Status = GO_WAIT_RECEIVE;
    
    return result;
}

/*!
 * @brief ��ʼ��������
 *
 * @param data ָ��������
 * @param size �������ݳ���
 */
uint8e_t E10x_GoTransmit( uint8e_t* data, uint8e_t size )
{
    uint8e_t cmd[6];
    uint8e_t tx_size = size + 1;//�ɱ䳤���ݰ�������1���ȱ�ʾλ
    uint8e_t result = 1 ;
    /* ��ֹ�������� */
    Ebyte_Port_DelayMs( 5 );
    /* FIFO ��λ */
    result = E10x_ClearFIFO( FIFO_READ );
    if( result != 0 )
    {
        return 1;
    }
    E10x_ClearFIFO( FIFO_WRITE );
    /* ģʽ�л�: E10X_SPI_ACTIVE ģʽ
     * ��ο�  datasheet �еĹ���ģʽ�л�����ͼ
     * ����ģʽ����Ϊ: Sleep��Stby->SPI active->Ready->TxTune->Tx */
    result = E10x_SetWorkMode( E10X_SPI_ACTIVE );
    if( result != 0 )
    {
        return 1;
    }
    /* FIFO д����������� */
    E10x_SetFIFO( data, size );
    /* ģʽ�л�: E10X_TX_TUNE ����Ԥ��ģʽ */
    result = E10x_SetWorkMode( E10X_TX_TUNE );
    if( result != 0 )
    {
        return 1;
    }
    /* ��ʼ���� */
    cmd[0] = 0x31;         //ָ��:START_TX(0x31) �л�ΪTX����ģʽ ��ʼ����FIFO�е�����
    cmd[1] = 0;            //�ŵ�����:   ������Ƶ���л�
    cmd[2] = 0;            //��������:   ����ʱ������ʼ���ͣ����ȴ����Ѷ�ʱ���������Ҫ������ɺ��Զ���������ģʽ �������߿�������Ϊ 0x10
    cmd[3] = tx_size >> 8; //���ͳ���:   ���ֽ�
    cmd[4] = tx_size;      //���ͳ���:   ���ֽ�
    cmd[5] = 128;          //������ʱ128us
    E10x_SendCommand( cmd, 6 );
    
    /* ״̬��¼ */
    E10x_Status = GO_WAIT_TRANSMIT;
    
    /* �������ͷ�ʽ �ڴ˵ȴ�������� */
    uint8e_t irqStatus = 0;
    do
    {
        irqStatus = E10x_GetIRQ();
    }
    while( !( irqStatus & 0x20 ) );
    
    /* ״̬��¼ */
    E10x_Status = GO_STBY;
    
    /* �ص��û����� */
    Ebyte_Port_TransmitCallback( 0x0001 );
    return 0 ;
}

/*!
 * @brief ��������ģʽ
 */
uint8e_t E10x_GoSleep( void )
{
    uint8e_t result = E10x_SetWorkMode( E10X_SLEEP );
    if( result != 0 )
    {
        return 1;
    }
    return 0;
}

/*!
 * @brief ģ���ʼ��
 *
 * @return 0:���� 1:��ʼ��ʧ��
 */
uint8e_t E10x_Init( void )
{
    uint8e_t result;
    /* ��λ */
    E10x_Reset();
    /* ���: SPIͨ���Ƿ�����
     * ���ʧ��: ���鸴λ����SDN ע��SDN�Ǹߵ�ƽ����λ
     *           ����SPIͨ������Ӳ�������Ƿ�����
     *           ����SPIͨ����������Ƿ���ȷ ע��CPOL=0 CPHA=0  SPI_CS�ر�Ӳ���Զ�����
     */
    result = E10x_IsReady();
    if( result != 0 )
    {
        return 1;
    }
    /* ���: SPI��ȡID ���ʧ�� ����Ӳ�� */
    result = E10x_IsExist();
    if( result != 0 ) return 1;     
    /* �������� */
    E10x_Config();
    /* ��ʼ�������� */
    E10x_GoReceive();
    return result;
}

/*!
 * @brief ���ڵ��ú��� ��������������ݽ���
 *
 * @return 0
 * @note ��Ҫ��������ѭ�����ã���Ϊͨ����ѯ�ڲ���ʶλ���ж��Ƿ������ݵ���
 *       ���������жϣ����ж��е��ñ���������
 */
uint8e_t E10x_TaskForPoll( void )
{
    uint8e_t irqStatus ;
    /* ��ȡ�ڲ��жϱ�ʶ */
    irqStatus = E10x_GetIRQ();
    /* 0x10��bit4 ��ʾ���ݽ�������ж���λ */
    if( irqStatus & 0x10 )
    {
        uint8e_t recvSize = 0;
        /* ��ȡ���� */
        E10x_GetFIFO( E10x_RxBuffer, &recvSize );
        
        /* ״̬��¼ */
        E10x_Status = GO_STBY;
        
        /* �ص��û����� */
        Ebyte_Port_ReceiveCallback( 0x0002, E10x_RxBuffer, recvSize );
    }
    return 0;
}

/*!
 * @brief ����
 */
void E10x_TaskForIRQ( void )
{
  
}

/*!
 * @brief ��ȡģ������
 *
 * @return ָ�������ַ�����ָ��
 * @note ��׼�ַ��� ĩβ���н����� '\0'
 */
uint8e_t* E10x_GetName( void )
{
    return E10x_NameString;
}

/* !
 * @brief ��ȡģ�����汾
 *
 * @return 8λ�ı���
 * @note ����0x10 ����V1.0
 */
uint8e_t E10x_GetDriverVersion( void )
{
    return  EBYTE_E10_PROGRAM_TYPE;
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
uint8e_t E10x_GetStatus(void)
{
  return (uint8e_t)E10x_Status;;
}