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
#include "jpeg.h"

unsigned int *fb_mem;
struct {
	void *start;
	size_t length;
} *buffers;

//µ÷ÓÃ·½Ê½£ºcaptureV(argc, argv)
int captureV(int argc,char **argv)
{

	int fd,cam_fd,jpg_fd;
	int ret,jpg_size;
	int x,y,i=0;
	char pic_name[10];
	char *jpg_mem;

	fd = open("/dev/fb0", O_RDWR);
	if(fd < 0)
	{
		printf(" open lcd  Failed !\n");
		return -1; 
	}

	fb_mem = (unsigned int*)mmap(NULL, 800*480*4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	cam_fd = open(argv[1],O_RDWR);
	if(cam_fd < 0)
	{
		printf(" open  camera Failed !\n");
		return -1; 	
	}
	struct v4l2_format fmt;
	struct v4l2_capability cap;
	if (-1 == ioctl (cam_fd, VIDIOC_QUERYCAP, &cap))
	{
		printf("  camera  VIDIOC_QUERYCAP  Failed !\n");
		return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£			
	}
	printf("  camera  VIDIOC_QUERYCAP version : %x \n",cap.version);

	int index;
	index = 0;
	if (-1 == ioctl (cam_fd, VIDIOC_S_INPUT, &index)) {
		printf("  camera  VIDIOC_S_INPUT  Failed !\n");
		return -1;
	}
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_G_FMT, &fmt))
	{
		printf("  camera  VIDIOC_G_FMT  Failed !\n");
		return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£	
	}
	printf("  camera  VIDIOC_G_FMT width : %d \n",fmt.fmt.pix.width);
	printf("  camera  VIDIOC_G_FMT height : %d \n",fmt.fmt.pix.height );

	if( V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat )
	printf("  camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_JPEG;
	if (-1 == ioctl (cam_fd, VIDIOC_S_FMT, &fmt))
		{
		printf("  camera  VIDIOC_S_FMT  Failed !\n");
		return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£	
		}
	memset(&fmt,0,sizeof(fmt));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_G_FMT, &fmt))
		{
		printf("  final camera  VIDIOC_G_FMT  Failed !\n");
		return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£	
		}
	
	printf("  final camera  VIDIOC_G_FMT width : %d \n",fmt.fmt.pix.width);
	printf("  final camera  VIDIOC_G_FMT height : %d \n",fmt.fmt.pix.height );

	if( V4L2_PIX_FMT_JPEG == fmt.fmt.pix.pixelformat )
	printf("  final camera  VIDIOC_G_FMT pixelformat : V4L2_PIX_FMT_JPEG \n");
	struct v4l2_requestbuffers reqbuf;
	memset (&reqbuf, 0, sizeof (reqbuf));
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqbuf.memory = V4L2_MEMORY_MMAP;
	reqbuf.count = 4;
	if (-1 == ioctl (cam_fd, VIDIOC_REQBUFS, &reqbuf)) {
		
		printf ("Video capturing or mmap-streaming is not supported\n");
		return -1;
	}
	buffers = calloc (reqbuf.count, sizeof (*buffers));
	struct v4l2_buffer buffer;

	for (i = 0; i < reqbuf.count; i++) {
		

		memset (&buffer, 0, sizeof (buffer));
		buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buffer.memory = V4L2_MEMORY_MMAP;
		buffer.index = i;
		
		if (-1 == ioctl (cam_fd, VIDIOC_QUERYBUF, &buffer)) {
			printf ("Video VIDIOC_QUERYBUF failed !\n");
			return -1;
		}
		buffers[i].length = buffer.length; /* remember for munmap() */
		buffers[i].start = mmap (NULL, buffer.length,
							PROT_READ | PROT_WRITE, /* required */
							MAP_SHARED, /* recommended */
							cam_fd, buffer.m.offset);


		if (buffers[i].start == MAP_FAILED) {
			printf ("Video mmap failed !\n");
			return -1;
		}

		ret = ioctl(cam_fd , VIDIOC_QBUF, &buffer);
		printf("VIDIOC_QBUF  buffer.index (%d) size  (%d)\n", buffer.index, buffer.length);
		if (ret < 0) {
		printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
			 return -1;
		}
	}
	struct v4l2_buffer v4lbuf;
	memset(&v4lbuf,0,sizeof(v4lbuf));
	v4lbuf.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
	v4lbuf.memory=V4L2_MEMORY_MMAP;
	enum v4l2_buf_type vtype= V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_STREAMON, &vtype))
	{
		printf ("Video VIDIOC_STREAMON failed !\n");
		return -1;
	}
	sleep(1);
	while(1)
	{
		for(i=0;i<4;i++)
		{
			v4lbuf.index = i;
			ret = ioctl(cam_fd , VIDIOC_DQBUF, &v4lbuf);
			if (ret < 0)
			{
				printf("VIDIOC_DQBUF (%d) failed (%d)\n", i, ret);
				 return -1;
			}
			
			Showjpeg(buffers[v4lbuf.index].start,buffers[v4lbuf.index].length,fb_mem);			
			ret = ioctl(cam_fd , VIDIOC_QBUF, &v4lbuf);
		//	printf("VIDIOC_QBUF  buffer.index (%d) size  (%d)\n", buffer.index, buffer.length);
			if (ret < 0) {
				printf("VIDIOC_QBUF (%d) failed (%d)\n", i, ret);
				return -1;
			}
		}
	}
	close(jpg_fd);
	vtype = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (-1 == ioctl (cam_fd, VIDIOC_STREAMOFF, &vtype))
	{
		printf ("Video VIDIOC_STREAMOFF failed !\n");
		return -1;
	}

	/* Cleanup. */
	for (i = 0; i < reqbuf.count; i++)
	munmap (buffers[i].start, buffers[i].length);

	ret = munmap(fb_mem, 800*480*4);
	if(ret == -1)
		{
		printf(" munmap  Failed !\n");
		return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£
		}

	ret = close(fd);
		if(ret == -1)
		{
			printf(" close Failed !\n");
			return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£
		}
	ret = close(cam_fd);
		if(ret == -1)
		{
			printf(" close  camera Failed !\n");
			return -1; //ï¿½ï¿½ï¿½ï¿½ï¿½ì³£
		}

	
	return 0;	 //ï¿½ï¿½ï¿½Ø³É¹ï¿½
}
