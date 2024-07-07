#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void*printa(void * arg)
{
    for (int i = 0; i < 5; i++)
    {
        printf("its ptha\n");
        sleep(1);
       
    }
}


int main(int argc,char const *argv[])
{

pthread_t pthid1;


pthread_create(&pthid1 , NULL , printa , NULL);


for (int m = 0; m < 5; m++)
    {
        printf("its main\n");
        sleep(1);
        
    }
pthread_join


/* 
pthread_exit(NULL);//不会打印exit，所有线程都没了
printf("exit"); */

return 0;//进程退出，杀死所有线程，不管线程有没有进行完

}