#include "ebyte_e31x.h"
#include "ebyte_e31x_register_def.h"

///����ʶ��ģ��Ƶ��
#if defined(EBYTE_E31_400M17S)
static uint8e_t E31x_NameString[] = "E31-400M17S";
#elif defined(EBYTE_E31_900M17S)
static uint8e_t E31x_NameString[] = "E31-900M17S";
#endif

///����ʶ����������汾��
#define EBYTE_E31_PROGRAM_TYPE 0x10

///����Ƶ��
#define  XTAL_FREQ      26000000

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
}E31x_Status_t;

/// ״̬��ʶ
static E31x_Status_t E31x_Status = GO_INIT;

///���ջ���
static uint8e_t E31x_RxBuffer[255] ;


static uint8e_t  E31x_PreambleSize;
static uint32e_t E31x_SyncWord;
static uint8e_t E31x_CrcOnOff;
static uint8e_t E31x_PllRxRange;
static uint8e_t E31x_PllTxRange;

const uint16e_t E31x_RegisterVauleInit[][2] =
{
    { REG_AX5043_MODULATION,   0x08 },
    { REG_AX5043_ENCODING,   0x00   },
    { REG_AX5043_FRAMING,   0x26   }, // ǰ����ģʽ��Raw, Pattern Match  CRCģʽ��CRC-16
    { REG_AX5043_PINFUNCSYSCLK,  0x01},
    { REG_AX5043_PINFUNCDCLK, 0x01  },
    { REG_AX5043_PINFUNCDATA, 0x01  },
    { REG_AX5043_PINFUNCANTSEL, 0x00},
    { REG_AX5043_PINFUNCPWRAMP, 0x01},
    { REG_AX5043_WAKEUPXOEARLY, 0x01},
    { REG_AX5043_IFFREQ1, 0x00   },
    { REG_AX5043_IFFREQ0, 0xFC   },
    { REG_AX5043_DECIMATION, 0x5A  },
    { REG_AX5043_RXDATARATE2, 0x00  },
    { REG_AX5043_RXDATARATE1, 0x3c  },
    { REG_AX5043_RXDATARATE0, 0x2f  },
    { REG_AX5043_MAXDROFFSET2, 0x00 },
    { REG_AX5043_MAXDROFFSET1, 0x00 },
    { REG_AX5043_MAXDROFFSET0, 0x00 },
    { REG_AX5043_MAXRFOFFSET2, 0x80 },
    { REG_AX5043_MAXRFOFFSET1, 0x01 },
    { REG_AX5043_MAXRFOFFSET0, 0x17 },
    { REG_AX5043_FSKDMAX1, 0x00   },
    { REG_AX5043_FSKDMAX0, 0xA6   },
    { REG_AX5043_FSKDMIN1, 0xFF   },
    { REG_AX5043_FSKDMIN0, 0x5a   },
    { REG_AX5043_AMPLFILTER, 0x00   },
    { REG_AX5043_RXPARAMSETS, 0xF4  },
    { REG_AX5043_AGCGAIN0, 0x97   },
    { REG_AX5043_AGCTARGET0, 0x76   },
    { REG_AX5043_TIMEGAIN0, 0xf8   },
    { REG_AX5043_DRGAIN0, 0xf2   },
    { REG_AX5043_PHASEGAIN0,  0xc3  },
    { REG_AX5043_FREQUENCYGAINA0, 0x0f},
    { REG_AX5043_FREQUENCYGAINB0, 0x1f},
    { REG_AX5043_FREQUENCYGAINC0, 0x0a},
    { REG_AX5043_FREQUENCYGAIND0, 0x0a},
    { REG_AX5043_AMPLITUDEGAIN0, 0x46 },
    { REG_AX5043_FREQDEV10, 0x00   },
    { REG_AX5043_FREQDEV00, 0x00   },
    { REG_AX5043_FOURFSK0, 0x00   },
    { REG_AX5043_BBOFFSRES0, 0x00  },
    { REG_AX5043_AGCGAIN1, 0x97   },
    { REG_AX5043_AGCTARGET1, 0x76  },
    { REG_AX5043_AGCAHYST1, 0x00   },
    { REG_AX5043_AGCMINMAX1, 0x00  },
    { REG_AX5043_TIMEGAIN1, 0xf6   },
    { REG_AX5043_DRGAIN1, 0xf1   },
    { REG_AX5043_PHASEGAIN1, 0xc3  },
    { REG_AX5043_FREQUENCYGAINA1, 0x0f},
    { REG_AX5043_FREQUENCYGAINB1, 0x1f},
    { REG_AX5043_FREQUENCYGAINC1, 0x0a},
    { REG_AX5043_FREQUENCYGAIND1, 0x0a},
    { REG_AX5043_AMPLITUDEGAIN1, 0x46 },
    { REG_AX5043_FREQDEV11, 0x00   },
    { REG_AX5043_FREQDEV01, 0x38   },
    { REG_AX5043_FOURFSK1, 0x00   },
    { REG_AX5043_BBOFFSRES1, 0x00  },
    { REG_AX5043_AGCGAIN3, 0xff   },
    { REG_AX5043_AGCTARGET3, 0x76  },
    { REG_AX5043_AGCAHYST3, 0x00   },
    { REG_AX5043_AGCMINMAX3, 0x00  },
    { REG_AX5043_TIMEGAIN3, 0xf5   },
    { REG_AX5043_DRGAIN3, 0xf0   },
    { REG_AX5043_PHASEGAIN3, 0xc3  },
    { REG_AX5043_FREQUENCYGAINA3, 0x0f},
    { REG_AX5043_FREQUENCYGAINB3, 0x1f},
    { REG_AX5043_FREQUENCYGAINC3, 0x0d},
    { REG_AX5043_FREQUENCYGAIND3, 0x0d},
    { REG_AX5043_AMPLITUDEGAIN3, 0x46 },
    { REG_AX5043_FREQDEV13, 0x00  },
    { REG_AX5043_FREQDEV03, 0x38  },
    { REG_AX5043_FOURFSK3, 0x00  },
    { REG_AX5043_BBOFFSRES3, 0x00 },
    { REG_AX5043_MODCFGF, 0x03  },
    { REG_AX5043_FSKDEV2, 0x00  },
    { REG_AX5043_FSKDEV1, 0x01  },
    { REG_AX5043_FSKDEV0, 0x43  },
    { REG_AX5043_MODCFGA, 0x05  },
    { REG_AX5043_TXRATE2, 0x00  },
    { REG_AX5043_TXRATE1, 0x03  },
    { REG_AX5043_TXRATE0, 0x06  },
    { REG_AX5043_TXPWRCOEFFB1, 0x0f },
    { REG_AX5043_TXPWRCOEFFB0, 0xff },
    { REG_AX5043_PLLRNGCLK, 0x04  },
    { REG_AX5043_BBTUNE, 0x0f  },
    { REG_AX5043_BBOFFSCAP, 0x77  },
    { REG_AX5043_PKTADDRCFG, 0x00 },
    { REG_AX5043_PKTLENCFG, 0x80  },
    { REG_AX5043_PKTLENOFFSET, 0x00 },
    { REG_AX5043_PKTMAXLEN, 0xc8  },
    { REG_AX5043_MATCH0PAT3, 0xaa },
    { REG_AX5043_MATCH0PAT2, 0xcc },
    { REG_AX5043_MATCH0PAT1, 0xaa },
    { REG_AX5043_MATCH0PAT0, 0xcc },
    { REG_AX5043_MATCH0LEN, 0x1f  },
    { REG_AX5043_MATCH0MAX, 0x1f  },
    { REG_AX5043_MATCH1PAT1, 0x55 },
    { REG_AX5043_MATCH1PAT0, 0x55 },
    { REG_AX5043_MATCH1LEN, 0x8a  },
    { REG_AX5043_MATCH1MAX, 0x0a  },
    { REG_AX5043_TMGRXBOOST, 0x3e   },
    { REG_AX5043_TMGRXSETTLE, 0x31 },
    { REG_AX5043_TMGRXOFFSACQ, 0x7f },
    { REG_AX5043_TMGRXCOARSEAGC, 0x14},
    { REG_AX5043_TMGRXAGC, 0x00  },
    { REG_AX5043_TMGRXRSSI, 0x08  },
    { REG_AX5043_TMGRXPREAMBLE2, 0x35},
    { REG_AX5043_RSSIREFERENCE, 0x19 },
    { REG_AX5043_RSSIABSTHR, 0xc6 },
    { REG_AX5043_BGNDRSSIGAIN, 0x02 },
    { REG_AX5043_BGNDRSSITHR, 0x00 },
    { REG_AX5043_PKTCHUNKSIZE, 0x0d },
    { REG_AX5043_PKTMISCFLAGS, 0x03 },
    { REG_AX5043_PKTACCEPTFLAGS, 0x20},
    { REG_AX5043_DACVALUE1, 0x00  },
    { REG_AX5043_DACVALUE0, 0x00  },
    { REG_AX5043_DACCONFIG, 0x00  },
    { REG_AX5043_0xF10, 0x04   },
    { REG_AX5043_0xF11, 0x00   },
    { REG_AX5043_0xF1C, 0x07   },
    { REG_AX5043_0xF21, 0x5c   },
    { REG_AX5043_0xF22, 0x53   },
    { REG_AX5043_0xF23, 0x76   },
    { REG_AX5043_0xF26, 0x92   },
#if defined(EBYTE_E31_400M17S)
    { REG_AX5043_0xF34, 0x28   },
#elif defined(EBYTE_E31_900M17S)
    { REG_AX5043_0xF34, 0x08   },
#endif
    { REG_AX5043_0xF35, 0x11 },

    { REG_AX5043_AGCGAIN0, 0x83   },
    { REG_AX5043_AGCGAIN1, 0x83   },
    { REG_AX5043_TMGRXOFFSACQ, 0x00   },
    { REG_AX5043_TMGRXCOARSEAGC, 0x73   },
    { REG_AX5043_AGCTARGET0, 0x84  },
    { REG_AX5043_AGCTARGET1, 0x84  },
    { REG_AX5043_AGCTARGET3, 0x84  },
    { REG_AX5043_0xF21, 0x68  },
    { REG_AX5043_0xF22, 0xFF  },
    { REG_AX5043_0xF26, 0x98  },
    { REG_AX5043_0xF23, 0x84  },
    { REG_AX5043_0xF44, 0x25  },
    { 0xFFFF, 0xDDDD  }
};

const uint16e_t E31x_DatarateAddress[42] =
{
    REG_AX5043_IFFREQ1,
    REG_AX5043_IFFREQ0,
    REG_AX5043_DECIMATION,
    REG_AX5043_RXDATARATE2,
    REG_AX5043_RXDATARATE1,
    REG_AX5043_RXDATARATE0,

    REG_AX5043_MAXRFOFFSET2,
    REG_AX5043_MAXRFOFFSET1,
    REG_AX5043_MAXRFOFFSET0,

    REG_AX5043_FSKDMAX1,
    REG_AX5043_FSKDMAX0,

    REG_AX5043_FSKDMIN1,
    REG_AX5043_FSKDMIN0,

    REG_AX5043_AGCGAIN0,
    REG_AX5043_AGCTARGET0,

    REG_AX5043_TIMEGAIN0,
    REG_AX5043_DRGAIN0,

    REG_AX5043_FREQUENCYGAINA0,
    REG_AX5043_FREQUENCYGAINB0,
    REG_AX5043_FREQUENCYGAINC0,
    REG_AX5043_FREQUENCYGAIND0,
    REG_AX5043_AGCGAIN1,
    REG_AX5043_TIMEGAIN1,
    REG_AX5043_DRGAIN1,

    REG_AX5043_FREQUENCYGAINA1,
    REG_AX5043_FREQUENCYGAINB1,
    REG_AX5043_FREQUENCYGAINC1,
    REG_AX5043_FREQUENCYGAIND1,

    REG_AX5043_TIMEGAIN3,
    REG_AX5043_DRGAIN3,

    REG_AX5043_FREQUENCYGAINA3,
    REG_AX5043_FREQUENCYGAINB3,
    REG_AX5043_FREQUENCYGAINC3,
    REG_AX5043_FREQUENCYGAIND3,

    REG_AX5043_FSKDEV2,
    REG_AX5043_FSKDEV1,
    REG_AX5043_FSKDEV0,
    REG_AX5043_MODCFGA,
    REG_AX5043_TXRATE2,
    REG_AX5043_TXRATE1,
    REG_AX5043_TXRATE0,
    REG_AX5043_BBTUNE,
};

const uint16e_t E31x_DatarateValue[42] =
{
    0x00, 0xFC, 0x20, 0x00, 0xa9, 0x45, 0x80, 0x04, 0x49,
    0x03, 0x95, 0xFc, 0x6b, 0xe7, 0x84, 0xaa, 0xa4, 0x0f,
    0x1f, 0x0a, 0x0a, 0xe7, 0xa8, 0xa3, 0x0f, 0x1f, 0x0a,
    0x0a, 0xa7, 0xa2, 0x0f, 0x1f, 0x0d, 0x0d, 0x00, 0x05,
    0x0b, 0x05, 0x00, 0x03, 0x06, 0x0f
};

const uint16e_t E31x_TxRegister[][2] =
{
    {REG_AX5043_PLLLOOP, 0x0b},
    { REG_AX5043_PLLCPI, 0x10    },
#if defined(EBYTE_E31_400M17S)
    { REG_AX5043_PLLVCODIV, 0x24   },
#elif defined(EBYTE_E31_900M17S)
    { REG_AX5043_PLLVCODIV, 0x20   },
#endif
    { REG_AX5043_PLLVCOI, 0x99   },
    { REG_AX5043_XTALCAP, 0x02   },
    { REG_AX5043_0xF00, 0x0f    },
    { REG_AX5043_REF, 0x03    },
    { REG_AX5043_0xF18, 0x06    }
};

const uint16e_t E31x_RxRegister[][2] =
{
    { REG_AX5043_PLLLOOP, 0x0b},
    { REG_AX5043_PLLCPI, 0x10  },
#if defined(EBYTE_E31_400M17S)
    { REG_AX5043_PLLVCODIV, 0x24   },
#elif defined(EBYTE_E31_900M17S)
    { REG_AX5043_PLLVCODIV, 0x20   },
#endif
    { REG_AX5043_PLLVCOI, 0x99  },
    { REG_AX5043_XTALCAP, 0x02  },
    { REG_AX5043_0xF00, 0x0f  },
    { REG_AX5043_REF, 0x03   },
    { REG_AX5043_0xF18, 0x02  }
};

/*!
 * @brief д�����Ĵ���
 *
 * @param address �Ĵ�����ַ
 * @param data   �Ĵ���ֵ
 * @note �������ֽ�/һ�ֽڵĵ�ַ����Ҫ���ֽ���д�룬�ο� AX5243�ֲ� SPI Timing
 */
static void E31x_SetRegister( uint16e_t address, uint8e_t data )
{
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ����2�ֽ�/1�ֽڵ�ַ */
    if( address & 0xFF00 )
    {
        /* д2�ֽڵ�ַ */
        address |= 0xF000;//��15bit ΪR/Wλ����1��ʾ����Ϊд
        Ebyte_Port_SpiTransmitAndReceivce( ( address >> 8 ) );
        Ebyte_Port_SpiTransmitAndReceivce( address );
        /* д���� */
        Ebyte_Port_SpiTransmitAndReceivce( data );
    }
    else
    {
        /* д1�ֽڵ�ַ */
        Ebyte_Port_SpiTransmitAndReceivce( ( ( uint8e_t )( address | 0x0080 ) ) ); //��7bit ΪR/Wλ
        /* д���� */
        Ebyte_Port_SpiTransmitAndReceivce( data );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief �������Ĵ���
 *
 * @param address �Ĵ�����ַ
 * @return    �Ĵ���ֵ
 * @note ����2byte/1byte�ĵ�ַ����Ҫ���֣��Ĵ���ֵ����1byte���ο� AX5243�ֲ� SPI Timing
 */
static uint8e_t E31x_GetRegister( uint16e_t address )
{
    uint8e_t result = 0;
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ����2�ֽ�/1�ֽڵ�ַ */
    if( address & 0xFF00 )
    {
        /* д2�ֽڵ�ַ */
        address |= 0x7000;//��15bit ΪR/Wλ����0��ʾ����Ϊ�� ��|0x7000
        Ebyte_Port_SpiTransmitAndReceivce( ( address >> 8 ) );
        Ebyte_Port_SpiTransmitAndReceivce( address );
        /* ������ */
        result = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    }
    else
    {
        /* д1�ֽڵ�ַ */
        Ebyte_Port_SpiTransmitAndReceivce( address & 0x7F );//��7bit ΪR/Wλ ��&0x7F
        /* ������ */
        result = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
    return result;
}

/*!
 * @brief ������������д��FIFO
 *
 * @param data ָ��������
 * @param size ���ݳ���
 */
static void E31x_SetFIFO( uint8e_t* data, uint8e_t size )
{
    uint8e_t  i;
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ָ�� FIFODATA �Ĵ�����ַ:0x29 bit7ΪR/Wλ*/
    Ebyte_Port_SpiTransmitAndReceivce( 0x29 | 0x80 );
    /* д���� */
    for( i = 0; i < size; i++ )
    {
        Ebyte_Port_SpiTransmitAndReceivce( *data++ );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief ��ȡFIFO
 *
 * @param data ָ��������
 * @param size ��ȡ����
 */
static void E31x_GetFIFO( uint8e_t* data, uint8e_t size )
{
    uint8e_t  i;
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 0 );
    /* ָ�� FIFODATA �Ĵ�����ַ:0x29 bit7ΪR/Wλ */
    Ebyte_Port_SpiTransmitAndReceivce( REG_AX5043_FIFODATA & 0x7F );
    /* ������ */
    for( i = 0; i < size; i++ )
    {
        *data++ = Ebyte_Port_SpiTransmitAndReceivce( 0xFF );
    }
    /* SPI CS */
    Ebyte_Port_SpiCsIoControl( 1 );
}

/*!
 * @brief ���FIFO
 */
static void E31x_ClearFIFO( void )
{
    /* FIFOSTAT �Ĵ�����ַ:0x28 */
    E31x_SetRegister( 0x28, 0x83 );
}

/*!
 * @brief �ж�ģ���Ƿ����
 *
 * @return 0������  1:������
 */
static uint8e_t E31x_IsExist( void )
{
    uint8e_t result;
    /* REVISION ID�Ĵ��� ��ַ:0x00  */
    result = E31x_GetRegister( 0 );
    /* ��ȡ�üĴ�����Ϊ0x51 */
    if( result != 0x51 )
    {
        return 1;
    }
    return 0;
}

/*!
 * @brief �����ⲿ����
 */
static void E31x_SetXTAL( void )
{
    uint8e_t result;
    /* PWRMODE �Ĵ�����ַ:0x02 */
    result = E31x_GetRegister( 0x02 );
    /* �ϲ����� */
    result &= 0xF0;
    result |= 0x05;//ģʽ0x05 STANDBY  ������������
    /* ��д */
    E31x_SetRegister( 0x02, result );
    /* ��ȡ����״̬ XTALSTATUS �Ĵ�����ַ:0x1D bit0��λ��ʾ�������� */
    do
    {
        result = E31x_GetRegister( 0x1D );
    }
    while( ( result & 0x01 ) != 1 );
}

/*!
 * @brief ��λ
 *
 * @return 0:���� 1:�쳣
 */
uint8e_t  E31x_Reset( void )
{
    /* ��λ RST  PWRMODE �Ĵ�����ַ:0x02 bit7*/
    Ebyte_Port_SpiCsIoControl( 1 );
    Ebyte_Port_DelayMs( 1 ); //��ʱ����100ns �ο��ֲ�Power-on-Reset (POR)
    E31x_SetRegister( 0x02, 0x80 );
    E31x_SetRegister( 0x02, 0x00 );
    /* ͨ�ż�� �ȴ�ͨ���ȶ� SCRATCH �Ĵ�����ַ:0x01 */
    do
    {
        E31x_SetRegister( 0x01, 0x55 );
    }
    while( E31x_GetRegister( 0x01 ) != 0x55 );
    /* ���ż�� PINFUNCIRQ �Ĵ�����ַ:0x24 */
    E31x_SetRegister( 0x24, 0x00 ); //IRQ ���0
    Ebyte_Port_DelayMs( 2 );
    if( Ebyte_Port_IrqIoRead() != 0 )
    {
        return 1;
    }
    E31x_SetRegister( 0x24, 0x01 ); //IRQ ���1
    Ebyte_Port_DelayMs( 2 );
    if( Ebyte_Port_IrqIoRead() == 0 )
    {
        return 1;
    }
    return 0;
}

static void E31x_SetTxRegisters( void )
{
    uint8e_t i;
    for( i = 0; i < 8 ; i++ )
    {
        E31x_SetRegister( E31x_TxRegister[i][0], E31x_TxRegister[i][1] );
    }
    E31x_SetRegister( REG_AX5043_PLLRANGINGB, E31x_PllTxRange & 0x0F );
    E31x_SetRegister( REG_AX5043_PLLRANGINGA, E31x_PllTxRange & 0x0F );
}

static void E31x_SetRxRegisters( void )
{
    uint8e_t i;
    for( i = 0; i < 8 ; i++ )
    {
        E31x_SetRegister( E31x_RxRegister[i][0], E31x_RxRegister[i][1] );
    }
    E31x_SetRegister( REG_AX5043_PLLRANGINGB, E31x_PllRxRange & 0x0F );
    E31x_SetRegister( REG_AX5043_PLLRANGINGA, E31x_PllRxRange & 0x0F );
}

/*!
 * @brief ����Ƶ��
 */
static void E31x_SetFrequency( uint32e_t freq_hz )
{
    uint32e_t freq = ( uint32e_t )( ( double )( freq_hz / ( double )( XTAL_FREQ ) ) * 1024 * 1024 * 16 );
    E31x_SetRegister( REG_AX5043_FREQB0,  freq & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQB1, ( freq >> 8 ) & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQB2, ( freq >> 16 ) & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQB3, ( freq >> 24 ) & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQA0,  freq & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQA1, ( freq >> 8 ) & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQA2, ( freq >> 16 ) & 0xFF );
    E31x_SetRegister( REG_AX5043_FREQA3, ( freq >> 24 ) & 0xFF );
}

/*!
 * @brief ����Ƶ�ʲ��Զ�����Ƶ�ʷ�Χ
 *
 * @return 0:���� 1:ʧ��
 */
static uint8e_t E31x_SetFrequencyAutoRange( uint32e_t frequency )
{
    uint8e_t pllloop_save, pllcpi_save;
    /* ��׼Ƶ�� */
    E31x_SetFrequency( frequency );
    /* �л�ΪRX���� */
    E31x_SetRxRegisters();
    /* ����PLL���� */
    pllloop_save = E31x_GetRegister( REG_AX5043_PLLLOOP );
    pllcpi_save = E31x_GetRegister( REG_AX5043_PLLCPI );
    E31x_SetRegister( REG_AX5043_PLLLOOP, 0x09 );
    E31x_SetRegister( REG_AX5043_PLLCPI, 0x08 );
    /* �Զ�������׼ֵ ��ʼΪ0x18  �������1A�����Զ��������� */
    E31x_SetRegister( REG_AX5043_PLLRANGINGA, 0x1A );
    /* �ȴ�RNGSTART��λ ���Զ���������  */
    while( E31x_GetRegister( REG_AX5043_PLLRANGINGA ) & 0x10 );
    /* ����ж� ���RNGERR��λ ��ʾ����ʧ�� */
    E31x_PllRxRange = E31x_GetRegister( REG_AX5043_PLLRANGINGA );
    if( E31x_PllRxRange & 0x20 )
    {
        return 1;
    }
    /* �л�ΪTX���� */
    E31x_SetTxRegisters();
    /* ���̺�����һ�� */
    E31x_SetRegister( REG_AX5043_PLLLOOP, 0x09 );
    E31x_SetRegister( REG_AX5043_PLLCPI, 0x08 );
    E31x_SetRegister( REG_AX5043_PLLRANGINGA, 0x1A );
    while( E31x_GetRegister( REG_AX5043_PLLRANGINGA ) & 0x10 );
    E31x_PllTxRange = E31x_GetRegister( REG_AX5043_PLLRANGINGA );
    if( E31x_PllTxRange & 0x20 )
    {
        return 1;
    }
    /* �ɹ� ��ʼ��д */
    E31x_SetRegister( REG_AX5043_PLLLOOP, pllloop_save );
    E31x_SetRegister( REG_AX5043_PLLCPI, pllcpi_save );
    E31x_SetRegister( REG_AX5043_PLLRANGINGB, E31x_PllTxRange & 0x0F );
    E31x_SetRegister( REG_AX5043_PLLRANGINGA, E31x_PllTxRange & 0x0F );
    return 0;
}

/*!
 * @brief ����ģ�����
 *
 * @param frequency     ��׼�ز�Ƶ��(ͨ��Ƶ��)
 * @param output_power  ���书�� 0~0x0FFF  Ĭ��:���ֵ0x0FFF��17dBm
 * @param preamble_size ǰ���볤��
 * @param sync_word     ͬ���� 4byte
 * @param crc           CRC����
 * @return 0:���� 1:��������ȷ
 */
static uint8e_t  E31x_Config( uint32e_t frequency, uint16e_t output_power, uint8e_t preamble_size,  uint32e_t sync_word, uint8e_t crc )
{
    uint16e_t i;
    uint8e_t result = 0;
    /* д���ʼ������ ��ο�ԭ��������� AX-RadioLAB  */
    for( i = 0 ; i < 256; i++ )
    {
        /* ��������� 0xFFFF 0xDDDD */
        if( ( E31x_RegisterVauleInit[i][0] == 0xFFFF ) && ( E31x_RegisterVauleInit[i][1] == 0xDDDD ) )
        {
            break;
        }
        /* д��Ĵ��� */
        E31x_SetRegister( E31x_RegisterVauleInit[i][0], E31x_RegisterVauleInit[i][1] );
    }
    /* �����ⲿ���� */
    E31x_SetXTAL();
    /* ���ÿ��� Data Rate  Ĭ��1.2Kbps */
    for( i = 0 ; i < 42; i++ )
    {
        E31x_SetRegister( E31x_DatarateAddress[i],  E31x_DatarateValue[i] );
    }
    /* ���÷��书�� TXPWRCOEFFB1 �Ĵ�����ַ:0x16A  TXPWRCOEFFB0 �Ĵ�����ַ:0x16B */
    if( output_power > 0x0FFF )
    {
        return 1;    //Ĭ�����ֵ 0x0FFF, 17dBm
    }
    E31x_SetRegister( 0x16A, ( ( uint8e_t )( output_power >> 8 ) ) );
    E31x_SetRegister( 0x16B, ( ( uint8e_t ) output_power ) );
    /* ����ͬ���� */
    E31x_SyncWord = sync_word;
    E31x_SetRegister( REG_AX5043_MATCH0PAT3, ( ( uint8e_t )( sync_word >> 24 ) ) );//S3
    E31x_SetRegister( REG_AX5043_MATCH0PAT2, ( ( uint8e_t )( sync_word >> 16 ) ) );//S2
    E31x_SetRegister( REG_AX5043_MATCH0PAT1, ( ( uint8e_t )( sync_word >> 8 ) ) ); //S1
    E31x_SetRegister( REG_AX5043_MATCH0PAT0, ( ( uint8e_t )  sync_word ) );        //S0
    //    E31x_SetRegister( REG_AX5043_MATCH0LEN, 0x1F );//�޸�ͬ���ֳ��� Ĭ��4�ֽ�0x1F  ���2�ֽڿ�������Ϊ0x0F
    /* ��¼ǰ���볤�� ����ʱ���õ� */
    E31x_PreambleSize = preamble_size;
    /* ��¼CRC ����ʱ���õ�
       ע��:оƬ��������,����û��CRC ���շ������յ�����  */
    E31x_CrcOnOff = crc;
    /* ����Ƶ�ʲ������Զ����� ���ܻ�ʧ�ܣ� */
    result = E31x_SetFrequencyAutoRange( frequency );
    if( result != 0 )
    {
        return 1;
    }
    return result;
}

/*!
 * @brief ��ʼ��������
 *
 * @return 0:���� 1:�쳣
 */
uint8e_t E31x_GoReceive( void )
{
    uint8e_t result = 0;
    /* ģʽ�л�: POWERDOWN */
    E31x_SetRegister( 0x02, 0x00 );
    /* �л����ռĴ������� */
    E31x_SetRxRegisters();
    /* �����ⲿ���� */
    E31x_SetXTAL();
    /* ����FIFO */
    E31x_SetRegister( REG_AX5043_PWRMODE, AX5043_PWRSTATE_FIFO_ON );
    /* ���RSSIƫ�Ƽ��㳣�� */
    E31x_SetRegister( REG_AX5043_RSSIREFERENCE, 0x19 );
    /* �ر�ǰ���볬ʱ���� */
    E31x_SetRegister( REG_AX5043_TMGRXPREAMBLE1, 0x00 );
    /* CRC�����Ϻ���CRC��ռ�ֽ� */
    result = E31x_GetRegister( REG_AX5043_PKTSTOREFLAGS ) & ( ~0x40 );
    E31x_SetRegister( REG_AX5043_PKTSTOREFLAGS, result );
    /* FIFO: ���ж� */
    uint16e_t fifoIRQ = 0x0001;//FIFO�ǿ��ж�
    E31x_SetRegister( REG_AX5043_IRQMASK0, ( ( uint8e_t )fifoIRQ ) );
    E31x_SetRegister( REG_AX5043_IRQMASK1, ( ( uint8e_t )( fifoIRQ >> 8 ) ) );
    /* FIFO: ������� */
    E31x_ClearFIFO();
    /* ģʽ�л�: FULLRX  ����ʼ���� */
    E31x_SetRegister( REG_AX5043_PWRMODE, 0x09 );
    /* ״̬��¼ */
    E31x_Status = GO_WAIT_RECEIVE;    
    return result;
}

/*!
 * @brief ��ʼ��������
 *
 * @param data ָ��������
 * @param size �������ݳ���
 */
uint8e_t E31x_GoTransmit( uint8e_t* data, uint8e_t size )
{
    uint8e_t result;
    /* ģʽ�л�: POWERDOWN */
    E31x_SetRegister( 0x02, 0x00 );
    /* �л�TX���� */
    E31x_SetTxRegisters();
    /* ���ⲿ���� */
    E31x_SetXTAL();
    /* �ر��ж� */
    E31x_SetRegister( REG_AX5043_IRQMASK0, 0x00 );
    E31x_SetRegister( REG_AX5043_0xF08, 0x07 ); //Ī��˵�� ��ʱ����
    /* ��FIFO */
    E31x_SetRegister( REG_AX5043_PWRMODE, AX5043_PWRSTATE_FIFO_ON );
    /* FIFO:���� ��ֵ/�ٽ��  */
    E31x_SetRegister( REG_AX5043_FIFOTHRESH1, 0x00 );
    E31x_SetRegister( REG_AX5043_FIFOTHRESH0, 0x80 );
    /* FIFO: ������� */
    E31x_ClearFIFO();
    /* FIFO ���ݰ����� 240�ֽ� */
    E31x_SetRegister( REG_AX5043_PKTCHUNKSIZE, 0x0D );
    E31x_SetRegister( REG_AX5043_IRQMASK1, 0x00 );
    /* ����֡:ǰ����  �ο�AN9347 �½�REPEATDATA COMMAND */
    E31x_SetRegister( REG_AX5043_FIFODATA, AX5043_FIFOCMD_REPEATDATA | ( 3 << 5 ) ); //֡ͷ ����REPEATDATA 0x62
    E31x_SetRegister( REG_AX5043_FIFODATA, 0x38 ); //��ʶ�� 0x38 ��֡��CRC
    E31x_SetRegister( REG_AX5043_FIFODATA, E31x_PreambleSize ); //�ظ����� ��ǰ���볤��
    E31x_SetRegister( REG_AX5043_FIFODATA, 0x55 ); //ǰ�������� ʵ������0xAA �ȷ���bit(LSB)
    /* ����֡:ͬ����  �ο� TRANSMIT DATA FORMAT ͬ����ƥ���ᱻ���� */
    E31x_SetRegister( REG_AX5043_FIFODATA, 0xA1 ); //֡ͷ A������Ϊ5 ����Ϊ1 ��ͨ����
    E31x_SetRegister( REG_AX5043_FIFODATA, 0x18 ); //��ʶ�� ��CRC
    E31x_SetRegister( REG_AX5043_FIFODATA, ( ( uint8e_t )  E31x_SyncWord ) );         //S0
    E31x_SetRegister( REG_AX5043_FIFODATA, ( ( uint8e_t )( E31x_SyncWord >> 8 ) ) );  //S1
    E31x_SetRegister( REG_AX5043_FIFODATA, ( ( uint8e_t )( E31x_SyncWord >> 16 ) ) ); //S2
    E31x_SetRegister( REG_AX5043_FIFODATA, ( ( uint8e_t )( E31x_SyncWord >> 24 ) ) ); //S3
    /* ����֡:�������� �ο� TRANSMIT DATA FORMAT */
    E31x_SetRegister( REG_AX5043_FIFODATA, 0xE1 ); //֡ͷ  E����ɱ����ݰ� 1������ͨ����
    E31x_SetRegister( REG_AX5043_FIFODATA, size + 2 ); //�������ݰ����� �������˱�ʶ����һλ����λ ���³���+2
    if( E31x_CrcOnOff )//CRC����
    {
        E31x_SetRegister( REG_AX5043_FIFODATA, 0x03 ); //��ʶ�� 0x03 ��֡ ��CRC
    }
    else
    {
        E31x_SetRegister( REG_AX5043_FIFODATA, 0x0B ); //��ʶ�� 0x0B ��֡ ��CRC
    }
    E31x_SetRegister( REG_AX5043_FIFODATA, size + 1 ); //DATA  �Ҿ��ÿ��Խ����鳤��λ
    /* ����֡:��ʵ���� */
    E31x_SetFIFO( data, size );
    /* ������/��������¼� REVMDONE bit0 */
    E31x_SetRegister( REG_AX5043_RADIOEVENTMASK0, 0x01 );
    /* ������IC�Զ�Ӳ��ʹ���ж� IRQMRADIOCTRL */
    E31x_SetRegister( REG_AX5043_IRQMASK0, 0x40 );
    /* FIFO: Commit �ύ ����ˢ�� */
    E31x_SetRegister( REG_AX5043_FIFOSTAT, 0x04 );
    /* ģʽ�л�: FULLTXģʽ ����ʼ���� */
    E31x_SetRegister( 0x02, 0x0D );
    /* ״̬��¼ */
    E31x_Status = GO_WAIT_TRANSMIT;   
    /* �����ȴ�������� */
    do
    {
        result = E31x_GetRegister( REG_AX5043_RADIOSTATE );
    }
    while( result != 0 );//����������ɺ� ״̬Ϊidle ��Ϊ0
    /* ״̬��¼ */
    E31x_Status = GO_STBY;     
    /* �ص��û����� */
    Ebyte_Port_TransmitCallback( 0x0001 );
    E31x_SetRegister( REG_AX5043_0xF08, 0x04 ); //Ī��˵�� ��ʱ����
    return 0 ;
}

/*!
 * @brief ��������ģʽ
 */
uint8e_t E31x_GoSleep( void )
{
    /* (��ѡ)�ر��ⲿ���� ��������ⲿ��Ƭ�����Ƶ�TXCO���� ��ô����ȥ����� */
    E31x_SetRegister( REG_AX5043_PINFUNCPWRAMP, 0x00 );
    /* ����POWERDOWNģʽ ���˼Ĵ�����������ģ�鶼�ر� ���ƼĴ��������ݲ��ᶪʧ
       оƬ���ṩ�������ģʽ 0x01 ���������ݽ���ʧ �����鿼�� */
    E31x_SetRegister( 0x02, 0x00 ) ;
    /* ״̬��¼ */
    E31x_Status = GO_SLEEP;    
    return 0;
}


/*!
 * @brief ģ���ʼ��
 *
 * @return 0:���� 1:��ʼ��ʧ��
 */
uint8e_t E31x_Init( void )
{
    uint8e_t result = 0;
    /* �ж�ģ���Ƿ���� */
    result = E31x_IsExist();
    if( result != 0 )
    {
        return 1;
    }
    /* ��λ */
    result = E31x_Reset();
    if( result != 0 )
    {
        return 1;
    }
    /* �������� */
    result = E31x_Config( E31X_FREQUENCY_START,
                          E31X_OUTPUT_POWER,   
                          E31X_PREAMBLE_SIZE,  
                          E31X_SYNC_WORD,
                          E31X_IS_CRC );
    if( result != 0 )
    {
        return 1;
    }
    /* �������ģʽ */
    E31x_GoReceive();
    return result;
}

/*!
 * @brief ���ڵ��ú��� ��������������ݽ���
 *
 * @return 0
 * @note ��Ҫ��������ѭ�����ã���Ϊͨ����ѯ�ڲ���ʶλ���ж��Ƿ������ݵ���
 *       ���������жϣ����ж��е��ñ���������
 */
uint8e_t E31x_TaskForPoll( void )
{
    uint8e_t statusIRQ ;
    /* ��ȡ״̬��Ϣ */
    statusIRQ = E31x_GetRegister( REG_AX5043_IRQREQUEST0 );
    /* 0x01����ʾFIFO�н��յ����� */
    if( statusIRQ & 0x01 )
    {
        /* ������˵��FIFO���ݵ�һ�ֽڱ�ʾ�����볤��
           ���ݰ���ʽ���Բο��ֲ�AND9347�½� FIFO Chunk Encoding  */
        uint8e_t chunkHeader = 0;
        E31x_GetFIFO( &chunkHeader, 1 );
        /* ��ͷ�е�bit7:bit5��ʾ����  bit4:bit0��ʾ����  */
        uint8e_t chunkSize = ( chunkHeader >> 5 );
        uint8e_t chunkType = ( chunkHeader & 0x1F );
        /* ������Ϊ���ֵ7ʱ ��ʾ����Ϊ�ɱ䳤���ݰ� ��Ҫ�Լ����������� */
        if( chunkSize == 7 )
        {
            E31x_GetFIFO( &chunkSize, 1 );//��ȡ����
        }
        /* �������ͽ��н�� */
        switch( chunkType )
        {
            /* 0x01:��ͨ���ݰ� DATA */
            case 0x01 :
                if( chunkSize != 0 )
                {
                    uint8e_t chunkFlag = 0;
                    E31x_GetFIFO( &chunkFlag, 1 );//��ȡ��ʶ�� �����д���
                    uint8e_t dataSize = 0;
                    E31x_GetFIFO( &dataSize, 1 ); //��ȡ���ݿ鳤��
                    dataSize -= 1;                //ȥ������λռ��1λ ��-1
                    E31x_GetFIFO( E31x_RxBuffer, dataSize );//��ȡ��Ч����
                    E31x_Status = GO_STBY;//״̬��¼                    
                    Ebyte_Port_ReceiveCallback( 0x0001, E31x_RxBuffer, dataSize ); //�ص��û�����
                }
                break;
            /* 0x11:RSSI */
            case 0x11 :
                break;
            default:
                break;
        }
    }
    return 0;
}

/*!
 * @brief ����
 */
void E31x_TaskForIRQ( void )
{
}

/*!
 * @brief ��ȡģ������
 *
 * @return ָ�������ַ�����ָ��
 * @note ��׼�ַ��� ĩβ���н����� '\0'
 */
uint8e_t* E31x_GetName( void )
{
    return E31x_NameString;
}

/*!
 * @brief ��ȡģ�����汾
 *
 * @return 8λ�ı���
 * @note ����0x10 ����V1.0
 */
uint8e_t E31x_GetDriverVersion( void )
{
    return  EBYTE_E31_PROGRAM_TYPE;
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
uint8e_t E31x_GetStatus( void )
{
    return (uint8e_t)E31x_Status;
}