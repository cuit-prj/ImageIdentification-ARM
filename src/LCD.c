#include "LCD.h"

//初始化LCD
void Init_LCD()
{
    fd_lcd =open("/dev/fb0",O_RDWR);
    if(-1==fd_lcd){
        perror("open lcd");
        return ;
    }
    p = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd_lcd,0);//内存映射（返回值为首地址=int *p）：将像素点坐标映射到内存里。（0.0）像素点为首地址，（x，y）地址：p+800*y+x
    if(NULL==p){
        perror("mmap");
    }
}

//关LCD
void UnInit_LCD()
{
    close(fd_lcd);
    munmap(p,800*480*4);
}


void Show_BMP(const char * bmp_name)
{
// a.打开图片文件
    int fd_bmp = open(bmp_name , O_RDONLY);
    if(-1 == fd_bmp)
    {
        printf("open %s error:%s\n" , bmp_name , strerror(errno));
        return ;
    }
    //跳过54byte的文件头
    lseek(fd_bmp , 54 , SEEK_SET);
// b.读出颜色数据
    char buf[800*480*3];
    read(fd_bmp , buf , 800*480*3);

// c.将颜色数据写入到显示屏（将RGB格式转为ARGB格式）
    int r,g,b,i=0;
    int color[800*480];
    for(int y=0 ;y<480 ;y++)
        for(int x=0 ;x<800 ;x++)
        {
            //将RGB格式转为ARGB格式
            b = buf[i++];
            g = buf[i++]<<8;
            r = buf[i++]<<16;
            //将颜色数据写入到显示屏
            color[800*y+x] =  b | g | r;
        }
    for(int y=0 ;y<480 ;y++)
        for(int x=0 ;x<800 ;x++)
        {
            //将颜色数据写入到显示屏
            *(p + 800*(479-y) +x) = color[800*y+x];
        }
// d.关闭文件
    close(fd_bmp);
}





//显示小屏
void Show_SBMP(const char * bmp_name, int start_x, int start_y)
{
    int fd_bmp =open(bmp_name ,O_RDONLY);
    if(-1==fd_bmp)
    {
        printf("erro");
    }
    //获取图片的长度和高度
    char head[54];
    read(fd_bmp,head,54);//此时文件指针已经☞54的位置了，不需要lseek了。
    
    int len = head[18]|head[19]<<8|head[20]<<16|head[21]<<24;
    int hig =*((int*)&head[22]);

// 一个标准的 BMP 文件头通常由 54 字节组成，包含了以下几个部分：

//     位图文件头（14 字节）
//     位图信息头（40 字节）

// head 数组用于存储这 54 字节的数据。
// 在 BMP 文件头中，图像的宽度和高度存储在位图信息头的特定位置：
//     宽度（4 字节）：从偏移量 18 开始（第 19 到 22 字节）
//     高度（4 字节）：从偏移量 22 开始（第 23 到 26 字节）

//     int width = head[18] | head[19] << 8 | head[20] << 16 | head[21] << 24;
//     head[18]：最低有效字节
//     head[19] << 8：次低有效字节
//     head[20] << 16：次高有效字节
//     head[21] << 24：最高有效字节
//  这些字节通过位移操作组合成一个 32 位的整数，表示图像的宽度。

// int height = *((int *)&head[22]);
//  这里直接将 head[22] 到 head[25] 的四个字节解释为一个 int 类型的整数，表示图像的高度。
    
    printf("长度%d高度%d",len,hig);

//读取图片数据
    char buf[len*hig*3];
    read(fd_bmp,buf, len * hig * 3);

//转换bmp到屏幕上
int color[len*hig];
int r,g,b,i=0;
for (size_t y = 0; y <hig ; y++)

{
  for (size_t x = 0; x < len; x++)
  {
    b=buf[i++];
    g=buf[i++]<<8;
    r=buf[i++]<<16;

    color[len*y+x]=b|g|r;

      if(hig-1-y+start_y>=0 && hig-1-y+start_y<480 && x+start_x>=0 && x+start_x<800)//超出部分不显示


        *(p+800*(hig - 1 - y + start_y )+( x + start_x ))=color[len*y+x];
  }
 
}
    close(fd_bmp);
}


void Show_Rainbow()
{
    int len,r,g,b,color;
    for(int y=0 ;y<480 ;y++)
    {
        for(int x=0 ; x<800 ;x++)
        {
            len = pow(pow(x-400 , 2) + pow(y-479 , 2),0.5)/60;
            switch (len)
            {
            case 0:color = 0x00ffffff;break;
            case 1:b=252;g=32;r=152;color = b | g<<8 | r<<16 |0x00<<24;break;
            case 2:b=249;g=33;r=114;color = b | g<<8 | r<<16 |0x00<<24;break;
            case 3:b=243;g=39;r=51; color = b | g<<8 | r<<16 |0x00<<24;break;
            case 4:b=49;g=250;r=52; color = b | g<<8 | r<<16 |0x00<<24;break;
            case 5:b=3;g=239;r=255; color = b | g<<8 | r<<16 |0x00<<24;break;
            case 6:b=4;g=161;r=255; color = b | g<<8 | r<<16 |0x00<<24;break;
            case 7:b=34;g=33;r=255; color = b | g<<8 | r<<16 |0x00<<24;break;
            default:color = 0x00ffffff;break;
            }
              *(p+800*y+x)=color;
        }
    }
}