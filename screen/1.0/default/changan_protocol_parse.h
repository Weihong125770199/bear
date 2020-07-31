//This file is used for parse MCU LCD data
#ifndef _CHANGAN_PROTOCOL_PARSE_H
#define _CHANGAN_PROTOCOL_PARSE_H
#define I2CDATA_MAX_LEN_TOUCH_TYPE_RES_LEN  11
#define I2CDATA_MAX_LEN_TOUCH_TYPE_CAP_LEN  30
typedef unsigned char u8;
typedef unsigned short u16;
struct mcu_cmd_packet
{
 u8 cmd_id;
 u8 data[129];
// unsigned int data_len;
};

//cmdid MFD to HU
//#define	MFD_SELF_CHECK_INFO	0x40
//#define TP_POSITION_INFO	0x41
#define TP_POSITION_INFO_F527	0x81
//#define EX_TP_INFO			0x42
#define EX_TP_INFO_F527			0x82
#define	TP_PROXIMITY_INFO	0x43
#define	RESISTANCE_TP_INFO	0x45
#define	MFD_MODE			0x46
#define	FUNC_DEFECT			0x48
#define	VOLTAGE_RANGE		0x49
#define	PRODUCTIOND_ID		0x4A
#define	MFD_VERSION			0x4B
#define	RESERVED			0x4C
#define	IAP_STATUS			0x70	//
#define	REQUEST_UPDATE_PKG	0x44	//
#define	UPDATE_RESULT	    0x46	//
#define UPDATE_ACK          0x41
#define UPDATE_PACKAGE_TOTAL_NUM          0x42
//HU to MFD
#define	HU_START_DONE		0x80
#define	ASK_MFD_SATUS		0x81
#define	CALIBRATION			0x82
#define	RESEND				0x83
#define	MFD_SHUTDOWN		0x84
#define	MFD_SLEEP			0x85
#define DIM_PWM             0x86
#define SET_MFD_MODE		0x90
#define	IAP_COMMAND			0x40	//
#define	UPDATE_PKG_NUM		0x43	//
#define UPDATE_PKG			0x45	//

/*
struct mcu_cmd cmd_MFD_to_HU = 
{
 {0x40,4},//MFD self check info
 {0x41,20},//TP Position info
 {0x42,27},//extendedTP Position info
 {0x43,5},//TP Proximity info
 {0x45,8},//resistance touch panel info
 {0x46,4},//MFD mode
 {0x48,4},//Function defect
 {0x49,4},//Voltage range
 {0x4A,4},//Production ID
 {0x4B,4},//Version
 {0x4C,20},//reserved
 {0x70,4},//IAP status
 {0x71,2},//request update packet
 {0x72,1},//request checksum
};

struct mcu_cmd cmd_HU_to_MFD =
{
 {0x80,1},//HU startup done
 {0x81,1},//Question Mfd Status
 {0x82,1},//Calibration(CTP)
 {0x83,1},//Resend
 {0x84,1},//MFD Shutdown
 {0x85,1},//MFD Sleep
 {0x86,1},//Dim(PWM)
 {0x90,4},//MFD mode
 {0xF0,4},//IAP Command
 {0xF1,2},//Amount of update data packet
 {0xF2,128},//Update data packet
 {0xF3,4},//checksum of update data
};
*/

//------------------MFD DATA structure start-----------------
struct mfd_self_check_info
{
 u8 done; //1 for done, 0 for not
 u8 tsp_type;
 u8 reserve2;
 u8 reserve3;
 u8 reserved[4];
};

#define	TS_DETECT_BIT	(1<<7)
#define	TS_PRESS_BIT	(1<<6)
#define	TS_RELEASE_BIT	(1<<5)
#define	TS_MOVE_BIT		(1<<4)
#define	TS_VECTOR_BIT	(1<<3)
#define	TS_AMP_BIT		(1<<2)
#define	TS_SUPPRESS_BIT	(1<<1)
#define	TS_UNGRIP_BIT	(1<<0)

struct tp_pos_info
{
 u8 touch_num;	//Number of touches in the case
 u8 touch_id1;	//Touch ID: from 1 to 5
 u8 status1;	//bit7:detect; bit6:press; bit5:release; bit4:move;bit3:vector;bit2:amp;bit1:suppress; bit0:ungrip
 u8 x1[2];		//X and Y position correspond with Pixel position of panel
 u8 y1[2];
 u8 size1;		//Reports the size of the touch area.
 u8 amplitude1;	//This can be use to detect the size or pressure of touch.
 u8 vector1;	//This gives an indication of the dirction of the touch.
 u8 touch_id2;
 u8 status2;
 u8 x2[2];
 u8 y2[2];
 u8 size2;
 u8 amplitude2;
 u8 vector2;
 u8 reserve;
};

struct ext_tp_pos_info
{
 u8 touch_id3;
 u8 status3;
 u8 x3[2];
 u8 y3[2];
 u8 size3;
 u8 amplitude3;
 u8 vector3;
 u8 touch_id4;
 u8 status4;
 u8 x4[2];
 u8 y4[2];
 u8 size4;
 u8 amplitude4;
 u8 vector4;
 u8 touch_id5;
 u8 status5;
 u8 x5[2];
 u8 y5[2];
 u8 size5;
 u8 amplitude5;
 u8 vector5;
};

struct tp_proximity_info
{
 u8 reserved[8];
};

struct resis_panle_info
{
 u16 y;//mcu tanslate y first, then x. x is vertical, y is horizontal which is different from LCD coordinate
 u16 x;
 u16 z1;
 u16 z2;
};

struct mfd_mode
{
 u8 mode;
 u8 blight_lev;
 u8 rev1;
 u8 rev2;
 u8 reserved[4];
};

struct fun_defect 
{
 u8 mfd_function;
 u8 display_fun;
 u8 touch_fun;
 u8 mfd_temperature;
 //u8 lvds_status;//0 lvds siginal locked, 1 lvds siginal not locked
 u8 reserved[4];
};

struct vol_range
{
 u8 vol_status;
 u8 reserved[7];
};

struct productID
{
 u8 rev1;
 u8 rev2;
 u8 rev3;
 u8 rev4;
 u8 reserved[4];
};

struct productVer
{
 u8 HwMain;
 u8 HwSub;
 u8 SwMain;
 u8 SwSub;
 u8 IapOrUser;
 u8 year;
 u8 month;
 u8 day;
};

#define RECEIVED_IAP_CMD	(1)
#define ENTER_IAP_MODE		(1<<1)
#define DURING_IAP 			(1<<2)
#define IAP_FINISHED		(1<<3)
#define IAP_FAILED			(1<<4)
#define IAP_WRITE_FLASH_ERR (1<<5)
#define CANNOT_ENTER_IAP	(1<<6)
#define I2C_COMM_ERR		(1<<7)
#define PKG_CHECK_ERROR 	(1<<8)
#define CHECKSUM_ERROR		(1<<9)
#define WRONG_UPDATE_PWD	(1<<10)
#define WRONG_PROD_ID		(1<<11)
#define NOT_IN_IAP_MODE		(1<<12)
#define APP_IN_IAP_MODE		(1<<13)

struct IAP_status
{
 u8 status;
 u8 data_len;
 u8 reserved[6];
};


struct update_result
{
 u8 result;
 
 
};



struct req_update
{
 u8 pkg_num[2];
 u8 reserved[6];
};

//------------------MFD DATA structure end-----------------

//------------------HU DATA structure start-----------------
struct HUready
{
 u8 cmdid;
 u8 ready;
 u8 reserved[3];
 u8 EXT_length;
 u8 chsum;
};

struct req_MFD_status
{
 u8 cmdid;
 u8 rev;
 u8 reserved[3];
 u8 EXT_length;
 u8 chsum;
};

struct calibration
{
 u8 cmdid;
 u8 rev;
 u8 reserved[3];
 u8 EXT_length;
 u8 chsum;
};

struct resend
{
 u8 cmdid;
 u8 rev;
 u8 reserved[3];
 u8 EXT_length;
 u8 chsum;
};

struct MFD_shutdown
{
 u8 cmdid;
 u8 reserved[4];
 u8 EXT_length;
 u8 chsum;
};

struct MFD_sleep
{
 u8 cmdid;
 u8 reserved[4];
 u8 EXT_length;
 u8 chsum;
};

struct MFD_mode
{
 u8 cmdid;
 u8 lcd_bl_mode;
 u8 bl_lev; 
 u8 reserved[2];
 u8 EXT_length;
 u8 chsum;
};

struct IAP_mode
{
 u8 cmdid;
 u8 cmd;
 u8 pwd[3];
 u8 EXT_length;
 u8 chsum;
};

struct up_pk_amount
{
 u8 cmdid;
 u8 pkg_num[2];
// u8 reserved[2];
 u8 EXT_length;
 u8 chsum;
};

struct update_pk
{
 u8 cmdid;
 u8 packetIndex[2];
 u8 size;
 u8 data[128];
 u8 EXT_length;
 u8 chsum;
};

enum UpdateResult  {
   UPDATE_RESULT_SUCCESS,
   UPDATE_RESULT_PACKAGE_NUMBER_ERROR,
   UPDATE_RESULT_FLASH_ERASER_ERROR,
   UPDATE_RESULT_FLASH_WRITE_ERROR,
   UPDATE_RESULT_SWAP_ERROR,
   UPDATE_RESULT_TIMEOUT,
   UPDATE_RESULT_INIT,

};

//------------------HU DATA structure end-----------------
#endif
