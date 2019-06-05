//��׼�������ͷ�ļ�
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

//���ļ���Ҫ��ͷ�ļ�
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//�򿪴�����Ҫ��ͷ�ļ�
#include <termios.h>
#include <unistd.h>

//����ģ���ͷ�ļ�
#include "SYN6288.h"

//�ַ�����ת��
#include <iconv.h>


//�������ڳ�ʼ������
int set_opt(int,int,int,char,int);
static void UART_SendData(int fd,unsigned char *buffer,int count);

//ȫ�ֱ���
char *uartPath = "/dev/ttySAC3";
int fd;

//�ַ�����ת��
static int charset_convert(const char *from_charset, const char *to_charset,
                           char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    iconv_t icd = (iconv_t)-1;
    size_t sRet = -1;
    char *pIn = in_buf;
    char *pOut = out_buf;
    size_t outLen = out_left;
 
    if (NULL == from_charset || NULL == to_charset || NULL == in_buf || 0 >= in_left || NULL == out_buf || 0 >= out_left)
    {
        return -1;
    }
 
    icd = iconv_open(to_charset, from_charset);
    if ((iconv_t)-1 == icd)
    {
        return -1;
    }
 
    sRet = iconv(icd, &pIn, &in_left, &pOut, &out_left);
    if ((size_t)-1 == sRet)
    {
        iconv_close(icd);
        return -1;
    }
 
    out_buf[outLen - out_left] = 0;
    iconv_close(icd);
    return (int)(outLen - out_left);
}


static int charset_convert_UTF8_TO_GB2312(char *in_buf, size_t in_left, char *out_buf, size_t out_left)
{
    return charset_convert("UTF-8", "GB2312", in_buf, in_left, out_buf, out_left);
}


int main(int argc, char **argv)
{
	unsigned char *buffer = (unsigned char *)malloc(sizeof(unsigned char) * 200);
	unsigned char *buffer_gb2312;
	int bufSize;
	int i, ret;

	if(argc != 2)
	{
		printf("argument number error!!\n");
		printf("%s \"the text you want to say\"\n", argv[0]);
		return -1;
	}

	if((fd = open(uartPath,O_RDWR|O_NOCTTY)) < 0)
	{
		printf("can not open %s\n",uartPath);
		return -1;
	}

	//printf("text: %s\n", argv[1]);
	
	set_opt(fd,9600,8,'N',1);

	bufSize = strlen(argv[1]);
	buffer_gb2312 = (unsigned char *)malloc(sizeof(unsigned char) * bufSize);

    ret = charset_convert_UTF8_TO_GB2312(argv[1], (size_t)bufSize, buffer_gb2312, (size_t)bufSize);
    if (-1 == ret)
    {
    	printf("failed convert UTF-8 to GB2312\n");
        return -1;
    }	

	bufSize = SYN_FrameInfo(0, buffer_gb2312, buffer);

	UART_SendData(fd, buffer, bufSize);

	free(buffer);
	free(buffer_gb2312);
	
	close(fd);
	
	return 0;
}


//���ڷ���һ�������
static void UART_SendData(int fd,unsigned char *buffer,int count)
{
	int j;

	write(fd, buffer, count);

	//��ӡ��������������Ļ
	printf("send : ");
	//��������������
	fflush(stdout);
	for(j=0;j<count;j++)
	{
		printf("%02X ",buffer[j]);
		fflush(stdout);
	}
	printf("\n");
}


/*******************************************************************************************
	��������set_opt();
	�������ã����ڳ�ʼ��
	����˵���� ����1��fd----open�������ص��ļ����
			   ����2 	��nSpeed----���������ã�2400��4800��9600��115200��
			   ����3��nBits----����λ���ã�7��8��
			   ����4��nEvent----У��λ���ã�'O'��'E'��'N'��
			   ����5��nStop----ֹͣλ��1��2��			   
*******************************************************************************************/
int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop){
	struct termios newtio,oldtio;
	//���ɵĲ��������ļ�����fdָ����ļ�
	if(tcgetattr(fd,&oldtio) != 0){
		perror("SetupSerial 1");
		return -1;
	}
	//��սṹ��newtio�е�����
	bzero(&newtio,sizeof(newtio));
	newtio.c_cflag |= CLOCAL|CREAD;
	newtio.c_cflag &= ~CSIZE;

	//��������λ
	switch (nBits)
		{
		case 7:{
			newtio.c_cflag |= CS7;
			break;}
		case 8:{
			newtio.c_cflag |= CS8;
			break;}
		}
	
	//����У��λ
	switch (nEvent)
		{
		case 'O':{
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= PARODD;
			newtio.c_iflag |= (INPCK | ISTRIP);
			break;}
		case 'E':{
			newtio.c_iflag |= (INPCK | ISTRIP);
			newtio.c_cflag |= PARENB;
			newtio.c_cflag &= ~PARODD;
			break;}
		case 'N':{
				newtio.c_cflag &= ~PARENB;
				break;}
		}

	//���ò�����
	switch (nSpeed)
		{
		case 2400:{
			cfsetispeed(&newtio,B2400);
			cfsetospeed(&newtio,B2400);
			break;}
		case 4800:{
			cfsetispeed(&newtio,B4800);
			cfsetospeed(&newtio,B4800);
			break;}
		case 9600:{
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;}
		case 115200:{
			cfsetispeed(&newtio,B115200);
			cfsetospeed(&newtio,B115200);
			break;}
		case 460800:{
			cfsetispeed(&newtio,B460800);
			cfsetospeed(&newtio,B460800);
			break;}
		default:{
			cfsetispeed(&newtio,B9600);
			cfsetospeed(&newtio,B9600);
			break;}
		}

	//����ֹͣλ
	if(nStop == 1){
		newtio.c_cflag &= ~CSTOPB;
	}
	else if(nStop == 2){
		newtio.c_cflag |= CSTOPB;
	}
	//�������
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;

/*******************************************************************************************
	��������tcflush();
	�������ã���մ��ڻ�����BUFFER�е�����
	����˵���� ����1��fd----open�������ص��ļ����
			   ����2��TCIFLUSH----������ܵ������ݣ��Ҳ��������
			   		  TCOFLUSH----�����д������ݣ��Ҳ��ᷢ�����ն�
			          TCIOFLUSH----����������ڷ�����IO����
	��������ֵ���ɹ�����0��ʧ�ܷ���-1			   
*******************************************************************************************/
	tcflush(fd,TCIFLUSH);

/*******************************************************************************************
	��������tcsetattr();
	�������ã����ô��ڲ�������
	����˵���� ����1��fd----open�������ص��ļ����
			   ����2��TCSANOW----�������ݴ�����Ͼ����̸ı�����
			   		  TCSADRAIN----�ȴ��������ݴ�������Ÿı�����
			          TCSAFLUSH----�����������������Ÿı�����
			   ����3��newtio----�ھɵĲ����������޸ĺ�Ĳ���
	��������ֵ���ɹ�����0��ʧ�ܷ���-1			   
*******************************************************************************************/	
	if((tcsetattr(fd,TCSANOW,&newtio)) != 0){
		perror("com set error");
		return -1;
	}
	return 0;
}

