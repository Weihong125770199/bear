
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

typedef unsigned char byte;
typedef unsigned short dbyte;
typedef unsigned long int dword;
typedef unsigned short word;
#pragma pack(push,2)  //保持2字节对齐
char video_path[100];

//输入YUV422I buffer数据，输出RGB buffer数据；
static void RGB888ToRGB888( unsigned char *src_buffer, unsigned char *des_buffer,int w, int h)
{
   memcpy(des_buffer,src_buffer,3*w*h);
}


//输入YUV422I buffer数据，输出RGB buffer数据；
static int YUYV422ToRGB888( unsigned char *src_buffer, unsigned char *des_buffer,int w, int h)
{
 unsigned char *yuv, *rgb;
 unsigned char u, v, y1, y2;
 
 yuv = src_buffer;
 rgb = des_buffer;
 
 if (yuv == NULL || rgb == NULL)
 {
  printf ("error: input data null!\n"); 
  return -1;
 }
 
 int size = w * h;
 
 for(int i = 0; i < size; i += 2)
 {
  y1 = yuv[2*i + 0];
  y2 = yuv[2*i + 2];
  u = yuv[2*i+1];
  v = yuv[2*i + 3];
  
  
 
#if 0
  rgb[3*i]     = (unsigned char)(y1 + 1.402*(u - 128));
  rgb[3*i + 1] = (unsigned char)(y1 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 2] = (unsigned char)(y1 + 1.772*(v - 128));
 
  rgb[3*i + 3] = (unsigned char)(y2 + 1.375*(u - 128));
  rgb[3*i + 4] = (unsigned char)(y2 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 5] = (unsigned char)(y2 + 1.772*(v - 128));
#endif 
 
  //为提高性能此处用移位运算；
  rgb[3*i]     = (unsigned char)(y1 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 1] = (unsigned char)(y1 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 2] = (unsigned char)(y1 + (v - 128) + ((199*(v - 128))>>8));
 
  rgb[3*i + 3] = (unsigned char)(y2 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 4] = (unsigned char)(y2 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 5] = (unsigned char)(y2 + (v - 128) + ((199*(v - 128))>>8));
 }
 
 return 0;
 
 
 
}


struct tagBITMAPFILEHEADER {
    //bmp file header
    dbyte bfType;        //文件类型
    dword bfSize;            //文件大小，字节为单位
    word bfReserved1;   //保留，必须为0
    word bfReserved2;   //保留，必须为0
    dword bfOffBits;         //从文件头开始的偏移量


    //bmp info head
    dword  biSize;            //该结构的大小
    dword  biWidth;           //图像的宽度，以像素为单位
    dword  biHeight;          //图像的高度，以像素为单位
    word biPlanes;          //为目标设备说明位面数，其值总是设为1
    word biBitCount;        //说明比特数/像素
    dword biCompression;     //图像数据压缩类型
    dword biSizeImage;       //图像大小，以字节为单位
    dword biXPelsPerMeter;   //水平分辨率，像素/米
    dword biYPelsPerMeter;   //垂直分辨率，同上
    dword biClrUsed;         //位图实际使用的彩色表中的颜色索引数
    dword biClrImportant;    //对图像显示有重要影响的颜色索引的数目
      
    //bmp rgb quad
     //对于16位，24位，32位的位图不需要色彩表
    //unsigned char rgbBlue;    //指定蓝色强度
    //unsigned char rgbGreen;   //指定绿色强度
    //unsigned char rgbRed;     //指定红色强度
    //unsigned char rgbReserved; //保留，设置为0 
}BMPFILEHEADER;

struct tagBITMAPFILEHEADER *bmp_p; 
void Init_bmp_head(int BitCount,int width,int height)
{
    
    bmp_p = &BMPFILEHEADER;
    bmp_p-> bfType = 0x4D42;    //文件类型
    bmp_p-> bfSize = (width*height*BitCount/8)+54;   //文件大小，字节为单位
    bmp_p-> bfReserved1 = 0x0;   //保留，必须为0
    bmp_p-> bfReserved2 = 0x0;   //保留，必须为0
    bmp_p-> bfOffBits = 0x36;         //从文件头开始的偏移量
 
    //bmp info head
    bmp_p-> biSize = 0x28;            //该结构的大小
    bmp_p-> biWidth = width;           //图像的宽度，以像素为单位
    bmp_p-> biHeight = height;          //图像的高度，以像素为单位
    bmp_p-> biPlanes = 0x01;          //为目标设备说明位面数，其值总是设为1
    bmp_p-> biBitCount = BitCount;        //说明比特数/像素
    bmp_p-> biCompression = 0;     //图像数据压缩类型
    bmp_p-> biSizeImage = width*height*BitCount /8;//0x09f8;       //图像大小，以字节为单位
    bmp_p-> biXPelsPerMeter = 0x0;//0x60;   //水平分辨率，像素/米
    bmp_p-> biYPelsPerMeter = 0x0;   //垂直分辨率，同上
    bmp_p-> biClrUsed = 0;         //位图实际使用的彩色表中的颜色索引数
    bmp_p-> biClrImportant = 0;    //对图像显示有重要影响的颜色索引的数目
     
}

/*  Print Four-character-code (FOURCC) */
static char *print_fourcc(__u32 fmt)
{
        static char code[5];

        code[0] = (unsigned char)(fmt & 0xff);
        code[1] = (unsigned char)((fmt >> 8) & 0xff);
        code[2] = (unsigned char)((fmt >> 16) & 0xff);
        code[3] = (unsigned char)((fmt >> 24) & 0xff);
        code[4] = '\0';

        return code;
}

int main(int count,char **argv ){
  //////
  int x=0;
  int y=0;
  int ret= 0;
  __u32 current_fmt =0;
  unsigned char *image_rgb;
  sprintf(video_path,"/dev/video%s",argv[1]);
  printf("opening %s \n",video_path);
  int fd = open(video_path,O_RDWR);
  printf("TK------->>>fd is %d\n",fd);
  //////
  struct v4l2_capability cap;
  ioctl(fd,VIDIOC_QUERYCAP,&cap);
  printf("TK---------->>>>>Driver Name:%s\nCard Name:%s\nBus info:%s\n",cap.driver,cap.card,cap.bus_info);
  //////
  struct v4l2_fmtdesc fmtdesc;
  fmtdesc.index = 0; fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while(ioctl(fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){
   printf("TK-------->>>>>fmtdesc.description is %s\n",fmtdesc.description);
   fmtdesc.index ++;
  }
  //////
  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(fd,VIDIOC_G_FMT,&fmt);
  printf("TK----------->>>>>fmt.fmt.width is %d\nfmt.fmt.pix.height is %d\nfmt.fmt.pix.colorspace is %d  fmt.fmt.pix.pixelformat=%s\n",fmt.fmt.pix.width,fmt.fmt.pix.height,fmt.fmt.pix.colorspace,print_fourcc(fmt.fmt.pix.pixelformat));

  /////
#if 0
  fmt.fmt.pix.height =480;
  fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_RGB24;
  ret=ioctl(fd,VIDIOC_S_FMT,&fmt);
  printf("TK----------->>>>> set fmt,ret=%d \n",ret);

  ioctl(fd,VIDIOC_G_FMT,&fmt);
  printf("TK----------->>>>>after set fmt.fmt.width is %d\nfmt.fmt.pix.height is %d\nfmt.fmt.pix.colorspace is %d  fmt.fmt.pix.pixelformat=%s\n",fmt.fmt
.pix.width,fmt.fmt.pix.height,fmt.fmt.pix.colorspace,print_fourcc(fmt.fmt.pix.pixelformat));

#endif
  x=fmt.fmt.pix.width;
  y=fmt.fmt.pix.height;
  current_fmt=fmt.fmt.pix.pixelformat;
  Init_bmp_head(24,x,y);
  //////
  struct v4l2_requestbuffers req;
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  printf("TK------------------VIDIOC_REQBUFS---START \n");
  ioctl(fd,VIDIOC_REQBUFS,&req);
  printf("TK------------------VIDIOC_REQBUFS---END \n");
  struct buffer{
    void *start;
    unsigned int length;
  }*buffers;
  buffers = (struct buffer*)calloc (req.count, sizeof(*buffers));
  unsigned int n_buffers = 0;
  for(n_buffers = 0; n_buffers < req.count; ++n_buffers){
    struct v4l2_buffer buf;
    memset(&buf,0,sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = n_buffers;

     printf("TK------------------VIDIOC_QUERYBUF---START \n");
    if(ioctl(fd,VIDIOC_QUERYBUF,&buf) == -1){
      printf("TK---------_>>>>>>error\n");
      close(fd);
      exit(-1);
    }
     printf("TK------------------VIDIOC_QUERYBUF---END buf.start=0x%x ,lenth=%d\n",buf.index ,buf.length);
    buffers[n_buffers].length = buf.length;
    buffers[n_buffers].start = mmap(NULL,buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,fd,buf.m.offset);
    if(MAP_FAILED == buffers[n_buffers].start){
      printf("TK--------__>>>>>error 2\n");
      close(fd);
      exit(-1);
    }
  }
  
  ////
  unsigned int i;
  enum v4l2_buf_type type;
  for(i = 0; i < 4; i++){
    struct v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
     printf("TK------------------VIDIOC_QBUF---START \n");
    ioctl(fd,VIDIOC_QBUF,&buf);
     printf("TK------------------VIDIOC_QBUF---END \n");
  }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  printf("TK------------------VIDIOC_STREAMON---START \n");
  ioctl(fd,VIDIOC_STREAMON,&type);
  printf("TK------------------VIDIOC_STREAMON---END \n");
  ////

  for(i=0;i<4;i++)
  {
  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index=i;
  printf("TK------------------VIDIOC_DQBUF---START \n");
  ioctl(fd,VIDIOC_DQBUF,&buf);
  printf("TK------------------VIDIOC_DQBUF---END \n");
  image_rgb=malloc(x*y*3+54);
  memset(image_rgb,0x0,x*y*3+54);
  memcpy(image_rgb,bmp_p,sizeof(BMPFILEHEADER));
  //YV12ToBGR24_Native(buffers[buf.index].start,image_rgb+54,x,y);
  //YCbCrToRGB24_Native(buffers[buf.index].start,image_rgb+54,x,y);
  if(current_fmt == V4L2_PIX_FMT_YUYV )
  {
    YUYV422ToRGB888(buffers[buf.index].start,image_rgb+54,x,y);
  } else if(current_fmt == V4L2_PIX_FMT_RGB24 )
  {
     RGB888ToRGB888(buffers[buf.index].start,image_rgb+54,x,y);
  }
 
  char path[20];
  snprintf(path,sizeof(path),"./%d.bmp",buf.index);
  int fdyuyv = open(path,O_WRONLY|O_CREAT,00700);
  printf("TK--------->>>>fd bmp is %d\n",fdyuyv);
  int resultyuyv = write(fdyuyv,image_rgb,x*y*3+54);
  printf("TK--------->>>resultbmp is %d\n",resultyuyv); 
  free(image_rgb);
  close(fdyuyv);
  ////
  }
  //free(bmp_p);
  close(fd);
  return 0;
}
