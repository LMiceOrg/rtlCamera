#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

static char g_buf[10*2*1024*1024];
static int g_size =0;

pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int save_image(const void* buf, int size);
int grab_image(void* buf);

int send_tcp_data(void* buf, int size);

void* worker_thread(void* pdata)
{
    int size;
    (void)pdata;

    for(;;)
    {
        //grab picture
        size = grab_image(g_buf);
        if(size <= 0)
        {
            sleep(1);
            continue;
        }

        //calculation

        //send to host
        send_tcp_data(g_buf, size);
    }
    printf("return worker\n");

    return 0;

}

int init_worker_proc(void)
{

    pthread_t cli;
    pthread_create(&cli, NULL, worker_thread, NULL);
    
    return 0;
}

int save_image(const void* buf, int size)
{
    pthread_mutex_lock(&g_mutex);

    g_size = size;
    memcpy(g_buf+size, buf, size);
    
    pthread_mutex_unlock(&g_mutex);

    return 0;
}

int grab_image(void* buf)
{
    pthread_mutex_lock(&g_mutex);

    memcpy(buf, g_buf+g_size, g_size);
    
    pthread_mutex_unlock(&g_mutex);

    return g_size;
}

