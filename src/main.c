#include "LCD.h"
#include "TOUCH.h"

int main(int argc,char const *argv[])
{
Init_LCD();
Init_touch();

int x,y,i=0,type;
char * bmp_name[]={"/Test/lyg/a.bmp","/Test/lyg/b.bmp","/Test/lyg/c.bmp"};
int len  = sizeof bmp_name / sizeof bmp_name[0];
    while (1)
    {
        Show_SBMP(bmp_name[i],0,0);
        type = read_touch(&x,&y);
        printf("(%d,%d)\n",x,y);
        if(type==UP)
        {
            i--;
            if(i<0)
                i = len-1;
        }
        else if(type==DOWN)
        {
            i++;
            if(i>=len)
                i=0;
        }
    }

UnInit_LCD();
close(fd_touch);
return 0;
}
