
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <termios.h>
#include "jpeg.h"
#include "ISO14443A.h"
#include <errno.h>
#include <sys/stat.h>



#define PIPE_PATH "/tmp/photoname_pipe"

unsigned int *fb_mem;
struct {
    void *start;
    size_t length;
} *buffers;
bool cardOn = false;
bool flag = false;
int cam_fd;
pthread_mutex_t lock;

void *waitting(void *arg);
void init_tty(int fd);
void request_card(int fd);
int get_id(int fd);
void refresh(int sig);
int captureV(void);
void *read_rfid(void *arg);


void *waitting(void *arg) {
    char r[] = {'-', '\\', '|', '/'};
    for(int i = 0;; i++) {
        fprintf(stderr, "\r%c", r[i % 4]);
        if(cardOn)
            usleep(80 * 1000);
        else
            usleep(200 * 1000);
    }
}
void init_tty(int fd) {
    struct termios config;
    bzero(&config, sizeof(config));

    cfmakeraw(&config);
    cfsetispeed(&config, B9600);
    cfsetospeed(&config, B9600);

    config.c_cflag |= CLOCAL | CREAD;
    config.c_cflag &= ~CSTOPB;

    config.c_cc[VTIME] = 0;
    config.c_cc[VMIN] = 1;

    tcflush(fd, TCIFLUSH);
    tcflush(fd, TCOFLUSH);

    if(tcsetattr(fd, TCSANOW, &config) != 0) {
        perror("设置串口失败");
        exit(0);
    }
}
void request_card(int fd) {
    init_REQUEST();
    char recvinfo[128];

    while(1) {
        tcflush(fd, TCIFLUSH);
        write(fd, PiccRequest_IDLE, PiccRequest_IDLE[0]);

        usleep(50 * 1000);

        bzero(recvinfo, 128);
        if(read(fd, recvinfo, 128) == -1)
            continue;

        if(recvinfo[2] == 0x00) {
            cardOn = true;
            break;
        }
        cardOn = false;
    }
}
int get_id(int fd) {
    tcflush(fd, TCIFLUSH);

    init_ANTICOLL();
    write(fd, PiccAnticoll1, PiccAnticoll1[0]);

    usleep(50 * 1000);

    char info[256];
    bzero(info, 256);
    read(fd, info, 128);

    uint32_t id = 0;
    if(info[2] == 0x00) {
        memcpy(&id, &info[4], info[3]);
        if(id == 0) {
            return -1;
        }
    } else {
        return -1;
    }
    return id;
}
void refresh(int sig) {
    flag = true;
}


void clear_pipe(const char *pipe_path) {
    int fd = open(pipe_path, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("open");
        return;
    }

    char buffer[1024];
    while (read(fd, buffer, sizeof(buffer)) > 0) {
        // 读取并丢弃数据
    }

    close(fd);
}

int captureV(void) {
    int fd, jpg_fd;
    int ret, jpg_size;
    int x, y, i = 0;
    char pic_name[20];
    char *jpg_mem;
    int fdphoto;
    char filename[256];

    // 创建有名管道
    if (mkfifo(PIPE_PATH, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

   

    fd = open("/dev/fb0", O_RDWR);
    if(fd < 0) {
        printf(" open lcd  Failed !\n");
        return -1; 
    }

    fb_mem = (unsigned int*)mmap(NULL, 800 * 480 * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (fb_mem == MAP_FAILED) {
        printf(" mmap lcd Failed !\n");
        return -1;
    }

    struct v4l2_format fmt;
    struct v4l2_capability cap;
    if (-1 == ioctl(cam_fd, VIDIOC_QUERYCAP, &cap)) {
        printf("  camera  VIDIOC_QUERYCAP  Failed !\n");
        return -1;
    }
    printf("  camera  VIDIOC_QUERYCAP version : %x \n", cap.version);

    int index = 0;
    if (-1 == ioctl(cam_fd, VIDIOC_S_INPUT, &index)) {
        printf("  camera  VIDIOC_S_INPUT  Failed !\n");
        return -1;
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(cam_fd, VIDIOC_G_FMT, &fmt)) {
        printf("  camera  VIDIOC_G_FMT  Failed !\n");
        return -1;
    }
    printf("  camera  VIDIOC_G_FMT width : %d \n", fmt.fmt.pix.width);
    printf("  camera  VIDIOC_G_FMT height : %d \n", fmt.fmt.pix.height);

    if(V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat) {
        printf("  camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");
    }
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = 640;
    fmt.fmt.pix.height = 480;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
    if (-1 == ioctl(cam_fd, VIDIOC_S_FMT, &fmt)) {
        printf("  camera  VIDIOC_S_FMT  Failed !\n");
        return -1;
    }
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(cam_fd, VIDIOC_G_FMT, &fmt)) {
        printf("  final camera  VIDIOC_G_FMT  Failed !\n");
        return -1;
    }
    
    printf("  final camera  VIDIOC_G_FMT width : %d \n", fmt.fmt.pix.width);
    printf("  final camera  VIDIOC_G_FMT height : %d \n", fmt.fmt.pix.height);

    if(V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat) {
        printf("  final camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");
    }

    struct v4l2_requestbuffers reqbuf;
    memset(&reqbuf, 0, sizeof(reqbuf));
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    reqbuf.count = 4;
    if (-1 == ioctl(cam_fd, VIDIOC_REQBUFS, &reqbuf)) {
        printf("Video capturing or mmap-streaming is not supported\n");
        return -1;
    }
    buffers = calloc(reqbuf.count, sizeof(*buffers));
    struct v4l2_buffer buffer;

    for(i = 0; i < reqbuf.count; i++) {
        memset(&buffer, 0, sizeof(buffer));
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (-1 == ioctl(cam_fd, VIDIOC_QUERYBUF, &buffer)) {
            printf("Video VIDIOC_QUERYBUF failed !\n");
            return -1;
        }
        buffers[i].length = buffer.length; /* remember for munmap() */
        buffers[i].start = mmap(NULL, buffer.length,
                                PROT_READ | PROT_WRITE, /* required */
                                MAP_SHARED, /* recommended */
                                cam_fd, buffer.m.offset);

        if(buffers[i].start == MAP_FAILED) {
            printf("Video mmap failed !\n");
            return -1;
        }

        ret = ioctl(cam_fd, VIDIOC_QBUF, &buffer);
        printf("VIDIOC_QBUF buffer.index (%d) size  (%d)\n", buffer.index, buffer.length);
        if(ret < 0) {
            printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
            return -1;
        }
    }

    struct v4l2_buffer v4lbuf;
    memset(&v4lbuf, 0, sizeof(v4lbuf));
    v4lbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuf.memory = V4L2_MEMORY_MMAP;
    enum v4l2_buf_type vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(cam_fd, VIDIOC_STREAMON, &vtype)) {
        printf("Video VIDIOC_STREAMON failed !\n");
        return -1;
    }

    while(1) {

        for(i = 0; i < 4; i++) {
            v4lbuf.index = i;
            ret = ioctl(cam_fd, VIDIOC_DQBUF, &v4lbuf);
            if(ret < 0) {
                printf("VIDIOC_DQBUF (%d) failed (%d)\n", i, ret);
                return -1;
            }

            Showjpeg(buffers[v4lbuf.index].start, buffers[v4lbuf.index].length, fb_mem);
            if (flag) {

                 fdphoto = open(PIPE_PATH, O_WRONLY);
                if (fdphoto == -1) {
                perror("open");
                exit(EXIT_FAILURE);
                }
                pthread_mutex_lock(&lock);
                sprintf(pic_name, "photois%d.jpg", (int)time(NULL));
                clear_pipe(PIPE_PATH);
                write(fdphoto, pic_name, strlen(pic_name) + 1);
                
                jpg_fd = open(pic_name, O_WRONLY | O_CREAT, 0666);
                if (jpg_fd >= 0) {
                    write(jpg_fd, buffers[v4lbuf.index].start, buffers[v4lbuf.index].length);
                    close(jpg_fd);
                }
                flag = false;
                pthread_mutex_unlock(&lock);
            }

            ret = ioctl(cam_fd, VIDIOC_QBUF, &v4lbuf);
            if (ret < 0) {
                printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
                return -1;
            }
        }
    }

    vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == ioctl(cam_fd, VIDIOC_STREAMOFF, &vtype)) {
        printf("Video VIDIOC_STREAMOFF failed !\n");
        return -1;
    }

    for(i = 0; i < reqbuf.count; i++)
        munmap(buffers[i].start, buffers[i].length);

    ret = munmap(fb_mem, 800 * 480 * 4);
    if(ret == -1) {
        printf(" munmap  Failed !\n");
        return -1;
    }

    ret = close(fd);
    if(ret == -1) {
        printf(" close Failed !\n");
        return -1;
    }

    ret = close(cam_fd);
    if(ret == -1) {
        printf(" close  camera Failed !\n");
        return -1;
    }

    return 0;
}
void *read_rfid(void *arg) {
    int fd = *((int*)arg);

    while(1) {
        request_card(fd);
        int id = get_id(fd);

        if(id != -1 && id != 0 && id != 0xFFFFFFFF) {
            fprintf(stderr, "\r识别到卡号：%x，请稍等\n", id);
            pthread_mutex_lock(&lock);
            flag = true;
            pthread_mutex_unlock(&lock);
            sleep(1); 
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage: %s /dev/ttyUSB0\n", argv[0]);
        return -1;
    }

    signal(SIGALRM, refresh);
    pthread_mutex_init(&lock, NULL);

    int fd = open(argv[1], O_RDWR | O_NOCTTY);
    if(fd == -1) {
        printf("open %s failed: %s\n", argv[1], strerror(errno));
        return -1;
    }
    init_tty(fd);

    long state = fcntl(fd, F_GETFL);
    state |= O_NONBLOCK;
    fcntl(fd, F_SETFL, state);

    cam_fd = open("/dev/video0", O_RDWR);
    if(cam_fd < 0) {
        printf("open camera failed\n");
        return -1;
    }

    pthread_t wait_tid, rfid_tid;
    pthread_create(&wait_tid, NULL, waitting, NULL);
    pthread_create(&rfid_tid, NULL, read_rfid, &fd);

    captureV();

    pthread_join(wait_tid, NULL);
    pthread_join(rfid_tid, NULL);

    close(fd);
    pthread_mutex_destroy(&lock);

    return 0;
}
