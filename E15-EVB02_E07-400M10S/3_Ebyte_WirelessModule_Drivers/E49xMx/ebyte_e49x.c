#include "ebyte_e49x.h"

/* ����ʶ����������汾�� */
#define EBYTE_E49_PROGRAM_TYPE 0x10 

/// ����ʶ��ģ��
#if defined(EBYTE_E49_400M20S)
static uint8e_t E49x_NameString[] = "E49-400M20S";
#elif defined(EBYTE_E49_900M20S)
static uint8e_t E49x_NameString[] = "E49-900M20S";
#endif 

#define  GPIO1_DOUT          0x00
#define  GPIO1_DIN           0x00
#define  GPIO1_IRQ1          0x01
#define  GPIO1_IRQ2          0x02
#define  GPIO1_DCLK          0x03

#define  GPIO2_IRQ1          0x00
#define  GPIO2_IRQ2          0x04
#define  GPIO2_DOUT          0x08
#define  GPIO2_DIN           0x08
#define  GPIO2_DCLK          0x0C

#define  GPIO3_CLKO          0x00
#define  GPIO3_DOUT          0x10
#define  GPIO3_DIN           0x10
#define  GPIO3_IRQ2          0x20
#define  GPIO3_DCLK          0x30

#define  FIFO_READ           0x00
#define  FIFO_WRITE          0x01   

#define  IRQ_RX_ACTIVE       0x00
#define  IRQ_TX_ACTIVE       0x01
#define  IRQ_RSSI_VLD        0x02
#define  IRQ_PREAM_OK        0x03
#define  IRQ_SYNC_OK         0x04
#define  IRQ_NODE_OK         0x05
#define  IRQ_CRC_OK          0x06
#define  IRQ_PKT_OK          0x07
#define  IRQ_SL_TMO          0x08
#define  IRQ_RX_TMO          0x09
#define  IRQ_TX_DONE         0x0A
#define  IRQ_RX_FIFO_NMTY    0x0B
#define  IRQ_RX_FIFO_TH      0x0C
#define  IRQ_RX_FIFO_FULL    0x0D
#define  IRQ_RX_FIFO_WBYTE   0x0E
#define  IRQ_RX_FIFO_OVF     0x0F
#define  IRQ_TX_FIFO_NMTY    0x10
#define  IRQ_TX_FIFO_TH      0x11
#define  IRQ_TX_FIFO_FULL    0x12
#define  IRQ_STATE_IS_STBY   0x13
#define  IRQ_STATE_IS_FS     0x14
#define  IRQ_STATE_IS_RX     0x15
#define  IRQ_STATE_IS_TX     0x16
#define  IRQ_LED             0x17
#define  IRQ_TRX_ACTIVE      0x18
#define  IRQ_PKT_DONE        0x19

#define  SL_TMO_IRQ_EN          0x80
#define  RX_TMO_IRQ_EN          0x40
#define  TX_DONE_IRQ_EN         0x20
#define  PREAM_OK_IRQ_EN        0x10
#define  SYNC_OK_IRQ_EN         0x08
#define  NODE_OK_IRQ_EN         0x04
#define  CRC_OK_IRQ_EN          0x02
#define  PKT_DONE_IRQ_EN        0x01

#define GET_IRQ_SL_TMO   0x2000
#define GET_IRQ_RX_TMO   0x1000
#define GET_IRQ_TX_DONE  0x0800
#define GET_IRQ_LBD      0x0080
#define GET_IRQ_COL_ERR  0x0040
#define GET_IRQ_PKT_ERR  0x0020
#define GET_IRQ_PREAM_OK 0x0010
#define GET_IRQ_SYNC_OK  0x0008
#define GET_IRQ_NODE_OK  0x0004
#define GET_IRQ_CRC_OK   0x0002
#define GET_IRQ_PKT_OK   0x0001

typedef enum 
{
    GO_INIT          =0x00,        
    GO_BUSY,          
    GO_STBY,          
    GO_RECEIVE,       
    GO_WAIT_RECEIVE,          
    GO_TRANSMIT,      
    GO_WAIT_TRANSMIT, 
    GO_SLEEP,         
    GO_ERROR         
}E49x_Status_t;

static E49x_Status_t E49x_Status = GO_INIT;
static uint8e_t E49x_ReceiveBuffer[64] = {0};
static uint8e_t E49x_IRQ_Trigger = 0;
/* * 
 * ��׼���� 
 * ��ʼƵ��:850MHz
 * Data Rate:2.4kbps
 * Deviation:4.8kHz
 * Bandwidth:Auto
 */
uint8e_t E49x_Config_CMT[12] = {
    0x00,0x66,0xEC,0x1D,0xF0,0x80,0x14,0x08,0x11,0x02,0x02,0x00
};
uint8e_t E49x_Config_System[12] = {
    0xAE,0xE0,0x35,0x00,0x00,0xF4,0x10,0xE2,0x42,0x20,0x00,0x81    
};
uint8e_t E49x_Config_Frequency[8] = {
    0x41,0x6D,0x80,0x86,0x41,0x62,0x27,0x16  
};
uint8e_t E49x_Config_DataRate[24] = {
    0x32,0x18,0x10,0x99,0xC1,0x9B,0x06,0x0A,0x9F,0x39,0x29,0x29,0xC0,0x51,0x2A,0x53,0x00,0x00,0xB4,0x00,0x00,0x01,0x00,0x00   
};
uint8e_t E49x_Config_Baseband[29] = {
    0x12,0x08,0x00,0xAA,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0xD4,0x2D,0x01,0x1F,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x60,0xFF,0x00,0x00,0x1F,0x10   
};
uint8e_t E49x_Config_TX[11] = {
    0x50,0x83,0x01,0x00,0x42,0xB0,0x00,0x8A,0x18,0x3F,0x7F
};

/* !
 * @brief IOģ���˫��SPIʱ������  ������ʱ
 * 
 * @param time ����
 */
static void E49x_SpiDelay(uint8e_t time)
{
    uint8e_t n = time;
    while(n--);
}

/* !
 * @brief IOģ���˫��SPI 1Byte=8Bit ��λ��� 
 * 
 * @param data �������
 * @note ʱ����ο��ĵ� AN142��2�� SPIд�Ĵ���ʱ��
 */
static void E49x_HalfSpiTransmit( uint8e_t data )
{
    uint8e_t i;
  
    /* ���� 1Byte=8bit��λ��� */
    for( i=0 ; i<8 ; i++)
    {
        /* SLCK=0 ��ΪCPOL=0 ����ʱ����Ϊ�� */
        Ebyte_Port_SlckIoControl(0);
        
        /* ����λ��0x80 ��ΪMSB��ʽ��� */
        if( data & 0x80 )
        {
            Ebyte_Port_SdioIoControl(1);
        }
        else
        {
            Ebyte_Port_SdioIoControl(0);
        }
        
        E49x_SpiDelay(7);
        
        /* ����λ���� */
        data <<=1;
        
        /* SLCK= 1�������� ��ΪCPOA=0 ��һ��������в��� */
        Ebyte_Port_SlckIoControl(1);
        
        E49x_SpiDelay(7);
    }  
}

/* !
 * @brief IOģ���˫��SPI ��λ��ȡ 8Bit����λ 
 * 
 * @return ��ȡ������ 1Byte
 * @note ʱ����ο��ĵ� AN142��2�� SPI���Ĵ���ʱ��
 */
static uint8e_t E49x_HalfSpiReceive( void )
{
    uint8e_t i;  
    uint8e_t data = 0xFF;
    
    for( i=0 ; i<8; i++ )
    {
        /* SLCK=0 ��ΪCPOL=0 ����ʱ����Ϊ�� */
        Ebyte_Port_SlckIoControl(0);
        
        E49x_SpiDelay(7);
        
        data <<= 1;
        
        /* SLCK=1 �����ӻ����Bit */
        Ebyte_Port_SlckIoControl(1);
        
        /* ��ȡSDIO ��¼Bit */
        if( Ebyte_Port_SdioIoControl(4))
        {
            data |= 0x01;
        }
        else
        {
            data &= ~0x01;
        }
        
        E49x_SpiDelay(7);
    }
    
    return data;
}

/* !
 * @brief ͨ����ַ���ڲ��Ĵ���д������
 * 
 * @param address �Ĵ�����ַ 
 * @param data ���� 
 * @note  2�߰�˫��SPI+2��CSƬѡͨ�Žӿڣ����Ǳ�׼4��ȫ˫��SPI�ӿ�
 */
static void E49x_SetRegister( uint8e_t address, uint8e_t data )
{

    /* SDIO ����*/
    Ebyte_Port_SdioIoControl(3);//����SDIOΪ���ģʽ
    Ebyte_Port_SdioIoControl(1);//����SDIO����ߵ�ƽ  
    
    /* SLCK ���� */
    Ebyte_Port_SlckIoControl(0);
    
    /* CS   ���� */
    Ebyte_Port_FcsbIoControl(1);
    Ebyte_Port_CsbIoControl(0); //CS��ѡһ CSB����͵�ƽ��ʾѡ��Ĵ���
    
    /* ���ٵȴ���� SLCK���� */
    E49x_SpiDelay(20);
    
    /* д��ַ */
    E49x_HalfSpiTransmit( address & 0x7F );//��ַ���⴦�� ��0x7F ��r/wλ=0
    
    /* д���� */
    E49x_HalfSpiTransmit( data ); 
    
    /* �ͷ����� */
    Ebyte_Port_SlckIoControl(0);
    E49x_SpiDelay(20);
    Ebyte_Port_CsbIoControl(1);
    
    /* SDIO */
    Ebyte_Port_SdioIoControl(2); //Ĭ���л�Ϊ���� 
}

/* !
 * @brief ͨ����ַ���ڲ��Ĵ�����ȡ����
 * 
 * @param address �Ĵ�����ַ 
 * @return data ���� 1Byte 
 * @note  2�߰�˫��SPI+2��CSƬѡͨ�Žӿڣ����Ǳ�׼4��ȫ˫��SPI�ӿ�
 *        ʱ����ο��ĵ� AN142��2��
 */
static uint8e_t E49x_GetRegister( uint8e_t address )
{
    uint8e_t result = 0xFF;

    /* SDIO ���� */
    Ebyte_Port_SdioIoControl(3);//����SDIOΪ���ģʽ
    Ebyte_Port_SdioIoControl(1);//����SDIO����ߵ�ƽ  

    /* SLCK ���� */
    Ebyte_Port_SlckIoControl(0);
    
    /* CS   ���� */
    Ebyte_Port_FcsbIoControl(1);
    Ebyte_Port_CsbIoControl(0); //CS��ѡһ CSB����͵�ƽ��ʾѡ��Ĵ���
    
    /* ���ٵȴ���� SLCK���� */
    E49x_SpiDelay(20);
    
    /* д��ַ */
    E49x_HalfSpiTransmit( address|0x80 ); //��ַ���⴦�� ��0x80 ��r/wλ=1
    
    /* SDIO �л�Ϊ����  */
    Ebyte_Port_SdioIoControl(2);
    
    /* ������ */
    result = E49x_HalfSpiReceive();
    
    /* �ͷ����� */
    Ebyte_Port_SlckIoControl(0);   
    E49x_SpiDelay(20);   
    Ebyte_Port_CsbIoControl(1);
    
    return result;
}

/* !
 * @brief д�ڲ����ݻ����� FIFO
 * 
 * @param data ָ���д�������׵�ַ��ָ��
 * @param size д�����ݳ���
 * @note һ��֧��32�ֽڡ���������˺ϲ����ͺ�����Ķ���FIFO����֧��64�ֽ�
 */
static void E49x_SetFIFO( uint8e_t *data , uint16e_t size )
{
    uint8e_t i;
  
    /* CS ���Ÿ�λ */
    Ebyte_Port_FcsbIoControl(1);
    Ebyte_Port_CsbIoControl(1);
    
    /* SLCK ���� */
    Ebyte_Port_SlckIoControl(0);
    
    /* SDIO �л�Ϊ���  */
    Ebyte_Port_SdioIoControl(3);
    
    /* ѭ��д��  */
    for( i=0 ; i<size ; i++)
    {
        /* CS��ѡһ FCSB����͵�ƽ��ʾѡ���ڲ�FIFO */
        Ebyte_Port_FcsbIoControl(0); 
        
        E49x_SpiDelay(20);
        
        /* д���� */
        E49x_HalfSpiTransmit( data[i] );
        
        Ebyte_Port_SlckIoControl(0);
        
        E49x_SpiDelay(30);
        
        Ebyte_Port_FcsbIoControl(1);
        
        E49x_SpiDelay(60);
    }
    
    Ebyte_Port_SlckIoControl(0);
}

/* !
 * @brief ���ڲ�FIFO
 * 
 * @param data ָ���������׵�ַ��ָ��
 * @param size ��ȡ�����ݳ���
 * @return 0:���� 1:FIFO�����ݵ����ȱ�ʾλ����ȷ
 * @note һ��FIFO���Ϊ32�ֽ�(��1�ֽڱ����ȱ�ʾλռ��)����������˺ϲ����ͺ�����Ķ���FIFO����֧��64�ֽ�
 */
static uint8e_t E49x_GetFIFO( uint8e_t *data , uint8e_t *size )
{
    uint8e_t i,length;
    
    /* CS ���Ÿ�λ */
    Ebyte_Port_FcsbIoControl(1);
    Ebyte_Port_CsbIoControl(1);
    
    /* SLCK ���� */
    Ebyte_Port_SlckIoControl(0);
  
    /* SDIO �л�Ϊ����  */
    Ebyte_Port_SdioIoControl(2);
    
    /* �ȶ�ȡ��һ�ֽڳ��ȱ�ʾλ 
       ��Ϊ�����˿ɱ䳤�����壬��λ��E49���ͷ�Ӳ���Զ�����  
    */
    Ebyte_Port_FcsbIoControl(0);   
    E49x_SpiDelay(20);    
    length = E49x_HalfSpiReceive();    
    Ebyte_Port_SlckIoControl(0);    
    E49x_SpiDelay(30);    
    Ebyte_Port_FcsbIoControl(1);    
    E49x_SpiDelay(60);    
    
    /* �жϳ��ȱ�ʾ�Ƿ���ȷ */
    if( length > 64 ) return 1;
    length -= 1;//ȥ��1���ȱ�ʾλ �������ݳ���
    
    /* ѭ����ȡ  */
    for( i=0 ; i<length ; i++)
    {
        /* CS��ѡһ FCSB����͵�ƽ��ʾѡ���ڲ�FIFO */
        Ebyte_Port_FcsbIoControl(0);
        
        E49x_SpiDelay(20);
        
        /* ������ */
        data[i] = E49x_HalfSpiReceive();
        
        Ebyte_Port_SlckIoControl(0);
        
        E49x_SpiDelay(30);
        
        Ebyte_Port_FcsbIoControl(1);
        
        E49x_SpiDelay(60);
    } 
    
    /* ��䳤�� */
    *size = length;
    
    return 0;
}

/* !
 * @brief ���FIFO����
 * @param mode ģʽ
 *           @arg FIFO_WRITE ��дģʽ
 *           @arg FIFO_READ  ����ģʽ
 *
 */
static void E49x_ClearFIFO( uint8e_t mode)
{
    if( mode == FIFO_WRITE )
    {    
        /* �������FIFO �Ĵ�����ַ:0x6C bit0:0x01  */
        E49x_SetRegister(0x6C, 0x01);//bit0  0����Ч��1������ TX FIFO �����λ��� 1 ֮�����轫������� 0�� �ڲ����Զ����Ϊ0
    }
    else
    {
        /* �������FIFO �Ĵ�����ַ:0x6C bit1:0x02 */
        E49x_SetRegister(0x6C, 0x02);//bit1  0����Ч��1������ RX FIFO �����λ��� 1 ֮�����轫������� 0�� �ڲ����Զ����Ϊ0��     
    } 
}

/* !
 * @brief ����FIFO�������дģʽ 
 * @param mode ģʽ
 *           @arg FIFO_WRITE ��дģʽ
 *           @arg FIFO_READ  ����ģʽ
 *
 */
static void E49x_GoFIFO( uint8e_t mode)
{
    uint8e_t config;
    
    /* �Ĵ�����ַ:0x69  bit2:0x04  bit0:0x01 */
    config = E49x_GetRegister(0x69);
    
    if( mode == FIFO_WRITE )
    {    
        config |= 0x04;//bit2 ����Ϊһ�� 64-byte FIFO ʱ��ʹ�á�0������ RX FIFO��1������ TX FIFO��
        config |= 0x01;//bit0 �����ڷ��� FIFO ǰ���úá�0��SPI �Ĳ����Ƕ� FIFO��1��SPI �Ĳ�����д FIFO��
    }
    else
    {
        config &= ~0x04;//bit2 
        config &= ~0x01;//bit0
    }
    
    E49x_SetRegister(0x69, config);  
}

/* !
 * @brief ��λ 
 * @note ģ���ڲ������õĲ����ᶪʧ
 */
void E49x_Reset(void)
{
    /* ���ָ�λ */
    E49x_SetRegister(0x7F, 0xFF);
}

/* !
 * @brief д�Ĵ��� ����Ϊ����ģʽ 
 *
 * @return 0:���� 1:�쳣
 * @note ����ģʽ�²������üĴ���
 */
static uint8e_t E49x_SetStby(void)
{
    uint8e_t retry  =10;
    uint8e_t result =1 ;
    
    
    /* ״̬��:׼��״̬�л� */
    E49x_Status = GO_BUSY;
    
    /* дָ��������ģʽ �Ĵ�����ַ:0x60  bit1:0x02 */
    E49x_SetRegister(0x60,0x02);
    

    while( retry > 0)
    {
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x61 bit3��bit0=0x02 */
        result = E49x_GetRegister(0x61) & 0x0F;
        if( result == 0x02 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_STBY;
            
            /* �������� */
            result = 0;
            break;
        }    
        
        Ebyte_Port_DelayUs(500);
        retry--;
    }
    
    /* ������ */
    if( result!=0 ) 
    {
        /* ״̬��:ģ���쳣 */
        E49x_Status = GO_ERROR;        
    }
    
    return result;
}



/* !
 * @brief д�Ĵ��� ����Ϊ����ģʽ 
 *
 * @return 0:���� 1:�쳣
 */
static uint8e_t E49x_SetReceive(void)
{
    uint8e_t retry  =10;
    uint8e_t result =1 ;
    
    /* ״̬��:׼��״̬�л� */
    E49x_Status = GO_BUSY;    
    
    /* дָ��������ģʽ �Ĵ�����ַ:0x60  bit3:0x08 */
    E49x_SetRegister(0x60,0x08);
    

    while( retry > 0)
    {
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x61 bit3��bit0=0x05 */
        result = E49x_GetRegister(0x61) & 0x0F;
        if( result == 0x05 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_RECEIVE;
            
            /* �������� */
            result = 0;
            break;
        }    
        
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x6D ��1 bitΪ��Чλ*/
        result = E49x_GetRegister(0x6D) & 0x01;
        if( result == 0x01 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_RECEIVE;
            
            /* �������� */
            result = 0;
            break;
        }
        
        Ebyte_Port_DelayUs(500);
        retry--;
    }
    

    /* ������ */
    if( result!=0 ) 
    {
        /* ״̬��:ģ���쳣 */
        E49x_Status = GO_ERROR;        
    }
    
    return result;
}

/* !
 * @brief д�Ĵ��� ����Ϊ����ģʽ 
 *
 * @return 0:���� 1:�쳣
 * @note �����Զ��Ķ�FIFO�е����ݽ������ߴ���
 */
static uint8e_t E49x_SetTransmit(void)
{
    uint8e_t retry  =10;
    uint8e_t result =1 ;
    
    /* ״̬��:׼��״̬�л� */
    E49x_Status = GO_BUSY;
    
    /* дָ����뷢��ģʽ �Ĵ�����ַ:0x60  bit6:0x40 */
    E49x_SetRegister(0x60,0x40);
    
    while( retry > 0)
    {
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x61 bit3��bit0=0x06*/
        result = E49x_GetRegister(0x61) & 0x0F;
        if( result == 0x06 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_TRANSMIT;
            
            /* �������� */
            result = 0;
            break;
        }    
        
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x6A  bit3=0x08 */
        result = E49x_GetRegister(0x6A) & 0x08;
        if( result == 0x08 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_TRANSMIT;
            
            /* �������� */
            result = 0;
            break;
        }  

        Ebyte_Port_DelayUs(500); 
        retry--;
    }
    
    /* ������ */
    if( result!=0 ) 
    {
        /* ״̬��:ģ���쳣 */
        E49x_Status = GO_ERROR;        
    }
    
    return result;
}

/* !
 * @brief д�Ĵ��� ����Ϊ����ģʽ 
 *
 * @return 0:���� 1:�쳣
 */
static uint8e_t E49x_SetSleep(void)
{
    uint8e_t retry  =10;
    uint8e_t result =1 ;
    
    /* ״̬��:׼��״̬�л� */
    E49x_Status = GO_BUSY;
    
    /* дָ���������ģʽ �Ĵ�����ַ:0x60  bit4:0x10 */
    E49x_SetRegister(0x60,0x10);
    
    while( retry > 0)
    {
        Ebyte_Port_DelayUs(100);
        
        /* ��ȡ�Ĵ�������ȷ�� �Ĵ�����ַ:0x61  bi3��bit1=0x01*/
        result = E49x_GetRegister(0x61) & 0x0F;
        if( result == 0x01 )
        {
            /* ״̬�����¼ */
            E49x_Status = GO_SLEEP;
            
            /* �������� */
            result = 0;
            break;
        }    
        
        Ebyte_Port_DelayUs(500);
        retry--;
    }
    
    /* ������ */
    if( result!=0 ) 
    {
        /* ״̬��:ģ���쳣 */
        E49x_Status = GO_ERROR;        
    }
    
    return result;
}


/* !
 * @brief ����ʵ�ʷ�����Ϣ��ĳ���
 * 
 * @param size �����ĳ���
 *
 * @note ����Ķ���NODEID ��ο�AN143�ֲ�������ȷ����Ϣ����
 *        ����: ����Ĵ���0x47 bit3:2 ��NODEID_SIZEΪ1 ��NODEID����Ϊ2��
 *              ��ôӦ�������Ϣ�峤��Ϊ ��ʾλ1+NODEID����2+�û����ݳ��� = �û����ݳ���+3
 */
static void E49x_SetPayloadSize( uint16e_t size )
{
    uint8e_t config; 
      
    /* �Ĵ�����ַ:0x45 bit6��bit4:0x70  ��Ӧ�����峤��10:8bit*/  
    config = E49x_GetRegister(0x45);

    config &= ~0x70;
    config |= (size >> 4) & 0x70;
    E49x_SetRegister(0x45, config);
    
    /* �Ĵ�����ַ:0x46 bit7��bit0:0xFF ��Ӧ�����峤��7:0bit*/
    config = size & 0xFF;
    E49x_SetRegister(0x46, config);      
}

/* !
 * @brief ����ͨ��IO��ʹ�÷�ʽ
 * 
 * @param config ���궨��  GPIO1_xxx | GPIO2_xxx | GPIO3_xxx
 *
 * @note һ�㶼��ʹ��IO��Ϊ�ж�Դ ֪ͨMCU����/�������
 */
static void E49x_SetGPIO( uint8e_t config )
{
    /* �Ĵ�����ַ:0x65 */
    E49x_SetRegister(0x65, config);
}

/* !
 * @brief �����ж�����
 * 
 * @param configIrq1 �����ж�1������ ���궨�� 
 * @param configIrq2 �����ж�2������ ���궨�� 
 *
 * @note һ�㶼�����÷���/��������жϣ����E49x_SetGPIO����ӳ�䵽IO��
 */
static void E49x_SetIRQ(uint8e_t configIrq1, uint8e_t configIrq2)
{
    
    /* �ж�1ӳ�� ռ��bit4��bit0��0x1F �Ĵ�����ַ:0x66 */
    configIrq1 &= 0x1F;
    configIrq1 |= (~0x1F) & E49x_GetRegister(0x66);
    E49x_SetRegister(0x66, configIrq1);

    /* �ж�2ӳ�� ռ��bit4��bit0��0x1F �Ĵ�����ַ:0x67 */
    configIrq2 &= 0x1F;
    configIrq2 |= (~0x1F) & E49x_GetRegister(0x67);
    E49x_SetRegister(0x67, configIrq2);
}



/* !
 * @brief ��ȡ�жϱ�־
 *
 * @return  �ж���λ���bit
 *  
 *           0x2000:SL_TMO ˯�߳�ʱ�жϱ�־
 *           0x1000:RX_TMO ���ճ�ʱ�жϱ�־
 *           0x0800:TX_DONE ��������жϱ�־   
 *           0x0080:LBD ��Ч���ɹ���⵽�͵�ѹ���жϱ�־
 *           0x0040:COL_ERR �жϱ�־
 *           0x0020:PKT_ERR �жϱ�־
 *           0x0010:PREAM_OK ǰ������ɹ��жϱ�־
 *           0x0008:SYNC_OK ͬ���ּ��ɹ��жϱ�־
 *           0x0004:NODE_OK �ڵ�ID���ɹ��жϱ�־
 *           0x0002:CRC_OK  CRC���ɹ��жϱ�־
 *           0x0001:PKT_OK  ���ݰ�������ɣ����ܶԴ��ж�ʹ���жϱ�־
 *
 */
static uint16e_t E49x_GetIRQ(void)
{
    uint8e_t irqPolar,irqSetFlag6D,irqSetFlag6A;
    
    /* ��ȡ�жϼĴ������� �Ĵ�����ַ:0x6D + 0x6A */
    irqSetFlag6D = E49x_GetRegister(0x6D);
    irqSetFlag6A = E49x_GetRegister(0x6A);
    
    /* ��ȡ�жϼ���(0:��ʾ�ߵ�ƽ����Ч�ж� 1:��ʾ�͵�ƽ����Ч�ж�) bit1:0x20 �Ĵ�����ַ:0x66 */
    irqPolar = (E49x_GetRegister(0x66) & 0x20)? 1:0 ;
    
    /* �͵�ƽΪ��Ч�ж�ʱ ��Ҫλ������  */
    if( irqPolar )
    {
        irqSetFlag6D = ~irqSetFlag6D;
        irqSetFlag6A = ~irqSetFlag6A;
    }    
    
    return  (irqSetFlag6A<<8) | irqSetFlag6D;
}

/* !
 * @brief ��������жϱ�־
 *
 * @note  �ж���λ���bit
 *           �Ĵ�����ַ 0x6D
 *           bit7:LBD ��Ч���ɹ���⵽�͵�ѹ���жϱ�־
 *           bit6:COL_ERR �жϱ�־
 *           bit5:PKT_ERR �жϱ�־
 *           bit4:PREAM_OK �жϱ�־
 *           bit3:SYNC_OK �жϱ�־
 *           bit2:NODE_OK �жϱ�־
 *           bit1:CRC_OK �жϱ�־
 *           bit0:PKT_OK �жϱ�־
 *
 *           �Ĵ�����ַ 0x6A
 *           bit5:SL_TMO �жϱ�־
 *           bit4:RX_TMO �жϱ�־
 *           bit3:TX_DONE �жϱ�־
 *
 * @note  �ж�������bit
 *           �Ĵ�����ַ 0x6A
 *           bit2:TX_DONE �ж�����
 *           bit1:SL_TMO �ж�����
 *           bit0:RX_TMO �ж�����
 *
 *           �Ĵ�����ַ 0x6B
 *           bit5:LBD ��Ч���ɹ���⵽�͵�ѹ���ж�����
 *           bit4:PREAM_OK �ж�����
 *           bit3:SYNC_OK �ж�����
 *           bit2:NODE_OK �ж�����
 *           bit1:CRC_OK �ж�����
 *           bit0:PKT_DONE �ж�����
 */
void E49x_ClearIRQ(void)
{
    uint8e_t irqPolar,irqSetFlag6D,irqSetFlag6A;
    uint8e_t irqClearFlag6A = 0;
    uint8e_t irqClearFlag6B = 0;
    
    /* ��ȡ�жϼĴ������� �Ĵ�����ַ:0x6D + 0x6A */
    irqSetFlag6D = E49x_GetRegister(0x6D);
    irqSetFlag6A = E49x_GetRegister(0x6A);
    
    /* ��ȡ�жϼ���(0:��ʾ�ߵ�ƽ����Ч�ж� 1:��ʾ�͵�ƽ����Ч�ж�) bit1:0x20 �Ĵ�����ַ:0x66 */
    irqPolar = (E49x_GetRegister(0x66) & 0x20)? 1:0 ;
    
    /* �͵�ƽΪ��Ч�ж�ʱ ��Ҫλ������  */
    if( irqPolar )
    {
        irqSetFlag6D = ~irqSetFlag6D;
        irqSetFlag6A = ~irqSetFlag6A;
    }
    
    /* �ж���λ��ʶλ�������ʶλ��һ�� ��Ҫ��λ��� */
    if( irqSetFlag6D & 0x80 ) 
    {
        irqClearFlag6B |= 0x20;  //�Ĵ���0x6D-LBD��λbit7 ��Ӧ �Ĵ���0x6B-LBD����bit5
    }  
    if( irqSetFlag6D & 0x40 ) 
    {
        irqClearFlag6B |= 0x01;  //�Ĵ���COL_ERR��λbit6  ��Ӧ �Ĵ���0x6B-PKT_DONE����bit0
    }        
    if( irqSetFlag6D & 0x20 ) 
    {
        irqClearFlag6B |= 0x01;  //�Ĵ���0x6D-PKT_ERR��λbit5 ��Ӧ �Ĵ���0x6B-PKT_DONE����bit0
    }    
    if( irqSetFlag6D & 0x10 ) 
    {
        irqClearFlag6B |= 0x10;  //�Ĵ���0x6D-PREAM_OK��λbit4 ��Ӧ �Ĵ���0x6B-PREAM_OK����bit4
    }  
    if( irqSetFlag6D & 0x08 ) 
    {
        irqClearFlag6B |= 0x08;  //�Ĵ���0x6D-SYNC_OK��λbit3 ��Ӧ �Ĵ���0x6B-SYNC_OK����bit3
    }  
    if( irqSetFlag6D & 0x04 ) 
    {
        irqClearFlag6B |= 0x04;  //�Ĵ���0x6D-NODE_OK��λbit2 ��Ӧ �Ĵ���0x6B-NODE_OK����bit2
    }  
    if( irqSetFlag6D & 0x02 ) 
    {
        irqClearFlag6B |= 0x02;  //�Ĵ���0x6D-CRC_OK��λbit1 ��Ӧ �Ĵ���0x6B-CRC_OK����bit1
    }  
    if( irqSetFlag6D & 0x01 ) 
    {
        irqClearFlag6B |= 0x01;  //�Ĵ���0x6D-PKT_OK��λbit0 ��Ӧ �Ĵ���0x6B-PKT_DONE����bit0
    }      
    
    if( irqSetFlag6A & 0x20 ) 
    {
        irqClearFlag6A |= 0x02;  //�Ĵ���0x6A-SL_TMO��λbit5 ��Ӧ �Ĵ���0x6A-SL_TMO����bit1
    }  
    if( irqSetFlag6A & 0x10 ) 
    {
        irqClearFlag6A |= 0x01;  //�Ĵ���0x6A-RX_TMO��λbit4 ��Ӧ �Ĵ���0x6A-PKT_DONE����bit0
    }  
    if( irqSetFlag6A & 0x08 ) 
    {
        irqClearFlag6A |= 0x04;  //�Ĵ���0x6A-TX_DONE��λbit3 ��Ӧ �Ĵ���0x6A-TX_DONE����bit2
    }      
    
    /* �������ʶд�ؼĴ��� */
    E49x_SetRegister(0x6A, irqClearFlag6A);
    E49x_SetRegister(0x6B, irqClearFlag6B);
}

/* !
 * @brief �����ж�
 * 
 * @param enableIrq �����жϵ�bitλ
 * @note һ�㶼������ӳ���Ӧ���ж�λ
 */
static void E49x_GoIRQ( uint8e_t  enableIrq)
{
    /* �����ж� �Ĵ�����ַ:0x68 */
    E49x_SetRegister(0x68, enableIrq);
}

/* !
 * @brief ��ʼ��������
 * 
 * @note һ�����յ����� �ᴥ��GPIO�ж�
 */
uint8e_t E49x_GoReceive(void)
{
    uint8e_t result = 0;
    uint8e_t irqGpio = GPIO1_IRQ2;
    
    /* �����������ģʽ */
    result = E49x_SetStby();
    if( result!=0  ) return 1;
    
    /* �ж�ͨ��IRQ2 */
    switch( RECEIVE_IRQ_GPIO )
    {
        case 1:irqGpio=GPIO1_IRQ2;break;
        case 2:irqGpio=GPIO2_IRQ2;break;
        case 3:irqGpio=GPIO3_IRQ2;break;
        default: break;
    }
    
    /* ����GPIO�ж�ӳ�� */                  
    E49x_SetGPIO(  irqGpio  );    
    
    /* �����ж�����ӳ��  ���ｫ��������ж�ӳ�䵽IRQ2  IRQ_TX_DONEռλ */
    E49x_SetIRQ (  IRQ_TX_DONE,  IRQ_PKT_OK );   
   
    /* ���ò�����Ч */
    result = E49x_SetSleep();    
    if( result!=0  ) return 1;   
       
    /* �����������ģʽ */
    result = E49x_SetStby();
    if( result!=0  ) return 1;
    
    /* ��������жϱ�ʶ */
    E49x_ClearIRQ();
      
    /* �����ж�(�ڲ��Ĵ�����ʶ����λ) */
    E49x_GoIRQ( PREAM_OK_IRQ_EN | SYNC_OK_IRQ_EN | PKT_DONE_IRQ_EN );
    
    /* �����ȡ�ڲ�FIFO */
    E49x_GoFIFO( FIFO_READ );
    
    /* �������FIFO */
    E49x_ClearFIFO( FIFO_READ );
    
    /* �������ģʽ */
    result = E49x_SetReceive();
    if( result!=0  ) return 1;
    
    /* ״̬��:�ȴ�������� */
    E49x_Status = GO_WAIT_RECEIVE;
     
    
    /* �������� 0 */
    return result;
  
}

/* !
 * @brief ��ʼ��������
 * 
 * @param data ָ����������ݵ�ָ��
 * @param size �����ͳ���
 * @return 0:���� 1:����ʧ��
 * @note ע��size ����ϲ��˷��ͽ���FIFO ��֧��64�ֽڣ�����Ϊ32�ֽ�
 *       ���FIFO �ǿյģ����뷢��״̬����ô�ͻ�һֱ�����趨prefix����(����ǰ����)��ֱ��FIFO����������
 *       ����ÿ�η�������ֶ��л�Ϊ����ģʽ
 */
uint8e_t E49x_GoTransmit( uint8e_t *data, uint8e_t size )
{
    uint8e_t result = 0;
    uint8e_t irqGpio = GPIO1_IRQ2;
    
    /* �����������ģʽ */
    result = E49x_SetStby();
    if( result!=0  ) return 1;
    
    /* �ж�ͨ��IRQ2 */
    switch( TRANSMIT_IRQ_GPIO )
    {
        case 1:irqGpio=GPIO1_IRQ2;break;
        case 2:irqGpio=GPIO2_IRQ2;break;
        case 3:irqGpio=GPIO3_IRQ2;break;
        default: break;
    }    
    
    /* ����GPIO�ж�ӳ�䵽IRQ2 */                  
    E49x_SetGPIO(  irqGpio );    
    
    /* �����ж�����ӳ��  ���ｫ��������ж�ӳ�䵽IRQ2 IRQ_PKT_OKռλ */
    E49x_SetIRQ (  IRQ_PKT_OK , IRQ_TX_DONE  );   
    
    /* ���ò�����Ч */
    result = E49x_SetSleep();    
    if( result!=0  ) return 1;   
       
    /* �����������ģʽ */
    result = E49x_SetStby();
    if( result!=0  ) return 1;
    
    /* ��������жϱ�ʶ */
    E49x_ClearIRQ();
      
    /* �����ж� */
    E49x_GoIRQ( TX_DONE_IRQ_EN  );
    
    /* ����д���ڲ�FIFO */
    E49x_GoFIFO( FIFO_WRITE );
    
    /* �������FIFO */
    E49x_ClearFIFO( FIFO_WRITE );
    
    /* ����������д��FIFO */
    E49x_SetFIFO( data , size);
    
    /* Ĭ��Ϊ �ɱ䳤������+��NODEID=���ݳ���size+���ȱ�ʾλ1  �ο��ĵ�AN143 */
    E49x_SetPayloadSize( size+1 );
      
    /* ���뷢��ģʽ �Զ�����FIFO����*/
    result = E49x_SetTransmit();
    if( result!=0  ) return 1;
    
    /* ״̬��:�ȴ�������� */
    E49x_Status = GO_WAIT_TRANSMIT;
    
    /* �������������ģʽ ���ڴ˼�鵽������ɲŷ���*/    
#if EBYTE_TRANSMIT_MODE_BLOCKED
    uint16e_t irqSet = 0;
    do
    {
        irqSet = E49x_GetIRQ();
        Ebyte_Port_DelayUs(500);
    }
    while( !(irqSet & GET_IRQ_TX_DONE ) );
    
    E49x_ClearIRQ();
    
    /* ״̬��:������� �������״̬ */
    E49x_SetStby();
    
    /* ������� �ص��û��ӿں��� */
    Ebyte_Port_TransmitCallback( irqSet );
#endif    
    /* �������� 0 */
    return result;    
}

/* !
 * @brief ���ģ���Ƿ����������д
 *
 * @return 0:���� 1:��д������,����Ӳ��
 */
static uint8e_t E49x_IsAlive(void)
{
    uint8e_t back, getData ;
    uint8e_t testRegister = 0x48;
    uint8e_t testData = 0xAA;
    uint8e_t result = 1;
      
    /* ������Ĵ���0x48д������ test */
    back = E49x_GetRegister(testRegister);
    E49x_SetRegister(testRegister, testData);

    /* ��ȡȷ�� �ָ�ԭ���� */
    getData = E49x_GetRegister(testRegister);
    E49x_SetRegister(testRegister, back);  
    
    if( getData == testData )
    {
        /* ���� */
        result = 0;
    }

    return result;
}

/* !
 * @brief �������Ʋ������� 
 */
static void E49x_Config(void)
{
    uint8e_t config = 0;  
    uint8e_t i,start;
    
    /* �������ñ��� bit4:0x10, �رո�λ���� bit5:0x20 �Ĵ�����ַ:0x61 */
    config  = E49x_GetRegister(0x61);
    config |= 0x10;       
    config &= ~0x20;      
    E49x_SetRegister(0x61, config);

    /* �������໷Ƶ������ (������һ������Ƶ�ʲ���ȷ�������޷���������) bit5:0x20 �Ĵ�����ַ:0x62 */
    config  = E49x_GetRegister(0x62);
    config |= 0x20;        
    E49x_SetRegister(0x62, config);
    
    /* �ر�LFOSC����ģ�� ��LFOSC��Ҫ�������� Sleep Timer���رպ󽵵������߹����Լ���Լ5ms�Ľ���ʱ�䣩bit7��bit5=0 �Ĵ�����ַ:0x0D  */
    config  = E49x_GetRegister(0x0D);
    config &= ~0xE0;
    
    /* �ϲ�����/���ո��Զ�����32�ֽ�FIFO ����64�ֽ�FIFO bit1:0x02  �Ĵ�����ַ:0x69 */
    config  = E49x_GetRegister(0x69);
    config |= 0x02; 
    E49x_SetRegister(0x69, config);
    
    /* ����������ɵĲ��� д�뵽�Ĵ��� */
    /* CMT��  ��ʼ��ַ:0x00 ����:12 */
    start = 0;
    for( i=0 ; i<12; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_CMT[i] );
    }
    /* ϵͳ��   ��ʼ��ַ:0x0C ����:12 */
    start = 0x0C;
    for( i=0 ; i<12; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_System[i] );
    }    
    /* Ƶ����   ��ʼ��ַ:0x18 ����:8 */
    start = 0x18;
    for( i=0 ; i<8; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_Frequency[i] );
    }       
    /* �������� ��ʼ��ַ:0x20 ����:24 */
    start = 0x20;
    for( i=0 ; i<24; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_DataRate[i] );
    }   
    /* ������   ��ʼ��ַ:0x38 ����:29 */
    start = 0x38;
    for( i=0 ; i<29; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_Baseband[i] );
    } 
    /* ������   ��ʼ��ַ:0x55 ����:11 */
    start = 0x55;
    for( i=0 ; i<11; i++ )
    {
        E49x_SetRegister( start+i, E49x_Config_TX[i] );
    } 
       
    /* ԭ��BUG��Ҫ���� 1.4�汾�������������Ժ��Դ����� */
    config = (~0x07) & E49x_GetRegister(0x09);
    E49x_SetRegister(0x09, config|0x02);   
    
    /* ����Ƶ�ʲ�����׼ �Ĵ�����ַ:0x64 ���㹫ʽΪ 2.5KHz*n */
    E49x_SetRegister(0x64, EBYTE_E49x_FREQUENCY_STEP );
    
    /* ����Ƶ�ʲ����ۼ� �Ĵ�����ַ:0x63 ���㹫ʽΪ ������׼*n */
    E49x_SetRegister(0x63, EBYTE_E49x_FREQUENCY_CHANNEL );
    
    /* ���ò���ͨ��ģʽ�л���Ч */
    E49x_SetSleep();
    
}

/* !
 * @brief ģ���������ģʽ
 *
 * @return 0:���� 1:��������ʧ��
 */
uint8e_t E49x_GoSleep(void)
{
    return E49x_SetSleep();
}

/* !
 * @brief ��ʼ��
 *
 * @return 0:���� 1:��ʼ��ʧ��
 */
uint8e_t E49x_Init(void)
{
    uint8e_t i;
    uint8e_t result = 0 ;
  
    /* ��λ �����ᶪʧ */
    E49x_Reset();
    
    /* ���ٵȴ�20ms */
    for (i=0;i<20;i++)
    {
        Ebyte_Port_DelayUs(1000);
    }    
    
    /* �����������ģʽ */
    result = E49x_SetStby();
    if( result!=0  ) return 1;
    
    /* �Ĵ�����д��� */
    result = E49x_IsAlive();
    if( result!=0  ) return 1;
    
    /* ���õ��Ʋ��� */
    E49x_Config();    
    
    /* Ĭ�Ͽ�ʼ���� */
    result = E49x_GoReceive();
    if( result!=0  ) return 1;
    
    return result;
}

/* !
 * @brief �û����ڵ��ú��� ������ɽ��ջص�
 *
 * @return 0:���� 1:�����������ݲ���ȷ
 */
uint8e_t E49x_TaskForPoll(void)
{
     uint16e_t irqSet = 0;
     uint8e_t getSize = 0;
     
    /* ����״̬ �ȴ��ж��ź� */
    if( E49x_Status == GO_WAIT_RECEIVE && E49x_IRQ_Trigger )
    {       
        /* ��ȡ״̬ */
        irqSet = E49x_GetIRQ();

        /* �Ƿ������ݽ���(�жϱ�ʶ��λ) */
        if( irqSet & GET_IRQ_PKT_OK )
        {          
            /* ģ����� */
            E49x_SetStby();
            
            /* ��ȡ���� */
            E49x_GetFIFO( E49x_ReceiveBuffer, &getSize );
            
            /* ����ж� */
            E49x_ClearIRQ();
            
            /* �ص�������ɺ��� */
            Ebyte_Port_ReceiveCallback( GET_IRQ_PKT_OK , E49x_ReceiveBuffer ,getSize);
        }
        else
        {
            //to-do ��������������ж� �����û��Լ�����
//            Ebyte_Port_ReceiveCallback( irqSet, 0 , 0 );
        }
        
        E49x_ClearIRQ();
        
        /* ��λ */
        E49x_IRQ_Trigger = 0;        
    }
    
    /* ������ģʽ�� ����״̬ �ȴ��ж��ź� */
#if !(EBYTE_TRANSMIT_MODE_BLOCKED)    
    if( E49x_Status == GO_WAIT_TRANSMIT && E49x_IRQ_Trigger  )
    {
        /* ��ȡ״̬ */
        irqSet = E49x_GetIRQ();

        /* �Ƿ��������(�жϱ�ʶ��λ) */
        if( irqSet & GET_IRQ_TX_DONE )
        {
            /* ģ����� */
            E49x_SetStby();       
            
            /* ����ж� */
            E49x_ClearIRQ();
            
            /* ������� �ص��û��ӿں��� */
            Ebyte_Port_TransmitCallback( irqSet );
        } 
        
        /* ��λ */
        E49x_IRQ_Trigger = 0;
    }
#endif    
    
    return 0;
}

/* !
 * @brief �жϴ����� 
 * @note  һ����˵����IO�ж��е��� ��ѭ����TaskForPoll�����ź�
 */
void E49x_TaskForIRQ(void)
{
    E49x_IRQ_Trigger = 1;
}

/*!
 * @brief ��ȡģ������
 * 
 * @return ָ�������ַ�����ָ�� 
 * @note ��׼�ַ��� ĩβ���н����� '\0'
 */
uint8e_t* E49x_GetName(void)
{
    return E49x_NameString;
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
uint8e_t E49x_GetStatus(void)
{
  return (uint8e_t)E49x_Status;;
}

/* !
 * @brief ��ȡģ�����汾
 * 
 * @return 8λ�ı��� 
 * @note ����0x10 ����V1.0
 */
uint8e_t E49x_GetDriverVersion(void)
{
  return  EBYTE_E49_PROGRAM_TYPE;
}
