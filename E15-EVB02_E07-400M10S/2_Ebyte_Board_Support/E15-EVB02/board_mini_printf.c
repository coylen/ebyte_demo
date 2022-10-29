#include <stdarg.h>
#include "stm8l15x_usart.h"
#include "board_mini_printf.h"

/* !
 * @brief Ŀ��Ӳ������ͨ�Žӿ�
 * 
 * @param data д������� 1 Byte
 */
static void send_uart_data(uint8_t data)
{
  while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
  USART_SendData8(USART1,data);
}
 
/*
  ���ܣ���int������תΪ2��8��10��16�����ַ���
  ������value --- �����int������
        str --- �洢ת�����ַ���
        radix --- ��������ѡ��
  ע�⣺8λ��Ƭ��int�ֽ�ֻռ2���ֽ�
*/
static char *sky_itoa(int value, char *str, unsigned int radix)
{
  char list[] = "0123456789ABCDEF";
  unsigned int tmp_value;
  int i = 0, j, k = 0;
	char tmp;
//  if (NULL == str) {
  if (0 == str) {
//    return NULL;
    return 0;
  }
  if (2 != radix && 8 != radix && 10 != radix && 16 != radix) {
//    return NULL;
    return 0;
  }
  if (radix == 10 && value < 0) {
    //ʮ������Ϊ����
    tmp_value = (unsigned int)(0 - value);
    str[i++] = '-';
    k = 1;
  } else {
    tmp_value = (unsigned int)value;
  }
  //����ת��Ϊ�ַ���������洢
  do {
    str[i ++] = list[tmp_value%radix];
    tmp_value /= radix;
  } while(tmp_value);
  str[i] = '\0';
  //�������ַ���ת��Ϊ����
  
  for (j = k; j < (i+k)/2; j++) {
    tmp = str[j];
    str[j] = str[i-j-1+k];
    str[i-j-1+k] = tmp;
  }
  return str;
}
 
/*
  ���ܣ���double������תΪ�ַ���
  ������value --- �����double������
        str --- �洢ת�����ַ���
        eps --- ����С��λѡ�����ٱ���һ��С��λ,���ౣ��4��С��λ
  ע�⣺8λ��Ƭ��int�ֽ�ֻռ2���ֽ�
*/
static void sky_ftoa(double value, char *str, unsigned int eps)
{
  unsigned int integer;
  double decimal;
  char list[] = "0123456789";
  int i = 0, j, k = 0;
	char tmp;
	double pp = 0.1;
	int tmp_decimal;
  //��������С��������ȡ����
  if (value < 0) {
    decimal = (double)(((int)value) - value);
    integer = (unsigned int)(0 - value);
    str[i ++] = '-';
    k = 1;
  } else {
    integer = (unsigned int)(value);
    decimal = (double)(value - integer);
  }
  //������������ת��Ϊ�ַ���������洢
  do {
    str[i ++] = list[integer%10];
    integer /= 10;
  } while(integer);
  str[i] = '\0';
  //�������ַ���ת��Ϊ����
  
  for (j = k; j < (i+k)/2; j++) {
    tmp = str[j];
    str[j] = str[i-j-1+k];
    str[i-j-1+k] = tmp;
  }
  //����С������
  if (eps < 1 || eps > 4) {
    eps = 4;
  }
  
  //�������⣬��ֹ����1.2���1.19�����
  
  for (j = 0; j <= eps; j++) {
    pp *= 0.1;
  }
  decimal += pp;
  while (eps) {
    decimal *= 10;
    eps --;
  }
  tmp_decimal = (int)decimal;
  str[i ++] = '.';
  k = i;
  //������������ת��Ϊ�ַ���������洢
  do {
    str[i ++] = list[tmp_decimal%10];
    tmp_decimal /= 10;
  } while(tmp_decimal);
  str[i] = '\0';
  //�������ַ���ת��Ϊ����
  for (j = k; j < (i+k)/2; j++) {
    tmp = str[j];
    str[j] = str[i-j-1+k];
    str[i-j-1+k] = tmp;
  }
  str[i] = '\0';
}
 
 
void mprintf(char * Data, ...)
{
  const char *s;
  int d;   
  char buf[16];
  uint8_t txdata;
  va_list ap;
  va_start(ap, Data);
  while ( * Data != 0 ) {				                          
    if ( * Data == 0x5c )  {									  
      switch ( *++Data ) {
        case 'r':							          
          txdata = 0x0d;
          send_uart_data(txdata);
          Data ++;
          break;
        case 'n':							          
          txdata = 0x0a;
          send_uart_data(txdata);
          Data ++;
          break;
        default:
          Data ++;
          break;
      }			 
    } else if ( * Data == '%') {									  
      switch ( *++Data ) {				
      case 's':										 
        s = va_arg(ap, const char *);
        for ( ; *s; s++) {
          send_uart_data(*((uint8_t *)s));
        }				
        Data++;				
        break;
      case 'd':			
        d = va_arg(ap, int);					
        sky_itoa(d, buf, 10);					
        for (s = buf; *s; s++) {
          send_uart_data(*((uint8_t *)s));
        }					
        Data++;				
        break;
      case 'x': {
        d = va_arg(ap, int);					
        sky_itoa(d, buf, 16);					
        for (s = buf; *s; s++) {
          send_uart_data(*((uint8_t *)s));
        }					
        Data++;			
        break;
      }
      case 'f': {
        double num = va_arg(ap, double);					
        sky_ftoa(num, buf, 4);
        for (s = buf; *s; s++) {
          send_uart_data(*((uint8_t *)s));
        }					
        Data++;			
        break;
      }
      default:
        Data++;				
        break;				
      }		 
    } else {
        send_uart_data(*((uint8_t *)Data));
        Data++;
    }
  }
}