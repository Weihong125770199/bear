#include <stdio.h>
#include <linux/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include "accessory_authentication.h"

struct i2c_rdwr_ioctl_data mfi_chip;
#define SLAVE_ADDRESS1 0X11
#define SLAVE_ADDRESS2 0X10
unsigned int slave_address = 0x11;

#define REPEATEDLY_OPERATE   5  /* 从设备忙时重复操作的次数 */
#define MFI_OPERATE_ERROR   -0XFFF
#define CHECK_ERROR_CODE    1

struct chip_reg_t{
    unsigned char reg_addr;
    unsigned char* buff;
    int rd_wr_len;
    unsigned char rd_wr_flag;/* 0 write, 1 = read */
};


/*
argument:
unsigned int addr: 0x20 or 0x22
return :
succeeded: slave address
failure: -1
*/
int set_mfi_chip_slave_addr(unsigned int addr)
{
    if((addr == SLAVE_ADDRESS2) || (addr == SLAVE_ADDRESS1))
    {
        slave_address = addr;
        return slave_address;
    }
    else
    {
        return -1;
    }
}

static int rd_wr_mfi_chip(int fd, struct chip_reg_t* chip_reg)
{
    unsigned char buf[2];
    int i;
    int ret;
    
    mfi_chip.nmsgs = 1;
    mfi_chip.msgs = (struct i2c_msg *)malloc(mfi_chip.nmsgs * sizeof(struct i2c_msg));
    
    ioctl(fd, I2C_TIMEOUT, 1);  /* 设置超时时间 */
    //ioctl(fd, I2C_RETRIES, 1);/* 设置重发次数 */

    /* write reg addr */
    mfi_chip.msgs[0].len = 1;   /* 信息长度为1，即读/写的数据长度 */
    mfi_chip.msgs[0].addr = slave_address;
    mfi_chip.msgs[0].flags = 0; /* 写命令 */
    mfi_chip.msgs[0].buf = buf;
    mfi_chip.msgs[0].buf[0] = chip_reg->reg_addr;
    for(i=0; i<REPEATEDLY_OPERATE; i++)/* mfi 2.0C版芯片要求: NACK, 延时500us重试 */
    {
        ret = ioctl (fd, I2C_RDWR, (unsigned long)&mfi_chip);
        if(ret >= 0)
        {
            break;
        }
        else
        {
            usleep(500);
        }
    }
    if(ret < 0)
    {
        printf("MFI: write chip reg addr err. reg=0x%x, ret=%d\n",chip_reg->reg_addr, ret);
        free(mfi_chip.msgs);
        return ret;
    }

    /* register read/write operate */
    mfi_chip.nmsgs = 1;
    mfi_chip.msgs[0].len = chip_reg->rd_wr_len;
    mfi_chip.msgs[0].addr = slave_address;
    mfi_chip.msgs[0].flags = chip_reg->rd_wr_flag;/* 读写命令 */
    mfi_chip.msgs[0].buf = chip_reg->buff;
    for(i=0; i<REPEATEDLY_OPERATE; i++)
    {
        ret = ioctl (fd, I2C_RDWR, (unsigned long)&mfi_chip);
        if(ret < 0)
        {
            usleep(500);
        }
        else
        {
            break;
        }
    }
    if(ret < 0)
    {
        printf("MFI: read mfi reg err. reg=0x%x, ret = %d\n",chip_reg->reg_addr, ret);
        free(mfi_chip.msgs);
        return ret;
    }
    free(mfi_chip.msgs);

    return ret;
}

static int write_mfi_chip(int fd, struct chip_reg_t* chip_reg)
{
    unsigned char buf[1100];
    int i;
    int ret;
    
    mfi_chip.nmsgs = 1;
    mfi_chip.msgs = (struct i2c_msg *)malloc(mfi_chip.nmsgs * sizeof(struct i2c_msg));
    
    ioctl(fd, I2C_TIMEOUT, 1);  /* 设置超时时间 */
    //ioctl(fd, I2C_RETRIES, 1);/* 设置重发次数 */

    /* write reg addr */
    mfi_chip.msgs[0].len = 1+chip_reg->rd_wr_len;   /* 信息长度为1，即读/写的数据长度 */
    mfi_chip.msgs[0].addr = slave_address;
    mfi_chip.msgs[0].flags = 0; /* 写命令 */
    mfi_chip.msgs[0].buf = buf;
    mfi_chip.msgs[0].buf[0] = chip_reg->reg_addr;
    for(i=0; i<chip_reg->rd_wr_len; i++)
    {
        mfi_chip.msgs[0].buf[i+1] = chip_reg->buff[i];
    }
    for(i=0; i<REPEATEDLY_OPERATE; i++)/* mfi 2.0C版芯片要求: NACK, 延时500us重试 */
    {
        ret = ioctl (fd, I2C_RDWR, (unsigned long)&mfi_chip);
        if(ret >= 0)
        {
            break;
        }
        else
        {
            usleep(500);
        }
    }
    if(ret < 0)
    {
        printf("MFI: write chip reg addr err. reg=0x%x, ret=%d\n",chip_reg->reg_addr, ret);
        free(mfi_chip.msgs);
        return ret;
    }

    return ret;
}


/*
argument:
u8 *certificate_data: max 1280 bytes. read from MFI coprocessor. will send to apple devices
int length: certificate data length

return:
int: coprocessor read/write status
*/
int req_authentication_certificate(unsigned char* certificate_data, 
                                                int* length)
{
    unsigned char* index;
    int i;
    int ret;
    int certificate_data_length = 0; 
    unsigned char part_amount = 0;
    unsigned int fd;
    struct chip_reg_t chip_reg;
    unsigned char buf[2] = {0};
    unsigned char mfi_chip_error_code = 0x00;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file. i2c-1\n", __LINE__);
		return -1;
	}

    #ifdef CHECK_ERROR_CODE
    read_mfi_chip_err_code(&mfi_chip_error_code);/* clear error code */
    mfi_chip_error_code = 0;
    #endif
    
    *length = -1;
	/* read mfi CERTIFICATE_DATA_LENGTH */
    chip_reg.reg_addr = ACCESSORY_CERTIFICATE_DATA_LENGTH;
    chip_reg.rd_wr_len = 2;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = buf;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }
    certificate_data_length = (buf[0]<<8) & 0x700;
    certificate_data_length += buf[1];
    printf("certificate data length=0x%x 0x%x\n",buf[0],buf[1]);
    if(certificate_data_length > 1280)
    {
        printf("certificate data length error=%d\n",certificate_data_length);
        return MFI_OPERATE_ERROR;
    }
    
    /* read mfi CERTIFICATE_DATA */
	index = certificate_data;
    part_amount = certificate_data_length/128;
    if(certificate_data_length%128 != 0)
    {
        part_amount++;
    }
    if(part_amount > 10)
    {
        part_amount = 10;
    }
    for(i=0; i<part_amount; i++)
    {
        chip_reg.reg_addr = i+ACCESSORY_CERTIFICATE_DATA_PART1;
        if(i == part_amount-1)
        {
            chip_reg.rd_wr_len = certificate_data_length%128;
            if(chip_reg.rd_wr_len == 0)
            {
                chip_reg.rd_wr_len = 128;
            }
        }
        else
        {
       	    chip_reg.rd_wr_len = 128;
       	}
        chip_reg.rd_wr_flag = I2C_M_RD;
        chip_reg.buff = index + i*128;
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: read reg addr err. reg=0x%x, ret=%d",__LINE__, chip_reg.reg_addr, ret);
            return ret;
        }
    }
    #if CHECK_ERROR_CODE
    if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
    {
        printf("line %d: read error code err, ret=%d",__LINE__, ret);
        return ret;
    }
    if(mfi_chip_error_code != 0x00)
    {
        printf("req authentication certificate operate error.code=0x%x\n",mfi_chip_error_code);
        return MFI_OPERATE_ERROR;
    }
    #endif
    *length = certificate_data_length;
    printf("Read certificate data succeeded. length=%d\n",certificate_data_length);
    
    return ret;
}

/*
argument:
u8 *challenge_data: maximum 128 bytes. write in MFI coprocessor. (from apply device)
int challenge_data_len,
u8 *challenge_response_data: max 128 bytes. read from MFI coprocessor.
int challenge_response_data_len

return:
int: coprocessor read/write status
*/
int req_authentication_challenge_response(unsigned char *challenge_data, 
                                                    int challenge_data_len,
                                                    unsigned char *challenge_response_data,
                                                    int* challenge_response_data_len)
{
    unsigned char read_status;
    unsigned int read_response_length;
    int i;
    int ret;
    struct chip_reg_t chip_reg;
    unsigned char buf[2] = {0};
    unsigned char mfi_chip_error_code = 0;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI line %d: Error on opening the device file\n", __LINE__);
		return 0;
	}

printf("start AA02\n");
    #ifdef CHECK_ERROR_CODE
    read_mfi_chip_err_code(&mfi_chip_error_code);/* clear error code */
    mfi_chip_error_code = 0;
    #endif
    
    if(challenge_data_len > 128)
    {
        printf("argument error. line%d,challenge_data_len=%d\n",__LINE__,challenge_data_len);
        return MFI_OPERATE_ERROR;
    }
    *challenge_response_data_len = -1;
    
    /* Write challenge data length<Challenge data length> */
    #if 0
    printf("write challenge data length = %d\n",challenge_data_len);
    chip_reg.reg_addr = CHALLENGE_DATA_LENGTH;
    chip_reg.rd_wr_len = 2;
    chip_reg.rd_wr_flag = 0;
    buf[1] = (unsigned char)challenge_data_len;
	buf[0] = (unsigned char)(challenge_data_len>>8);
    chip_reg.buff = buf;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }
    #else
    #if 0
    printf("write challenge data length = %d\n",challenge_data_len);
    chip_reg.reg_addr = CHALLENGE_DATA_LENGTH;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = 0;
    //buf[1] = (unsigned char)challenge_data_len;
	buf[0] = (unsigned char)(challenge_data_len>>8);
    chip_reg.buff = buf;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }
    #endif
    chip_reg.reg_addr = CHALLENGE_DATA_LENGTH;
    chip_reg.rd_wr_len = 2;
    chip_reg.rd_wr_flag = 0;
    buf[1] = (unsigned char)challenge_data_len;
	buf[0] = (unsigned char)(challenge_data_len>>8);
    chip_reg.buff = buf;
    ret = write_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }
    #endif
    
    #ifdef CHECK_ERROR_CODE
    if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
    {
        printf("line %d: read error code err, ret=%d",__LINE__, ret);
        return ret;
    }
    if(mfi_chip_error_code != 0x00)
    {
        printf("line%d:write challenge data length error. length= %d.error code=0x%x\n",__LINE__,challenge_data_len, mfi_chip_error_code);
        return MFI_OPERATE_ERROR;
    }
    #endif
printf("write challenge data.\n");
    /* Write challenge data <Challenge data> */
    chip_reg.reg_addr = CHALLENGE_DATA;
    chip_reg.rd_wr_len = challenge_data_len;
    chip_reg.rd_wr_flag = 0;
    chip_reg.buff = challenge_data;
    ret = write_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }
    #ifdef CHECK_ERROR_CODE
    if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
    {
        printf("line %d: read error code err, ret=%d",__LINE__, ret);
        return ret;
    }
    if(mfi_chip_error_code != 0x00)
    {
        printf("line%d:write challenge data. error code=0x%x\n",__LINE__, mfi_chip_error_code);
        return MFI_OPERATE_ERROR;
    }
    #endif

printf("write control.\n");
     /* Write authentication control : PROC_CONTROL=1 */
     chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
     chip_reg.rd_wr_len = 1;
     chip_reg.rd_wr_flag = 0;
     buf[0] = 0x01;    /* Start new challenge, response-generation process */
     chip_reg.buff = buf;
     ret = write_mfi_chip(fd, &chip_reg);
     if(ret < 0)
     {
         printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
         return ret;
     }
#ifdef CHECK_ERROR_CODE
#if 0
     for(i=0; i<100; i++)
     {
         usleep(5000);
     if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
     {
         printf("line %d: read error code err, ret=%d\n",__LINE__, ret);
         //return ret;
     }
     if(mfi_chip_error_code != 0x00)
     {
         printf("line%d: write control error. code=0x%x\n",__LINE__, mfi_chip_error_code);
         printf("i=%d\n",i);
         return MFI_OPERATE_ERROR;
     }
     }
     printf("i=%d\n",i);
#endif     
#endif
    usleep(500000);
     
printf("read status.\n");     
    /* Read Status */
    chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = &read_status;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
        return ret;
    }

#ifdef CHECK_ERROR_CODE
         if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
         {
             printf("line %d: read error code err, ret=%d",__LINE__, ret);
             return ret;
         }
         if(mfi_chip_error_code != 0x00)
         {
             printf("line%d: read status error. code=0x%x\n",__LINE__, mfi_chip_error_code);
             return MFI_OPERATE_ERROR;
         }
#endif
    
    read_status = (read_status >> 4) & 0x07;
    if(read_status == 0x01)/* Accessory challenge response successfully generated. */
    {
printf("read challenge response length.\n");    
        /* Read challenge response length <Challenge response data length> */
        chip_reg.reg_addr = CHALLENGE_RESPONSE_DATA_LENGTH;
        chip_reg.rd_wr_len = 2;
        chip_reg.rd_wr_flag = I2C_M_RD;
        chip_reg.buff = buf;
        chip_reg.buff[0] = 0;
        chip_reg.buff[1] = 0;
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
            return ret;
        }
        read_response_length = buf[1];
        if(read_response_length > 128)
        {
            printf("line%d,read response length error =%d\n",__LINE__,read_response_length);
            return MFI_OPERATE_ERROR;
        }
        
        printf("read challenge response data.\n");    
        /* Read challenge response data <Challenge response data> */
        chip_reg.reg_addr = CHALLENGE_RESPONSE_DATA;
        chip_reg.rd_wr_len = read_response_length;
        chip_reg.rd_wr_flag = I2C_M_RD;
        chip_reg.buff = challenge_response_data;
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: write reg addr err. reg=0x%x, ret=%d",__LINE__,chip_reg.reg_addr, ret);
            return ret;
        }
        #ifdef CHECK_ERROR_CODE
        if(read_mfi_chip_err_code(&mfi_chip_error_code) < 0)
        {
            printf("line %d: read error code err, ret=%d",__LINE__, ret);
            return ret;
        }
        if(mfi_chip_error_code != 0x00)
        {
            printf("line%d: read challenge response data error.length=%d.code=0x%x\n",__LINE__, read_response_length,mfi_chip_error_code);
            return MFI_OPERATE_ERROR;
        }
        #endif
        
        *challenge_response_data_len = read_response_length;
    }
    else if(read_status == 0x00)/* Most recent process did not produce valid result. */
    {
        printf("error line%d\n",__LINE__);
    }
    else if(read_status == 0x02)/* Challenge successfully generated */
    {
        printf("error line%d\n",__LINE__);
    }
    else if(read_status == 0x03)/* Apple device challenge response successfully verified */
    {
        printf("error line%d\n",__LINE__);
    }
    else if(read_status == 0x04)/* Apple device certificate successfully validated */
    {
        printf("error line%d\n",__LINE__);
    }
    else/* reserved. */
    {
        printf("error line%d\n",__LINE__);
    }
    
    return ret;
}

/*
unsigned char* certificate_data: maximum 128*8 bytes. from apple devices, write in coprocessor;
int certificate_data_length:
unsigned char* challenge_data: max 128 bytes. read from coprocessor , return to apple devices 
int challenge_data_length:
*/
/* R18 p363 */
int device_authentication_certificate(unsigned char* certificate_data,
                                                    int certificate_data_length,
                                                    unsigned char* challenge_data,
                                                    int* challenge_data_length)
{
    unsigned char* index;
    unsigned char i;
    unsigned char certificata_data_part_amount;
    unsigned char read_status;
    unsigned int read_challenge_length;
    struct chip_reg_t chip_reg;
    unsigned char buf[2] = {0};
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return -1;
	}
    if(certificate_data_length > 1024)
    {
        printf("argument error. line%d,certificate_data_len=%d\n",__LINE__,certificate_data_length);
        return MFI_OPERATE_ERROR;
    }

    /* Write apple devices certificate data length <devices certificate data length (2 bytes)> */
    chip_reg.reg_addr = APPLE_DEVICE_CERTIFICATE_DATA_LEN;
    chip_reg.rd_wr_len = 2;
    chip_reg.rd_wr_flag = 0;
    buf[1] = (unsigned char)certificate_data_length;
	buf[0] = (unsigned char)(certificate_data_length>>8);
    chip_reg.buff = buf;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write apple devices certificate data length error. ret=%d",__LINE__, ret);
        return ret;
    }
    
    /* Write apple devices certificate data <apple devices certificate data (128*8 bytes)> */
    index = certificate_data;
    certificata_data_part_amount = certificate_data_length/128;
    if(certificate_data_length%128 != 0)
    {
        certificata_data_part_amount++;
    }
    if(certificata_data_part_amount > 8)
    {
        certificata_data_part_amount = 8;
    }
    for(i=0; i<certificata_data_part_amount; i++)
    {
        chip_reg.reg_addr = i+APPLE_DEVICE_CERTIFICATE_DATA_PART1;
        if(i == certificata_data_part_amount-1)
        {
            chip_reg.rd_wr_len = certificate_data_length%128;
            if(chip_reg.rd_wr_len == 0)
            {
                chip_reg.rd_wr_len = 128;
            }
        }
        else
        {
       	    chip_reg.rd_wr_len = 128;
       	}
        chip_reg.buff = index + i*128;
        chip_reg.rd_wr_flag = 0;    /* write */
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: write apple devices certificate data err. reg=0x%x, ret=%d",__LINE__, chip_reg.reg_addr, ret);
            return ret;
        }
    }
    
    /* Write authentication control : PROC_CONTROL=0x04 */
    chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = 0;
    read_status = 0x04;/* Start new certificate-validation process */
    chip_reg.buff = &read_status;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write authentication control errr.ret=%d",__LINE__,ret);
        return ret;
    }

    /* Read Status */
    chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = &read_status;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read authentication status err. ret=%d",__LINE__, ret);
        return ret;
    }

    read_status = (read_status >> 4) & 0x07;
    if(read_status == 0x01)/* Accessory challenge response successfully generated. */
    {
 
    }
    else if(read_status == 0x00)/* Most recent process did not produce valid result. */
    {
    }
    else if(read_status == 0x02)/* Challenge successfully generated */
    {        
        /* Read challenge length <Challenge response data length> */
         chip_reg.reg_addr = CHALLENGE_RESPONSE_DATA_LENGTH;
        chip_reg.rd_wr_len = 2;
        chip_reg.rd_wr_flag = I2C_M_RD;
        chip_reg.buff = buf;
        chip_reg.buff[0] = 0;
        chip_reg.buff[1] = 0;
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: read challenge response data length err. ret=%d",__LINE__, ret);
            return ret;
        }
        read_challenge_length = buf[0];
        if(read_challenge_length > 128)
        {
            printf("line%d,read challenge length erro =%d\n",__LINE__,read_challenge_length);
            return MFI_OPERATE_ERROR;
        }
        
        /* Read challenge data <Challenge response data> */
        chip_reg.reg_addr = CHALLENGE_RESPONSE_DATA;
        chip_reg.rd_wr_len = read_challenge_length;
        chip_reg.rd_wr_flag = I2C_M_RD;
        chip_reg.buff = challenge_data;
        ret = rd_wr_mfi_chip(fd, &chip_reg);
        if(ret < 0)
        {
            printf("line %d: read challenge response err. ret=%d",__LINE__, ret);
            return ret;
        }
        *challenge_data_length = read_challenge_length;
    }
    else if(read_status == 0x03)/* Apple device challenge response successfully verified */
    {
    }
    else if(read_status == 0x04)/* Apple device certificate successfully validated */
    {
    }
    else/* reserved. */
    {
    }
    
    return ret;
}


/*
argment:
unsigned char* challenge_data: challenge data, from apple devices;
int challenge_data_length:
unsigned char response_status: 
    device authentication succeeded return 0x03(APPLE_DEVICE_CHALLENGE_RESPONSE_SUCCESSFULLY);

return:
*/
/* 
R18 p363
R18 p61
R18 p66
*/
int device_authentication_response(unsigned char* challenge_data,
                                                int challenge_data_length,
                                                unsigned char* response_status)
{
    unsigned char read_status;
    struct chip_reg_t chip_reg;
    unsigned char buf[2] = {0};
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    if(challenge_data_length > 128)
    {
        printf("argument error. line%d,challenge_data_length=%d\n",__LINE__,challenge_data_length);
        return MFI_OPERATE_ERROR;
    }

    *response_status = 0x00;
    /* Write challenge data length <certificate data length (2bytes)> */
    chip_reg.reg_addr = CHALLENGE_DATA_LENGTH;
    chip_reg.rd_wr_len = 2;
    chip_reg.rd_wr_flag = 0;
	buf[0] = (unsigned char)(challenge_data_length>>8);
	buf[1] = (unsigned char)(challenge_data_length);
    chip_reg.buff = buf;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write challenge data length error. ret=%d",__LINE__, ret);
        return ret;
    }

    /* write challenge data <challenge data (128 bytes)> */
    chip_reg.reg_addr = CHALLENGE_DATA;
    chip_reg.rd_wr_len = challenge_data_length;
    chip_reg.rd_wr_flag = 0;
    chip_reg.buff = challenge_data;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write challenge data error. ret=%d",__LINE__, ret);
        return ret;
    }
    
    /* Write authentication control : PROC_CONTROL=0x03 */
    chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = 0;
    read_status = 0x03;/* Start new challenge response-verification process */
    chip_reg.buff = &read_status;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: write authentication control errr.ret=%d",__LINE__,ret);
        return ret;
    }
    
    /* Read Status */
    chip_reg.reg_addr = AUTHENTICATION_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = &read_status;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read authentication status err. ret=%d",__LINE__, ret);
        return ret;
    }
    *response_status= (read_status >> 4) & 0x07;
   
    return ret;
}


int read_mfi_chip_version(unsigned char* mfi_chip_version)
{
    unsigned char chip_version;
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read chip version */
    chip_reg.reg_addr = MFI_CHIP_DEVICE_VERSION;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = &chip_version;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip version err. ret=%d",__LINE__, ret);
        return ret;
    }
    *mfi_chip_version = chip_version;
    
    return ret;
}

int read_mfi_chip_firmware_version(unsigned char* mfi_chip_firmware_version)
{
    unsigned char firmware_version;
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read firmware version */
    chip_reg.reg_addr = MFI_CHIP_FIRMWARE_VERSION;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = &firmware_version;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip firmware error. ret=%d",__LINE__, ret);
        return ret;
    }
    *mfi_chip_firmware_version = firmware_version;
    
    return ret;
}

int read_mfi_chip_authentication_protocol_version(unsigned int* mfi_chip_protocol_version)
{
    unsigned char protocol_version[2] = {0};
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read authentication protocol major version */
    chip_reg.reg_addr = MFI_CHIP_PROTOCOL_MAJOR_VERSION;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = protocol_version;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip authentication protocol major version. ret=%d",__LINE__, ret);
        return ret;
    }
    
    /* Read authentication protocol minor version */
    chip_reg.reg_addr = MFI_CHIP_PROTOCOL_MINOR_VERSION;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = protocol_version+1;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip authentication protocol minor version. ret=%d",__LINE__, ret);
        return ret;
    }
    *mfi_chip_protocol_version = (protocol_version[0] << 8) + protocol_version[1];
    
    return ret;
}


int read_mfi_chip_device_id(unsigned int* mfi_chip_device_id)
{
    unsigned char device_id[4];
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read device id */
    chip_reg.reg_addr = MFI_CHIP_DEVICE_ID;
    chip_reg.rd_wr_len = 4;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = device_id;
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip device id. ret=%d",__LINE__, ret);
        return ret;
    }
    *mfi_chip_device_id = (device_id[0]<<24)\
                            + (device_id[1]<<16)\
                            + (device_id[2]<<8)\
                            + device_id[3];
    
    return ret;
}


int read_mfi_chip_err_code(unsigned char* mfi_chip_err_code)
{
    unsigned char err_code;
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read error code */
    chip_reg.reg_addr = MFI_CHIP_ERROR_CODE;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = (unsigned char*)(&err_code);
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip error code. ret=%d\n",__LINE__, ret);
        return ret;
    }
    *mfi_chip_err_code = err_code;
    
    return ret;
}

int read_mfi_chip_authentication_status(unsigned char authentication_status)
{
    int ret = 0;

    return ret;
}

int write_mfi_chip_authentication_control(unsigned char authentication_control)
{
    int ret = 0;

    return ret;
}

int read_mfi_chip_challenge_resonse_data_length(unsigned int length)
{
    int ret = 0;

    return ret;
}

int write_mfi_chip_challenge_resonse_data_length(unsigned int length)
{
    int ret = 0;

    return ret;
}

int read_mfi_chip_challenge_resonse_data(unsigned int length,
                                                        unsigned char* response_data)
{
    int ret = 0;

    return ret;
}

int write_mfi_chip_challenge_resonse_data(unsigned int length,
                                                        unsigned char* response_data)
{
    int ret = 0;

    return ret;
}

int read_mfi_chip_challenge_data_length(unsigned int length)
{
    int ret = 0;
    
    return ret;
}

int write_mfi_chip_challenge_data_length(unsigned int length)
{
    int ret = 0;
    
    return ret;
}

int read_mfi_chip_challenge_data(unsigned int length,
                                            unsigned char* challenge_data)
{
    int ret = 0;
    
    return ret;
}

int write_mfi_chip_challenge_data(unsigned int length,
                                            unsigned char* challenge_data)
{
    int ret = 0;
    
    return ret;
}

int read_mfi_chip_accessory_certificate_data_length(unsigned int length)
{
    int ret = 0;
    
    return ret;
}


int read_mfi_chip_accessory_certificate_data(unsigned int length,
                                                        unsigned char* certificate_data)
{
    int ret = 0;
    
    return ret;
}

int read_mfi_chip_self_test_status(unsigned char* mfi_chip_self_test_status)
{
    unsigned char self_test_status;
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
    
    /* Read self test status */
    chip_reg.reg_addr = SELF_TEST_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = I2C_M_RD;
    chip_reg.buff = (unsigned char*)(&self_test_status);
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip self test status. ret=%d",__LINE__, ret);
        return ret;
    }
    *mfi_chip_self_test_status = self_test_status;
    
    return ret;

}

int write_mfi_chip_self_test_control(unsigned char mfi_chip_self_test_control)
{
    unsigned char self_test_control;
    struct chip_reg_t chip_reg;
    int ret;
    unsigned int fd;

    fd = open("/dev/i2c-1", O_RDWR);
    if (!fd){
		printf("MFI %d: Error on opening the device file\n", __LINE__);
		return 0;
	}
	
    /* write self test control */
    chip_reg.reg_addr = SELF_TEST_CONTROL_AND_STATUS;
    chip_reg.rd_wr_len = 1;
    chip_reg.rd_wr_flag = 0;
    self_test_control = mfi_chip_self_test_control;
    chip_reg.buff = (unsigned char*)(&self_test_control);
    ret = rd_wr_mfi_chip(fd, &chip_reg);
    if(ret < 0)
    {
        printf("line %d: read mfi chip write self test control. ret=%d",__LINE__, ret);
        return ret;
    }
    
    return ret;
}

int read_mfi_chip_system_event_count(unsigned char* event_count)
{
    int ret = 0;
    return ret;
}

int read_mfi_chip_apple_device_certificate_data_length(unsigned int length)
{
    int ret = 0;
    return ret;
}

int read_mfi_chip_apple_device_certificate_data(unsigned int length,
                                                                unsigned char* certificate_data)
{
    int ret = 0;
    return ret;
}

int write_mfi_chip_apple_device_certificate_data(unsigned int length,
                                                                unsigned char* certificate_data)
{
    int ret = 0;
    return ret;
}

/* END */

