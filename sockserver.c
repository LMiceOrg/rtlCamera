#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <pthread.h>

#define TCP_PORT 18991
#define UDP_PORT 18991

void* udp_broadcast_thread(void* pdata);
void* tcp_accept_thread(void* pdata);
int init_udp_server(int, int, int);
int init_tcp_server(void);
int send_tcp_data(void* buf, int size);

struct sock_param {
int listen_fd;
int accept_fd[1];
int udp_fd;
int pic_sz;
int pic_w;
int pic_h;
};

static struct sock_param pm;

int init_tcp_server(void)
{
    int opt;
    struct sockaddr_in sa;
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(fd == -1)
    {
        return EXIT_FAILURE;        
    }
    opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family =         AF_INET;
    sa.sin_port =           htons(TCP_PORT);
    sa.sin_addr.s_addr =    INADDR_ANY;
    
    if(bind(fd, (const struct sockaddr*)&sa, sizeof(struct sockaddr_in) ) == -1)
    {
        close(fd);
        return EXIT_FAILURE;
    } 
    if(listen(fd, 10) == -1)
    {
        close(fd);
        return EXIT_FAILURE;
    }

    //Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    pm.listen_fd = fd;
    pm.accept_fd[0] = -1;
    pthread_t cli_trd;
    pthread_create(&cli_trd, NULL, tcp_accept_thread, (void*)&pm);
    return 0;
}

void* tcp_accept_thread(void* pdata)
{
    (void)pdata;
    for(;;)
    {
        struct sockaddr_in cli_sa;
        socklen_t cli_len;
        int cli_fd = accept(pm.listen_fd, (struct sockaddr*)&cli_sa, &cli_len);
        
        if(cli_fd <0)
        {
            sleep(1);
            //close(pm.listen_fd);
            continue;
        }
	    pm.accept_fd[0] = cli_fd;
        
    }
    return 0;
}

int send_tcp_data(void* buf, int len)
{
    int fd = pm.accept_fd[0];
    if(fd> 0)
    {
        return send(fd, buf, len, 0);
    } 
    else
        return -1;
}


int init_udp_server(int w, int h, int sz)
{
    int ret;
    int opt;
    int fd;
    struct sockaddr_in sa;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1)
    {
        return -1;
    }
 
    opt = 1;   
    ret = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));
    if(ret == -1)
    {
        return -1;
    }

    pm.udp_fd = fd;
    pm.pic_sz = sz;
    pm.pic_w = w;
    pm.pic_h = h;

    pthread_t cli;
    pthread_create(&cli, NULL, udp_broadcast_thread, NULL);
    return 0;
    
}

void* udp_broadcast_thread(void* pdata)
{
    char msg[128];
    size_t sz;
    struct sockaddr_in sa;
    socklen_t len; 
    (void)pdata;
    
    memset(&sa, 0, sizeof(struct sockaddr_in));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(UDP_PORT);
    sa.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    len = sizeof(sa);

    for(;;)
    {
        FILE* fp;
	memset(msg, 0, sizeof(msg));
        fp = fopen("/home/pi/config.txt", "r");
        if(fp)
        {
            sz = fread(msg, 1, 16, fp);
            fclose(fp);
        } 
        else 
        {
            sz = sprintf(msg, "camera-worker");
        }
	sz += sprintf(msg+sz, "|%d|%d|%d", pm.pic_sz, pm.pic_w, pm.pic_h);
        sendto(pm.udp_fd, msg, sz, 0, (const struct sockaddr*)&sa, len);
	printf("send udp\n");
        sleep(5);
    }

    return 0;
}
