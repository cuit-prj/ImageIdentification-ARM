#ifndef LCD_H
#define LCD_H
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
int fd_lcd;
int *p;
void Init_LCD();
void UnInit_LCD();
void Show_BMP(const char * bmp_name);
void Show_SBMP(const char * bmp_name, int start_x, int start_y);
void Show_Rainbow();
#endif