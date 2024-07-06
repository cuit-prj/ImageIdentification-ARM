#include"TOUCH.h"

void Init_touch()
{
fd_touch =open("/dev/input/event0",O_RDONLY);
if (fd_touch == -1) {
    perror("Failed to open touch device");
   
}
}


void get_xy(int *x,int*y)
{

    struct input_event buf;//输入事件结构体
    *x=*y=-1;
    while(1)
    {
        read(fd_touch ,&buf,sizeof buf);
        if(buf.type==EV_ABS)
        {
            if(buf.code==ABS_X)
            {
                *x=buf.value;
                *x=*x*800/1024;
            }
            if(buf.code==ABS_Y)
            {
                *y=buf.value;
                *y=*y*480/600;
                if(*x>=0)break;
            }
        }
    }
    
}



int read_touch(int *x , int *y)
{
    struct input_event buf; //定义输入事件结构体

    *x = *y = -1;
    int end_x,end_y,start_x,start_y;//定义点击起始位和松开结束位，通过此计算滑动操作
    while (1)
    {
        read(fd_touch , &buf , sizeof buf);
        if(buf.type == EV_ABS)  //绝对位移事件
        {
            if(buf.code == ABS_X)
            {
                *x = buf.value;
                *x = *x*800/1024; //黑色屏幕
            }
            if(buf.code == ABS_Y)
            {
                *y = buf.value;
                *y = *y*480/600; //黑色屏幕
            }
        }
        if(buf.type == EV_KEY && buf.code == BTN_TOUCH )
        {
            if(buf.value == 0)//松开
            {
                end_x = *x;
                end_y = *y;
                break;
            }
            else if(buf.value == 1)//按下
            {
                start_x = *x;
                start_y = *y;
            }
        } 
    }
    //计算首尾触点的距离
    int t_x = end_x - start_x;
    int t_y = end_y - start_y;

    if(abs(t_x)<20 && abs(t_y)<20)//用手点击是点一块地方，在首尾距离较近的移动情况下视作点击操作
        return CLICK;

/*
---------------------------------------------------------------------------------------
|  A(0,0)                                                                             |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                    M(x,y)                                           |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                                     |
|                                                                          Z(799,479) |
---------------------------------------------------------------------------------------
*/
    if(abs(t_x) > abs(t_y))//左右移动距离大于上下移动距离，视作左右滑动操作
    {
        if(t_x > 0)
            return RIGHT;
        else
            return LEFT;
    }
    else
    {
        if(t_y > 0)
            return DOWN;
        else
            return UP;
    }
}

void test_touch()
{
int x,y,type;
while (1)
{
type=read_touch(&x,&y);
switch (type)
{
case CLICK:printf("%d,%d\n",x,y);break;
case UP:printf("UP\n");break;
case DOWN:printf("DOWN\n");break;
case LEFT:printf("LEFT\n");break;
case RIGHT:printf("RIGHT\n");break;
default:printf("erro\n");break;
}
}
}