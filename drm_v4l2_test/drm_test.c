#define _FILE_OFFSET_BITS 64
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "error.h"
#include <fcntl.h>
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
  
  
 
#if 1
  rgb[3*i]     = (unsigned char)(y1 + 1.402*(u - 128));
  rgb[3*i + 1] = (unsigned char)(y1 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 2] = (unsigned char)(y1 + 1.772*(v - 128));
 
  rgb[3*i + 3] = (unsigned char)(y2 + 1.375*(u - 128));
  rgb[3*i + 4] = (unsigned char)(y2 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 5] = (unsigned char)(y2 + 1.772*(v - 128));
#endif 
 
#if 0
  //为提高性能此处用移位运算；
  rgb[3*i]     = (unsigned char)(y1 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 1] = (unsigned char)(y1 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 2] = (unsigned char)(y1 + (v - 128) + ((199*(v - 128))>>8));
 
  rgb[3*i + 3] = (unsigned char)(y2 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 4] = (unsigned char)(y2 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 5] = (unsigned char)(y2 + (v - 128) + ((199*(v - 128))>>8));
#endif
 }
 
 return 0;
 
 
 
}

//输入YUV422I buffer数据，输出RGB buffer数据；
static int YUYV422ToRGB888_no_color( unsigned char *src_buffer, unsigned char *des_buffer,int w, int h)
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
  u = 128;
  v = 128;
  
  
 
#if 1
  rgb[3*i]     = (unsigned char)(y1 + 1.402*(u - 128));
  rgb[3*i + 1] = (unsigned char)(y1 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 2] = (unsigned char)(y1 + 1.772*(v - 128));
 
  rgb[3*i + 3] = (unsigned char)(y2 + 1.375*(u - 128));
  rgb[3*i + 4] = (unsigned char)(y2 - 0.344*(u - 128) - 0.714*(v - 128));
  rgb[3*i + 5] = (unsigned char)(y2 + 1.772*(v - 128));
#endif 
 
#if 0
  //为提高性能此处用移位运算；
  rgb[3*i]     = (unsigned char)(y1 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 1] = (unsigned char)(y1 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 2] = (unsigned char)(y1 + (v - 128) + ((199*(v - 128))>>8));
 
  rgb[3*i + 3] = (unsigned char)(y2 + (u - 128) + ((104*(u - 128))>>8));
  rgb[3*i + 4] = (unsigned char)(y2 - (89*(v - 128)>>8) - ((183*(u - 128))>>8));
  rgb[3*i + 5] = (unsigned char)(y2 + (v - 128) + ((199*(v - 128))>>8));
#endif
 }
 
 return 0;
 
 
 
}


//输入YUV422I buffer数据，输出RGB buffer数据；
static int diff_image( unsigned char *old_buffer, unsigned char *new_buffer,unsigned char *result_buffer,int w, int h)
{

 int i=0;
 for(i=0;i< w*h;i++)
 {
  *(result_buffer+3*i) = *(new_buffer+3*i) - *(old_buffer+3*i);
  *(result_buffer+3*i+1) = *(new_buffer+3*i+1) - *(old_buffer+3*i+1);
  *(result_buffer+3*i+2) = *(new_buffer+3*i+2) - *(old_buffer+3*i+2);

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











drmModeConnector* FindConnector(int fd)
   {
       drmModeRes *resources = drmModeGetResources(fd); //drmModeRes描述了计算机所有的显卡信息：connector，encoder，crtc，modes等。
       if (!resources)
         {
            return NULL;
         }
 
       drmModeConnector* conn = NULL;
       int i = 0;
       for (i = 0; i < resources->count_connectors; i++)
           {
               conn = drmModeGetConnector(fd, resources->connectors[i]);
               if (conn != NULL)
                  {
                     //找到处于连接状态的Connector。
                     if (conn->connection == DRM_MODE_CONNECTED && conn->count_modes > 0 && conn != NULL)
                        {
                                     break;
                        }else
                        {
                                     drmModeFreeConnector(conn);
                        }
                  }
           }
 
        drmModeFreeResources(resources);
        return conn;
   }
 


int FindCrtc(int fd, drmModeConnector *conn)
   {
            drmModeRes *resources = drmModeGetResources(fd);
            if (!resources)
               {
                   fprintf(stderr, "drmModeGetResources failed\n");
                   return -1;
               }
               

             int i=0;
             int j=0;
             printf("get number of encoder=%d \n",resources->count_encoders); 
             for (i = 0; i < resources->count_encoders; ++i)
               {
                     drmModeEncoder *enc = drmModeGetEncoder(fd, resources->encoders[i]);
                     printf("get mode encoder enc %d \n",i); 
                     if (NULL != enc)
                        {
                           for (j = 0; j < resources->count_crtcs; ++j)
                               {
                                   // connector下连接若干encoder，每个encoder支持若干crtc
                                   //，possible_crtcs的某一位为1代表相应次序（不是id哦）的crtc可用。
                                   printf("connector[%d].encoder[%d].possible_crtcs=0x%x \n",i,j,enc->possible_crtcs);
                                    if ((enc->possible_crtcs & (1 << j)))
                                        {
                                             int id = resources->crtcs[j];
                                             printf("got crtcId =0x%x \n",id);
                                             drmModeFreeEncoder(enc);
                                             drmModeFreeResources(resources);
                                             return id;
                                        }
                                 }
 
                          drmModeFreeEncoder(enc);
                          }else
                          {
                            printf("get %d/%d  encoder fail \n",i+1,conn->count_encoders);
                          }
                 }
 
                 drmModeFreeResources(resources);
                 return -1;
   }
 
void SetColor(unsigned char *dest, int stride, int w, int h,unsigned char *src,int src_w,int src_h,int start_x ,int start_y)
{
     struct color {
                    unsigned r, g, b;
                  };
 
     int x=0;
     int y=0;

     for(y=0;y<src_h;y++)
         {
            for(x=0;x<src_w;x++)
             {

               #if 0
               *(dest+(y*h+x)*3) = *(dest+(y*src_h+x)*3);
               *(dest+(y*h+x)*3 + 1) = *(dest+(y*src_h+x)*3 + 1);
               *(dest+(y*h+x)*3 + 2) =   *(dest+(y*src_h+x)*3 + 2);
               #endif

 
               #if 1
               
               *(dest+((y+start_y)*w+x+start_x)*4) =    *(src+(y*src_w+x)*3 ) ; //blue
               *(dest+((y+start_y)*w+x+start_x)*4+1) =  *(src+(y*src_w+x)*3 + 1) ;// *(src+(y*src_w+x)*3); //green
               *(dest+((y+start_y)*w+x+start_x)*4 + 2) = *(src+(y*src_w+x)*3 +2 );//*(src+(y*src_w+x)*3 + 1);  //red
               *(dest+((y+start_y)*w+x+start_x)*4 + 3) = 0xff;// 0xff;//*(src+(y*src_w+x)*3);// *(src+(y*src_w+x)*3 + 2);
               #endif

                  


             }





         }
 
           
 
printf("draw picture\n");
}


int main(int argc, char *argv[])
{
    int ret, fd;
    int x=0;
    int y=0;

    drm_magic_t magic;
    fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC | O_NONBLOCK);
    if (fd < 0)
    {
          /* Probably permissions error */
           fprintf(stderr, "couldn't open %s, skipping\n", "");
           return -1;
    }

    //drmSetMaster(fd);
    drmGetMagic(fd,&magic);
    ret = drmAuthMagic(fd,magic);
    printf("get author ret=%d \n",ret);
 
    drmModeConnectorPtr connector = (drmModeConnectorPtr *) FindConnector(fd);
    int width = connector->modes[0].hdisplay;
    int height = connector->modes[0].vdisplay;


    printf("display is %d*%d.\n", width, height);
 
    int crtcid = FindCrtc(fd, connector);

    printf("get crtcid =%d \n",crtcid);
 
    struct drm_mode_create_dumb creq;
    memset(&creq, 0, sizeof(creq));
 
    creq.width = width;
    creq.height = height;
    creq.bpp = 32;
    creq.flags = 0;
 
    ret = drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq);
    if (ret)
       {
           printf("create dumb failed!\n");
       }

    uint32_t framebuffer = -1;
    uint32_t stride = creq.pitch;
 
    //使用缓存的handel创建一个FB，返回fb的id：framebuffer。
    ret = drmModeAddFB(fd, width, height, 24, 32, creq.pitch, creq.handle, &framebuffer);
 
    if (ret)
       {
           printf("failed to create fb\n");
           return -1;
       }
    struct drm_mode_map_dumb mreq; //请求映射缓存到内存。
    mreq.handle = creq.handle;
 
    ret = drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq);
    if (ret)
       {
          printf("map dumb failed!\n");
       }

     // 猜测：创建的缓存位于显存上，在使用之前先使用drm_mode_map_dumb将其映射到内存空间。
     // 但是映射后缓存位于内核内存空间，还需要一次mmap才能被程序使用。
    unsigned char* buf = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (buf == MAP_FAILED)
      {
          printf("mmap failed!\n");
      }

    memset(buf, 255, creq.size);
 
    //一切准备完毕，只差连接在一起了！
    ret = drmModeSetCrtc(fd, crtcid, framebuffer, 0, 0, &connector->connector_id, 1, connector->modes);
    if (ret)
       {
          fprintf(stderr, "failed to set mode: %d  erron=%s\n",ret,strerror(errno));
          return -1;
       }









  __u32 current_fmt =0;
  unsigned char *image_rgb;
  unsigned char *old_rgb;
  unsigned char *result_rgb;
  sprintf(video_path,"/dev/video%s",argv[1]);
  printf("opening %s \n",video_path);
  int viedo_fd = open(video_path,O_RDWR);
  printf("TK------->>>viedo_fd is %d\n",viedo_fd);
  //////
  struct v4l2_capability cap;
  ioctl(viedo_fd,VIDIOC_QUERYCAP,&cap);
  printf("TK---------->>>>>Driver Name:%s\nCard Name:%s\nBus info:%s\n",cap.driver,cap.card,cap.bus_info);
  //////
  struct v4l2_fmtdesc fmtdesc;
  fmtdesc.index = 0; fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  while(ioctl(viedo_fd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){
   printf("TK-------->>>>>fmtdesc.description is %s\n",fmtdesc.description);
   fmtdesc.index ++;
  }
  //////
  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(viedo_fd,VIDIOC_G_FMT,&fmt);
  printf("TK----------->>>>>fmt.fmt.width is %d\nfmt.fmt.pix.height is %d\nfmt.fmt.pix.colorspace is %d  fmt.fmt.pix.pixelformat=%s\n",fmt.fmt.pix.width,fmt.fmt.pix.height,fmt.fmt.pix.colorspace,print_fourcc(fmt.fmt.pix.pixelformat));

  /////
#if 0
  fmt.fmt.pix.height =480;
  fmt.fmt.pix.pixelformat= V4L2_PIX_FMT_RGB24;
  ret=ioctl(viedo_fd,VIDIOC_S_FMT,&fmt);
  printf("TK----------->>>>> set fmt,ret=%d \n",ret);

  ioctl(viedo_fd,VIDIOC_G_FMT,&fmt);
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
  ioctl(viedo_fd,VIDIOC_REQBUFS,&req);
  printf("TK------------------VIDIOC_REQBUFS---END \n");
  struct buffer{
    void *start;
    unsigned int length;
  }*buffers;
  buffers = (struct buffer*)calloc (req.count, sizeof(*buffers));
  unsigned int n_buffers = 0;
  for(n_buffers = 0; n_buffers < req.count; ++n_buffers){
    struct v4l2_buffer v4l2_buf;
    memset(&v4l2_buf,0,sizeof(v4l2_buf));
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    v4l2_buf.index = n_buffers;

     printf("TK------------------VIDIOC_QUERYBUF---START \n");
    if(ioctl(viedo_fd,VIDIOC_QUERYBUF,&v4l2_buf) == -1){
      printf("TK---------_>>>>>>error\n");
      close(viedo_fd);
      exit(-1);
    }
     printf("TK------------------VIDIOC_QUERYBUF---END v4l2_buf.start=0x%x ,lenth=%d\n",v4l2_buf.index ,v4l2_buf.length);
    buffers[n_buffers].length = v4l2_buf.length;
    buffers[n_buffers].start = mmap(NULL,v4l2_buf.length,PROT_READ|PROT_WRITE,MAP_SHARED,viedo_fd,v4l2_buf.m.offset);
    if(MAP_FAILED == buffers[n_buffers].start){
      printf("TK--------__>>>>>error 2\n");
      close(viedo_fd);
      exit(-1);
    }
  }
  
  ////
  unsigned int i;
  enum v4l2_buf_type type;
#if 0
  for(i = 0; i < 4; i++){
    struct v4l2_buffer v4l2_buf;
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    v4l2_buf.index = i;
     printf("TK------------------VIDIOC_QBUF---START \n");
    ioctl(viedo_fd,VIDIOC_QBUF,&v4l2_buf);
     printf("TK------------------VIDIOC_QBUF---END \n");
  }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  printf("TK------------------VIDIOC_STREAMON---START \n");
  ioctl(viedo_fd,VIDIOC_STREAMON,&type);
  printf("TK------------------VIDIOC_STREAMON---END \n");
  ////
#endif

 image_rgb=malloc(x*y*3+54);
 old_rgb=malloc(x*y*3);
 result_rgb = malloc(x*y*3);
while(1)
{

#if 1
for(i = 0; i < 4; i++){
    struct v4l2_buffer v4l2_buf;
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    v4l2_buf.index = i;
    ioctl(viedo_fd,VIDIOC_QBUF,&v4l2_buf);
     
  }
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(viedo_fd,VIDIOC_STREAMON,&type);
  ////
#endif
  for(i=0;i<4;i++)
  {
  struct v4l2_buffer v4l2_buf;
  v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  v4l2_buf.memory = V4L2_MEMORY_MMAP;
  v4l2_buf.index=i;
  
  ioctl(viedo_fd,VIDIOC_DQBUF,&v4l2_buf);
  //image_rgb=malloc(x*y*3+54);
  //old_rgb=malloc(x*y*3);
  //result_rgb = malloc(x*y*3);
  memset(image_rgb,0x0,x*y*3+54);
  memcpy(image_rgb,bmp_p,sizeof(BMPFILEHEADER));
  //YV12ToBGR24_Native(buffers[v4l2_buf.index].start,image_rgb+54,x,y);
  //YCbCrToRGB24_Native(buffers[v4l2_buf.index].start,image_rgb+54,x,y);
  if(current_fmt == V4L2_PIX_FMT_YUYV )
  {
    YUYV422ToRGB888(buffers[v4l2_buf.index].start,image_rgb+54,x,y);
    SetColor(buf, stride, width, height,image_rgb+54,x,y,0,0);
    YUYV422ToRGB888_no_color(buffers[v4l2_buf.index].start,image_rgb+54,x,y);
    SetColor(buf, stride, width, height,image_rgb+54,x,y,x+1,0);
   //weihong
    diff_image(old_rgb,image_rgb+54,result_rgb,x,y);
    memcpy(old_rgb,image_rgb+54,x*y*3);
    SetColor(buf, stride, width, height,result_rgb,x,y,0,y+1);
 
    drmModePageFlip(fd, crtcid, framebuffer, DRM_MODE_PAGE_FLIP_EVENT, 0);
    
    //getchar();
  } else if(current_fmt == V4L2_PIX_FMT_RGB24 )
  {
     RGB888ToRGB888(buffers[v4l2_buf.index].start,image_rgb+54,x,y);
  }
 
  //free(image_rgb);
  
  ////
  }
  getchar();
}
  getchar();
  //free(bmp_p);
  close(viedo_fd);
  close(fd);
  exit(0);
  return ret;
}

