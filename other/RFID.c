#include <stdio.h>
#include <assert.h>
#include <fcntl.h> 
#include <unistd.h>
#include <termios.h> 
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include "ISO14443A.h"

bool cardOn = false;
void *waitting(void *arg)
{
    char r[] = {'-', '\\', '|', '/'};
    for(int i=0;;i++)
    {
        fprintf(stderr, "\r%c", r[i%4]);
		if(cardOn)
            usleep(80*1000);
		else
            usleep(200*1000);
    }
}

void init_tty(int fd)
{    
	//声明设置串口的结构体
	struct termios config;
	bzero(&config, sizeof(config));

	// 设置无奇偶校验
	// 设置数据位为8位
	// 设置为非规范模式（对比与控制终端）
	cfmakeraw(&config);

	//设置波特率
	cfsetispeed(&config, B9600);
	cfsetospeed(&config, B9600);

	// CLOCAL和CREAD分别用于本地连接和接受使能
	// 首先要通过位掩码的方式激活这两个选项。    
	config.c_cflag |= CLOCAL | CREAD;

	// 一位停止位
	config.c_cflag &= ~CSTOPB;

	// 可设置接收字符和等待时间，无特殊要求可以将其设置为0
	config.c_cc[VTIME] = 0;
	config.c_cc[VMIN] = 1;

	// 用于清空输入/输出缓冲区
	tcflush (fd, TCIFLUSH);
	tcflush (fd, TCOFLUSH);

	//完成配置后，可以使用以下函数激活串口设置
	if(tcsetattr(fd, TCSANOW, &config) != 0)
	{
		perror("设置串口失败");
		exit(0);
	}
}

// 不断发送A指令，一旦探测到卡片就退出
void request_card(int fd)
{
	init_REQUEST();
	char recvinfo[128];

	while(1)
	{
		// 向串口发送指令
		tcflush (fd, TCIFLUSH);
		write(fd, PiccRequest_IDLE, PiccRequest_IDLE[0]);

		usleep(50*1000);

		bzero(recvinfo, 128);
		if(read(fd, recvinfo, 128) == -1)
			continue;

		//应答帧状态部分为0 则请求成功
		if(recvinfo[2] == 0x00)	
		{
			cardOn = true;
			break;
		}
		cardOn = false;
	}
}

void usage(int argc, char **argv)
{
	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s <tty>\n", argv[0]);
		exit(0);
	}
}

int get_id(int fd)
{
	// 刷新串口缓冲区
	tcflush (fd, TCIFLUSH);

	// 初始化获取ID指令并发送给读卡器
	init_ANTICOLL();
	write(fd, PiccAnticoll1, PiccAnticoll1[0]);

	usleep(50*1000);

	// 获取读卡器的返回值
	char info[256];
	bzero(info, 256);
	read(fd, info, 128);

	// 应答帧状态部分为0 则成功
	uint32_t id = 0;
	if(info[2] == 0x00) 
	{
		memcpy(&id, &info[4], info[3]);
		if(id == 0)
		{
			return -1;
		}
	}
	else
	{
		return -1;
	}
	return id;
}

bool flag = true;
void refresh(int sig)
{
	// 卡片离开1秒后
	flag = true;
}


//调用方法：RFID_TEST(argc, (char **)argv);
int RFID_TEST(int argc, char **argv)
{
	usage(argc, argv);
	signal(SIGALRM, refresh);//捕捉信号，注册信号响应函数refresh

	// 初始化串口
	int fd = open(argv[1], O_RDWR | O_NOCTTY);
	if(fd == -1)
	{
		printf("open %s failed: %s\n", argv[1], strerror(errno));
		exit(0);
	}
	init_tty(fd);

	// 将串口设置为非阻塞状态，避免第一次运行卡住的情况
	long state = fcntl(fd, F_GETFL);
	state |= O_NONBLOCK;
	fcntl(fd, F_SETFL, state);

	// 显示读卡状态
	pthread_t tid;
	pthread_create(&tid, NULL, waitting, NULL);

	int id;
	while(1)
	{
		// 检测附近是否有卡片
		request_card(fd);

		// 获取卡号
		id = get_id(fd);

		// 忽略非法卡号
		if(id == 0 || id == 0xFFFFFFFF)
			continue;

		// flag为真意味着：卡片刚放上去
		if(flag)
		{
			fprintf(stderr, "\r读到卡号:%x\n", id);
			flag = false;
		}
		alarm(1);
	}

	close(fd);
	exit(0);
}
