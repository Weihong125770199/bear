
#ifndef __ACCESSORY_AUTHENTICATION_H__
#define __ACCESSORY_AUTHENTICATION_H__

/* coprocessor register */
#define MFI_CHIP_DEVICE_VERSION             0X00
#define MFI_CHIP_FIRMWARE_VERSION           0X01
#define MFI_CHIP_PROTOCOL_MAJOR_VERSION     0X02
#define MFI_CHIP_PROTOCOL_MINOR_VERSION     0X03
#define MFI_CHIP_DEVICE_ID                  0X04
#define MFI_CHIP_ERROR_CODE                 0X05
#define AUTHENTICATION_CONTROL_AND_STATUS   0X10
#define CHALLENGE_RESPONSE_DATA_LENGTH      0X11
#define CHALLENGE_RESPONSE_DATA             0X12
#define CHALLENGE_DATA_LENGTH               0X20
#define CHALLENGE_DATA                      0X21
#define ACCESSORY_CERTIFICATE_DATA_LENGTH   0X30
#define ACCESSORY_CERTIFICATE_DATA_PART1    0X31
#define ACCESSORY_CERTIFICATE_DATA_PART2    0X32
#define ACCESSORY_CERTIFICATE_DATA_PART3    0X33
#define ACCESSORY_CERTIFICATE_DATA_PART4    0X34
#define ACCESSORY_CERTIFICATE_DATA_PART5    0X35
#define ACCESSORY_CERTIFICATE_DATA_PART6    0X36
#define ACCESSORY_CERTIFICATE_DATA_PART7    0X37
#define ACCESSORY_CERTIFICATE_DATA_PART8    0X38
#define ACCESSORY_CERTIFICATE_DATA_PART9    0X39
#define ACCESSORY_CERTIFICATE_DATA_PART10   0X3A
#define SELF_TEST_CONTROL_AND_STATUS        0X40
#define SYSTEM_EVENT_COUNTER                0X4D
#define ACCESSORY_CERTIFICATE_SERIAL_NUMBER 0X4E
#define APPLE_DEVICE_CERTIFICATE_DATA_LEN   0X50
#define APPLE_DEVICE_CERTIFICATE_DATA_PART1 0X51
#define APPLE_DEVICE_CERTIFICATE_DATA_PART2 0X52
#define APPLE_DEVICE_CERTIFICATE_DATA_PART3 0X53
#define APPLE_DEVICE_CERTIFICATE_DATA_PART4 0X54
#define APPLE_DEVICE_CERTIFICATE_DATA_PART5 0X55
#define APPLE_DEVICE_CERTIFICATE_DATA_PART6 0X56
#define APPLE_DEVICE_CERTIFICATE_DATA_PART7 0X57
#define APPLE_DEVICE_CERTIFICATE_DATA_PART8 0X58

/* coprocessor authentication status */
#define INVALID_RESULT                                  0X00
#define ACCESSORY_CHALLENGE_RESPONSE_SUCCESSFULLY       0X01
#define CHALLENGE_SUCCESSFULLY                          0X02
#define APPLE_DEVICE_CHALLENGE_RESPONSE_SUCCESSFULLY    0X03
#define APPLE_DEVICE_CERTIFICATE_SUCCESSFULLY           0X04
/* coprocessor authentication control */
#define NO_OPERATION                                0X00
#define START_NEW_CHALLENGE_RESPONSE_GENERATION     0X01
#define START_NEW_CHALLENGE_GENERATION              0X02
#define START_NEW_CHALLENGE_RESPONSE_VERIFICATION   0X03
#define START_NEW_CERTIFICATE_VALIDATION            0X04

/* AA00 */
extern int req_authentication_certificate(unsigned char* certificate_data, 
                                                int* length);
/* AA02 */
extern int req_authentication_challenge_response(unsigned char *challenge_data, 
                                                        int challenge_data_len,
                                                        unsigned char *challenge_response_data,
                                                        int* challenge_response_data_len);
/* AA11 */
extern int device_authentication_certificate(unsigned char* certificate_data,
                                                    int certificate_data_length,
                                                    unsigned char* challenge_data,
                                                    int* challenge_data_length);
/* AA13 */
extern int device_authentication_response(unsigned char* challenge_data,
                                                int challenge_data_length,
                                                unsigned char* response_status);


extern int read_mfi_chip_version(unsigned char* mfi_chip_version);
extern int read_mfi_chip_firmware_version(unsigned char* mfi_chip_firmware_version);
extern int read_mfi_chip_authentication_protocol_version(unsigned int* mfi_chip_protocol_version);
extern int read_mfi_chip_device_id(unsigned int* mfi_chip_device_id);
extern int read_mfi_chip_err_code(unsigned char* mfi_chip_err_code);
extern int read_mfi_chip_self_test_status(unsigned char* self_test_status);
extern int write_mfi_chip_self_test_control(unsigned char self_test_control);
extern int read_mfi_chip_system_event_count(unsigned char* event_count);
#endif

