// ATMEL AVR - A P P L I C A T I O N   N O T E
//
// Title:          AVR068 - STK500 Communication Protocol
// Filename:       command.h
// Version:        1.0
// Last updated:   31.01.2005
// Support E-mail: avr@atmel.com
//
// Last updated:   20.08.2011
// RikusW: Added AVRISP mkII / STK600 commands
//
//-------------------------------------------------------------------
// STK message constants

#define MESSAGE_START                       0x1B
#define TOKEN                               0x0E

//-------------------------------------------------------------------
// STK general command constants

#define CMD_SIGN_ON                         0x01
#define CMD_SET_PARAMETER                   0x02
#define CMD_GET_PARAMETER                   0x03
#define CMD_SET_DEVICE_PARAMETERS           0x04 //OLD
#define CMD_OSCCAL                          0x05
#define CMD_LOAD_ADDRESS                    0x06
#define CMD_FIRMWARE_UPGRADE                0x07

//AVRISP mkII
#define CMD_RESET_PROTECTION                0x0A

//STK600
#define CMD_CHECK_TARGET_CONNECTION         0x0D
#define CMD_LOAD_RC_ID_TABLE                0x0E
#define CMD_LOAD_EC_ID_TABLE                0x0F
#define CMD_CLEAR_RC_ID_TABLE               0x09

#define CMD_JTAG_AVR32                      0x80
#define CMD_ENTER_PROGMODE_JTAG_AVR32       0x81
#define CMD_LEAVE_PROGMODE_JTAG_AVR32       0x82

#define CMD_JTAG_AVR                        0x90

//-------------------------------------------------------------------
// STK ISP command constants

#define CMD_ENTER_PROGMODE_ISP              0x10
#define CMD_LEAVE_PROGMODE_ISP              0x11
#define CMD_CHIP_ERASE_ISP                  0x12
#define CMD_PROGRAM_FLASH_ISP               0x13
#define CMD_READ_FLASH_ISP                  0x14
#define CMD_PROGRAM_EEPROM_ISP              0x15
#define CMD_READ_EEPROM_ISP                 0x16
#define CMD_PROGRAM_FUSE_ISP                0x17
#define CMD_READ_FUSE_ISP                   0x18
#define CMD_PROGRAM_LOCK_ISP                0x19
#define CMD_READ_LOCK_ISP                   0x1A
#define CMD_READ_SIGNATURE_ISP              0x1B
#define CMD_READ_OSCCAL_ISP                 0x1C
#define CMD_SPI_MULTI                       0x1D

//-------------------------------------------------------------------
// STK HVPP command constants

#define CMD_ENTER_PROGMODE_HVPP             0x20
#define CMD_LEAVE_PROGMODE_HVPP             0x21
#define CMD_CHIP_ERASE_HVPP                 0x22
#define CMD_PROGRAM_FLASH_HVPP              0x23
#define CMD_READ_FLASH_HVPP                 0x24
#define CMD_PROGRAM_EEPROM_HVPP             0x25
#define CMD_READ_EEPROM_HVPP                0x26
#define CMD_PROGRAM_FUSE_HVPP               0x27
#define CMD_READ_FUSE_HVPP                  0x28
#define CMD_PROGRAM_LOCK_HVPP               0x29
#define CMD_READ_LOCK_HVPP                  0x2A
#define CMD_READ_SIGNATURE_HVPP             0x2B
#define CMD_READ_OSCCAL_HVPP                0x2C    

#define CMD_SET_CONTROL_STACK               0x2D

//-------------------------------------------------------------------
// STK HVSP command constants

#define CMD_ENTER_PROGMODE_HVSP             0x30
#define CMD_LEAVE_PROGMODE_HVSP             0x31
#define CMD_CHIP_ERASE_HVSP                 0x32
#define CMD_PROGRAM_FLASH_HVSP              0x33
#define CMD_READ_FLASH_HVSP                 0x34
#define CMD_PROGRAM_EEPROM_HVSP             0x35
#define CMD_READ_EEPROM_HVSP                0x36
#define CMD_PROGRAM_FUSE_HVSP               0x37
#define CMD_READ_FUSE_HVSP                  0x38
#define CMD_PROGRAM_LOCK_HVSP               0x39
#define CMD_READ_LOCK_HVSP                  0x3A
#define CMD_READ_SIGNATURE_HVSP             0x3B
#define CMD_READ_OSCCAL_HVSP                0x3C

//-------------------------------------------------------------------
// STK status constants

// Success
#define STATUS_CMD_OK                       0x00

// Warnings
#define STATUS_CMD_TOUT                     0x80
#define STATUS_RDY_BSY_TOUT                 0x81
#define STATUS_SET_PARAM_MISSING            0x82

// Errors
#define STATUS_CMD_FAILED                   0xC0
#define STATUS_CKSUM_ERROR                  0xC1
#define STATUS_CMD_UNKNOWN                  0xC9
#define ANSWER_CKSUM_ERROR                  0xB0

//AVRISP mkII
#define STATUS_ISP_READY                    0x00
#define STATUS_CONN_FAIL_MOSI               0x01
#define STATUS_CONN_FAIL_RST                0x02
#define STATUS_CONN_FAIL_SCK                0x04
#define STATUS_TGT_NOT_DETECTED             0x10
#define STATUS_TGT_REVERSE_INSERTED         0x20

//STK600
#define STATUS_CMD_ILLEGAL_PARAMETER        0xCA

// hw_status
// Bits in status variable
// Bit 0-3: Slave MCU
// Bit 4-7: Master MCU
#define STATUS_AREF_ERROR            0 // Set to '1' if AREF is short circuited
#define STATUS_VTG_ERROR             4 // Set to '1' if VTG is short circuited
#define STATUS_RC_CARD_ERROR         5 // Set to '1' if board id changes when board is powered
#define STATUS_PROGMODE              6 // Set to '1' if board is in programming mode
#define STATUS_POWER_SURGE           7 // Set to '1' if board draws excessive current

//-------------------------------------------------------------------
// STK parameter constants

#define PARAM_BUILD_NUMBER_LOW              0x80
#define PARAM_BUILD_NUMBER_HIGH             0x81
#define PARAM_HW_VER                        0x90
#define PARAM_SW_MAJOR                      0x91
#define PARAM_SW_MINOR                      0x92
#define PARAM_VTARGET                       0x94
#define PARAM_VADJUST                       0x95
#define PARAM_OSC_PSCALE                    0x96
#define PARAM_OSC_CMATCH                    0x97
#define PARAM_SCK_DURATION                  0x98
#define PARAM_TOPCARD_DETECT                0x9A
#define PARAM_STATUS                        0x9C
#define PARAM_DATA                          0x9D
#define PARAM_RESET_POLARITY                0x9E
#define PARAM_CONTROLLER_INIT               0x9F

//STK600
#define PARAM_STATUS_TGT_CONN               0xA1
#define PARAM_DISCHARGEDELAY                0xA4
#define PARAM_SOCKETCARD_ID                 0xA5
#define PARAM_ROUTINGCARD_ID                0xA6
#define PARAM_EXPCARD_ID                    0xA7
#define PARAM_SW_MAJOR_SLAVE1               0xA8
#define PARAM_SW_MINOR_SLAVE1               0xA9
#define PARAM_SW_MAJOR_SLAVE2               0xAA
#define PARAM_SW_MINOR_SLAVE2               0xAB
#define PARAM_BOARD_ID_STATUS               0xAD
#define PARAM_RESET                         0xB4
#define PARAM_JTAG_ALLOW_FULL_PAGE_STREAM   0x50
#define PARAM_JTAG_EEPROM_PAGE_SIZE         0x52
#define PARAM_JTAG_DAISY_BITS_BEFORE        0x53
#define PARAM_JTAG_DAISY_BITS_AFTER         0x54
#define PARAM_JTAG_DAISY_UNITS_BEFORE       0x55
#define PARAM_JTAG_DAISY_UNITS_AFTER        0x56

// Parameter constants for 2 byte values
#define PARAM2_SCK_DURATION                 0xC0
#define PARAM2_CLOCK_CONF                   0xC1
#define PARAM2_AREF0                        0xC2
#define PARAM2_AREF1                        0xC3
#define PARAM2_JTAG_FLASH_SIZE_H            0xC5
#define PARAM2_JTAG_FLASH_SIZE_L            0xC6
#define PARAM2_JTAG_FLASH_PAGE_SIZE         0xC7
#define PARAM2_RC_ID_TABLE_REV              0xC8
#define PARAM2_EC_ID_TABLE_REV              0xC9

//-------------------------------------------------------------------
// XPROG commands

#define CMD_XPROG                         0x50
#define CMD_XPROG_SETMODE                 0x51
#define XPRG_PROTOCOL_PDI                 0
#define XPRG_PROTOCOL_JTAG                1
#define XPRG_PROTOCOL_TPI                 2


#define XPRG_CMD_ENTER_PROGMODE           0x01
#define XPRG_CMD_LEAVE_PROGMODE           0x02
#define XPRG_CMD_ERASE                    0x03
#define XPRG_CMD_WRITE_MEM                0x04
#define XPRG_CMD_READ_MEM                 0x05
#define XPRG_CMD_CRC                      0x06
#define XPRG_CMD_SET_PARAM                0x07

// Memory types
#define XPRG_MEM_TYPE_APPL                1
#define XPRG_MEM_TYPE_BOOT                2
#define XPRG_MEM_TYPE_EEPROM              3
#define XPRG_MEM_TYPE_FUSE                4
#define XPRG_MEM_TYPE_LOCKBITS            5
#define XPRG_MEM_TYPE_USERSIG             6
#define XPRG_MEM_TYPE_FACTORY_CALIBRATION 7

// Erase types
#define XPRG_ERASE_CHIP                   1
#define XPRG_ERASE_APP                    2
#define XPRG_ERASE_BOOT                   3
#define XPRG_ERASE_EEPROM                 4
#define XPRG_ERASE_APP_PAGE               5
#define XPRG_ERASE_BOOT_PAGE              6
#define XPRG_ERASE_EEPROM_PAGE            7
#define XPRG_ERASE_USERSIG                8
#define XPRG_ERASE_FUSE                   9

// Write mode flags
#define XPRG_MEM_WRITE_ERASE              0
#define XPRG_MEM_WRITE_WRITE              1

// CRC types
#define XPRG_CRC_APP                      1
#define XPRG_CRC_BOOT                     2
#define XPRG_CRC_FLASH                    3

// Error codes
#define XPRG_ERR_OK                       0
#define XPRG_ERR_FAILED                   1
#define XPRG_ERR_COLLISION                2
#define XPRG_ERR_TIMEOUT                  3

// XPROG parameters of different sizes
// 4-byte address
//#define XPRG_PARAM_NVMBASE                1
// 2-byte page size
//#define XPRG_PARAM_EEPPAGESIZE            2

#define XPRG_PARAM_NVMBASE                  0x01
#define XPRG_PARAM_EEPPAGESIZE              0x02
#define XPRG_PARAM_NVMCMD_REG               0x03
#define XPRG_PARAM_NVMCSR_REG               0x04

#define XPRG_PAGEMODE_WRITE                 2 //(1 << 1)
#define XPRG_PAGEMODE_ERASE                 1 //(1 << 0)


//-------------------------------------------------------------------


