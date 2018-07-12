/*
* Copyright (c) 2016 AutoChips Inc.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/cpufreq.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <mach/dma.h>
#include <linux/delay.h>
#include <asm/delay.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>

#include <linux/slab.h>
#include <linux/sched.h>
/*#include <linux/smp_lock.h>*/
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/backing-dev.h>
#include <linux/compat.h>
#include <linux/mount.h>

#include <asm/uaccess.h>

#include "x_typedef.h"
#include "drv_dual.h"
#include "backcar_msg.h"
#include "drv_av_d.h"
#include "pmx_hal.h"
#include "wch_if.h"
//#include "ac83xx_memory.h"
#include "ac83xx_gpio_pinmux.h"
#include "ac83xx_pinmux_table.h"
#include "pinmux_reg.h"
#include <linux/gpio.h>


#include <linux/irq.h> 
#include <asm/uaccess.h> 
#include <mach/pinmux.h> 
#include <mach/ac83xx_pinmux_table.h> 
#include <mach/ac83xx.h> 
#include <mach/ac83xx_basic.h>

#include <linux/types.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/sched.h>

#include "x_typedef.h"
#include "windev.h"
#include "oal.h"
#include "x_ver.h"

#include <linux/of.h>
#include <linux/of_address.h>
#include <memory.h>
#include "mach/83xx_irqs_vector.h"

//aux_detect port define
#define AUX_DETCET_PORT PIN_140_TS_SYNC


#define  FASTCAMERAUI_ENABLE      1
#define MTK_KERNEL_LINUX_LICENSE     "Proprietary"
#define BC_LOG_TAG "[BC_DRIVER]"

/**
Revision Control
*/
#define DBC_MOD_NAME    "DualArmBackCar"
#define DBC_VER_MAIN    1
#define DBC_VER_MINOR   0
#define DBC_VER_REV     0


MODULE_LICENSE("GPL");


/*extern void  HideBackVideo(void);*/

/************  [ IOCTL Code ]  ***************/


#define IOCTL_AUX_DETECT_GPIOINIT _IOR('M', 0x6, unsigned)
#define IOCTL_AUX_DETECTGPIO _IOR('M', 0x7, unsigned)


static struct fasync_struct *fasync_queue=NULL;
struct timer_list auxdetect_timer;
unsigned char time_flag=1;
wait_queue_head_t auxdetect_queue;
unsigned char auxdetect_flag = 0;

struct gpio_desc *pGPIO53;

//延时50ms再处理中断，防止中断误触
void auxdetect_timer_func(unsigned long data){	 

	time_flag = 1;
	auxdetect_flag=1;
	wake_up(&auxdetect_queue);

  unsigned long u8Val = gpio_get_value(AUX_DETCET_PORT);

  //printk("\r\n\r\n%s:%d\r\n\r\n",__FUNCTION__,u8Val);
	/*
	if (&fasync_queue) {
		//kill_fasync(&fasync_queue, SIGIO, POLL_IN);
		printk("[BACKCAR]kernel send interrupt to app\n");
	}
	*/
}

static irqreturn_t auxdetect_io_irq_handler(int irq,void *id){

	//启动定时器,定时100ms，jiffies为系统滴答时间，HZ为1秒
	//只启动一次，每次中断需要重新打开
	if(time_flag){
		mod_timer(&auxdetect_timer, jiffies + HZ);
		time_flag = 0;
	}
       
        // unsigned long u8Val = gpio_get_value(AUX_DETCET_PORT);

        // printk(KERN_ERR"\r\n\r\n%s:%d\r\n\r\n",__FUNCTION__,u8Val);	
	//ac83xx_mask_ack_bim_irq(irq);
	//printk(KERN_ERR"%s success\n", __FUNCTION__);
	return IRQ_HANDLED;
}


void GPIO_Init(void)
{ 
  int irq_flags = 0;

  pGPIO53 =  gpio_to_desc(AUX_DETCET_PORT);
  if(pGPIO53 == NULL)
  {
    printk("%s gpio_to_desc fail\r\n", __FUNCTION__);
    return;
  }

  gpiod_direction_input(pGPIO53);
  int irq = gpio_to_irq(AUX_DETCET_PORT); 

  if(irq<0)
  {
    printk("%s gpio_to_irq fail\r\n", __FUNCTION__);
    return;
  }
  
  irq_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING ;
  
  //GPIO_IRQTYPE_HIGHLEVEL
  //request_irq(irq,auxdetect_io_irq_handler,GPIO_IRQTYPE_HIGHLEVEL,"gpio_irq_aux_detect",NULL);
	request_irq(irq,auxdetect_io_irq_handler,irq_flags,"gpio_irq_aux_detect",NULL);
  
  printk("AUX_DETCET  %s success\n", __FUNCTION__);
}

unsigned long GPIO_get_aux_detect_value(void)
{
	unsigned long u8Val = gpio_get_value(AUX_DETCET_PORT);
	return u8Val;
}

EXPORT_SYMBOL(GPIO_get_aux_detect_value);

static long auxdetect_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  void __user *argp = (void __user *)arg; 

  if(cmd == IOCTL_AUX_DETECTGPIO)
  {
    unsigned long u4Val =  gpio_get_value(AUX_DETCET_PORT);
    copy_to_user(argp, &u4Val, sizeof(unsigned long)); 
    return u4Val;
  }
  return -1;
}

static int auxdetect_open(struct inode *inode, struct file *file)
{
	MOD_VERSION_INFO(DBC_MOD_NAME, DBC_VER_MAIN, DBC_VER_MINOR, DBC_VER_REV);
	pr_info("auxdetect_open\n");

  //GPIO_Init();

	return 0;
}

ssize_t auxdetect_read(struct file *file, char __user *buf, size_t size, loff_t *ppos){
	
	unsigned long u8Val;	

	printk("wait auxdetect state change\n");
	
	wait_event(auxdetect_queue, auxdetect_flag); 

	u8Val = gpio_get_value(AUX_DETCET_PORT);
	
	if(copy_to_user( buf, (char*)&u8Val, sizeof(unsigned long)) != 0)
		return -EFAULT;

	auxdetect_flag = 0;
	
	return sizeof(unsigned long);
}

static int auxdetect_fun(int fd, struct file *filp, int on){
	int ret = -EINVAL;
	
	ret  = fasync_helper(fd, filp, on, &fasync_queue);
	if (ret) {
		printk("register fasync_helper failed\n");
		return ret;
	}

	printk("%s success\n", __FUNCTION__);
	return 0;
}

static int auxdetect_release(struct inode *inode, struct file *file)
{
	pr_info("auxdetect_release\n");
  
  gpiod_put(pGPIO53);

	return 0;
}

static const struct file_operations auxdetectdrv_fops = {
	.owner            = THIS_MODULE,
	.open = auxdetect_open,
	.read = auxdetect_read,
	.fasync = auxdetect_fun,
	.release = auxdetect_release,
	.unlocked_ioctl   = auxdetect_ioctl,
};

static struct miscdevice auxdetect_dev = {
	MISC_DYNAMIC_MINOR,
	"auxdetectdrv",
	&auxdetectdrv_fops
};

static int __init auxdetect_drv_init(void)
{
	int ret = 0;
        printk(KERN_ERR"auxdetect_drv_init \n");
	ret = misc_register(&auxdetect_dev);
        printk(KERN_ERR"auxdetect_drv misc_register ret =%d \n",ret);
	if (ret) {
		pr_info("Unable to register \"auxdetect\" misc device\n");
		return ret;

	}


	init_timer(&auxdetect_timer);    
	auxdetect_timer.function = auxdetect_timer_func;

	init_waitqueue_head(&auxdetect_queue);

  GPIO_Init();
	
	return ret;
}

static void __exit auxdetect_drv_exit(void)
{
	misc_deregister(&auxdetect_dev);
}


module_init(auxdetect_drv_init);
module_exit(auxdetect_drv_exit);







