#include <stdio.h>
#include "jpeglib.h"
#include "jconfig.h"
#include <stdlib.h>

int Showjpeg (char * jpegdata,int size,unsigned int *fb_mem)
{

	//一、为JPEG对象分配空间并初始化
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);


	//二、指定数据源

	jpeg_mem_src(&cinfo, jpegdata,size);


	//三、获取文件信息
	(void) jpeg_read_header(&cinfo, TRUE);

	//四、解压
	(void) jpeg_start_decompress(&cinfo);

	/* JSAMPLEs per row in output buffer */
	
	// printf("\routput_width  (%d) output_components (%d)\n", cinfo.output_width , cinfo.output_components);
	fprintf(stderr , "\r output_width  (%d) output_components (%d)", cinfo.output_width , cinfo.output_components);

	//五、取出数据
	int x,i;
	char *buffer;
	int row_stride = cinfo.output_width * cinfo.output_components;
	//开辟一行数据空间
	buffer = malloc(row_stride);

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */

 
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		(void) jpeg_read_scanlines(&cinfo, (JSAMPARRAY)&buffer, 1);
		// 将24bit RGB 刷新到LCD
		//将bmp_buf 24bit的图像数据写入32bit的fb_mem

		i=0;
		for(x=0;x<cinfo.output_width;x++)
		{
			*(fb_mem+80+(cinfo.output_scanline-1)*800+x) =  buffer[i]<<16|buffer[i+1]<<8|buffer[i+2];
				i+=3;
		}
	}

	 //六、解压完毕
	(void) jpeg_finish_decompress(&cinfo);
	//七、释放资源
	 jpeg_destroy_decompress(&cinfo);
  
  return 0;
}



