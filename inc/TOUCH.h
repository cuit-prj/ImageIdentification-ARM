#ifndef TOUCH_H
#define TOUCH_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>  //全局错误码机制
#include <string.h>
#include <unistd.h>
#include <stdio.h> 
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <linux/input.h>

enum {UP,DOWN,LEFT,RIGHT,CLICK};//枚举
int fd_touch;
//初始化触摸屏
void Init_touch();
void get_xy(int *x,int*y);

int read_touch(int *x,int*y);
void test_touch();


#endif