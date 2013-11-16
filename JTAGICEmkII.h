// vim:ts=4 sts=0 sw=4

#ifndef JTAGICEmkII_H
#define JTAGICEmkII_H

#define USB_DEVICE_JTAGICEMKII 0x2103

class Jtag2Commands;
class STK500J2Commands;

//-----------------------------------------------------------------------------
//STK600.h needs to be included after JTAGICEmkII.h
//but JTAGICEmkII.h needs ProgIF_AVRISPmkII_ISP, so what to do ?

class ProgIF_AVRISPmkII_ISP : public ProgIF_STK500_ISP
{
public:
	ProgIF_AVRISPmkII_ISP();
	~ProgIF_AVRISPmkII_ISP() {};
	int SetPClock(int u);
	int ApplyPClock();
	void Hz2P(int u);
	int P2Hz();
};

//---------------------------------------------------------

class ProgIF_JTAGICEmkII_ISP : public ProgIF_AVRISPmkII_ISP
{
public:
	ProgIF_JTAGICEmkII_ISP() {};
	~ProgIF_JTAGICEmkII_ISP() {};
	bool Connect(const char *);
	Jtag2Commands *cmdj2;
};

//---------------------------------------------------------

class ProgIF_JTAGICEmkII_MEGA_JTAG : public ProgIF
{
public:
	ProgIF_JTAGICEmkII_MEGA_JTAG();
	~ProgIF_JTAGICEmkII_MEGA_JTAG() {};
	u32 GetDevIf() { return DIF_MJ; };
	bool Connect(const char *);
	void SetDevice(Device *);

	int SetPClock(int u);
	int ApplyPClock();
	void Hz2P(int u);
	int P2Hz();

	u8 ReadSig(u8);
	u8 ReadCal(u8);
	u32 ReadJtagID();
	u32 ReadSignature();

	u8 ReadFuses(u8 f[32]);
	void WriteFuses(u8 f[32]);

	void ChipErase();
	bool ReadFlashBlock  (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock (u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

	Jtag2Commands *cmdj2;
protected:
	u8 ReadLock();
	u8 ReadFuse(u8);
	void WriteLock(u8);
	void WriteFuse(u8,u8);

	void SetDescriptor();

	u8 jclk;
	bool bDefaultP;
};

//---------------------------------------------------------
//Not implemented

class ProgIF_JTAGICEmkII_PDI : public ProgIF
{
public:
	ProgIF_JTAGICEmkII_PDI() { inm = "pdi"; };
	~ProgIF_JTAGICEmkII_PDI() {};
	u32 GetDevIf() { return DIF_PDI; };
	bool Connect(const char *) { return false; };
	Jtag2Commands *cmdj2;
};

//---------------------------------------------------------
//Not implemented

class ProgIF_JTAGICEmkII_XMEGA_JTAG : public ProgIF
{
public:
	ProgIF_JTAGICEmkII_XMEGA_JTAG() { inm = "xj"; };
	~ProgIF_JTAGICEmkII_XMEGA_JTAG() {};
	u32 GetDevIf() { return DIF_XJ; };
	bool Connect(const char *) { return false; };
	Jtag2Commands *cmdj2;
};

//---------------------------------------------------------
//Not implemented

class ProgIF_JTAGICEmkII_AVR32_JTAG : public ProgIF
{
public:
	ProgIF_JTAGICEmkII_AVR32_JTAG() { inm = "aj"; };
	~ProgIF_JTAGICEmkII_AVR32_JTAG() {};
	u32 GetDevIf() { return DIF_AJ; };
	bool Connect(const char *) { return false; };
	Jtag2Commands *cmdj2;
};

//-----------------------------------------------------------------------------

class ProgJTAGICEmkIIu : public ProgUSB
{
public:
	ProgJTAGICEmkIIu();
	~ProgJTAGICEmkIIu();

	Jtag2Commands *cmdj2;
	STK500J2Commands *cmds5j2;
	ProgIF_JTAGICEmkII_ISP isp;
	ProgIF_JTAGICEmkII_MEGA_JTAG mj;
	ProgIF_JTAGICEmkII_PDI pdi;
	ProgIF_JTAGICEmkII_XMEGA_JTAG xj;
	ProgIF_JTAGICEmkII_AVR32_JTAG aj;
};

//---------------------------------------------------------

class ProgJTAGICEmkIIs : public ProgJTAGICEmkIIu
{
public:
	ProgJTAGICEmkIIs();
	~ProgJTAGICEmkIIs() {};
	ProgPortSerial serial;
};

//-----------------------------------------------------------------------------

typedef struct
{
	u8 comm_id;
	u8 m_bl;
	u8 m_fw_min;
	u8 m_fw_maj;
	u8 m_hw;
	u8 s_bl;
	u8 s_fw_min;
	u8 s_fw_maj;
	u8 s_hw;
	u8 serial[6];
	char device_id[20]; //XXX ???
} Jtag2SignOn;

typedef struct
{
	u8 type;
	u8 mode;
	u32 address;
} Jtag2GetBreak;

//---------------------------------------------------------

class Jtag2Commands : public Commands
{
public:
	 Jtag2Commands() {};
	~Jtag2Commands() {};
	bool Connect(const char *c);

	bool SignOn();
	bool SignOff();
	bool GetSync();
	bool SetDescriptor(Device *);

	bool Enter();
	bool Leave();
	bool ChipErase();

	bool ReadMem (u8* buf, u8 type, u32 count, u32 address);
	bool WriteMem(u8* buf, u8 type, u32 count, u32 address);

	bool GetParameter (u8 p,u8  &v);
	bool GetParameter2(u8 p,u16 &v);
	bool GetParameter4(u8 p,u32 &v);

	bool SetParameter (u8 p,u8  v);
	bool SetParameter2(u8 p,u16 v);
	bool SetParameter4(u8 p,u32 v);

//---------------------------------------------------------

	inline bool SetEmulatorMode(u8 mode) { return SetParameter(3,mode); };
	bool SetDaisyChain(u8 ub, u8 ua, u8 bb, u8 ba);
	bool GetDaisyChain(u8 &ub, u8 &ua, u8 &bb, u8 &ba);

//---------------------------------------------------------

	bool Go();
	bool Stop(u8 hl);
	bool Reset(u8 hl);
	bool Step(u8 hl, u8 type);
	bool RunTo(u32 address);

	u32 GetPC();
	bool SetPC(u32);

	bool SetBreakPoint(u8 type, u8 number, u8 mode, u32 address);
	bool GetBreakPoint(u8 number,Jtag2GetBreak *b);
	bool ClearBreakPoint(u8 number, u32 address);
	bool ClearEvents(); //init
	bool RestoreTarget(); //exit
	bool ErasePageSPM(u32);

//	bool SetIR(u8);
//	bool JtagData(u8,u8*);
//	WriteSAB
//	ReadSAB
//	ReadSABBlock
//	WriteSABBlock
//	ChipEraseXMega

//	bool SelfTest(u8);

	void StartFrame(u8**);
	u32 EndFrame(u8**);
};

class STK500J2Commands : public STK500Commands
{
public:
	 STK500J2Commands() {};
	~STK500J2Commands() {};

	bool SignOn();
	bool SignOff();

	void StartFrame(u8**);
	u32 EndFrame(u8**);
	u8 *count;

	Jtag2Commands *cmdj2;
};

//-----------------------------------------------------------------------------
// mkII commands

#define CMND_SIGN_OFF			0x00
#define CMND_GET_SIGN_ON		0x01
#define CMND_SET_PARAMETER		0x02
#define CMND_GET_PARAMETER		0x03
#define CMND_WRITE_MEMORY		0x04
#define CMND_READ_MEMORY		0x05
#define CMND_WRITE_PC			0x06
#define CMND_READ_PC			0x07
#define CMND_GO					0x08
#define CMND_SINGLE_STEP		0x09
#define CMND_FORCED_STOP		0x0A
#define CMND_RESET				0x0B
#define CMND_SET_DEVICE_DESCRIPTOR 0x0C
#define CMND_ERASEPAGE_SPM		0x0D //0E
#define CMND_GET_SYNC			0x0F
#define CMND_SELFTEST			0x10
#define CMND_SET_BREAK			0x11
#define CMND_GET_BREAK			0x12
#define CMND_CHIP_ERASE			0x13
#define CMND_ENTER_PROGMODE		0x14
#define CMND_LEAVE_PROGMODE		0x15
#define CMND_SET_N_PARAMETERS	0x16 //17-19
#define CMND_CLR_BREAK			0x1A //1B
#define CMND_RUN_TO_ADDR		0x1C
//#define CMND_SPI_CMD			0x1D //1E-21
#define CMND_CLEAR_EVENTS		0x22
#define CMND_RESTORE_TARGET		0x23
// AVR32
#define CMND_JTAG_INSTR			0x24
#define CMND_JTAG_DATA			0x25 //26-27
#define CMND_JTAG_SAB_WRITE		0x28
#define CMND_JTAG_SAB_READ		0x29 //2A
#define CMND_RESET_AVR			0x2B // ????
#define CMND_JTAG_BLOCK_READ	0x2C
#define CMND_JTAG_BLOCK_WRITE	0x2D //2E

#define CMND_ISP_PACKET			0x2F // wrapper for STK500 commands
#define CMND_XMEGA_ERASE		0x34

//-----------------------------------------------------------------------------
// mkII responses

#define RSP_OK					0x80
#define RSP_PARAMETER			0x81
#define RSP_MEMORY				0x82
#define RSP_GET_BREAK			0x83
#define RSP_PC					0x84
#define RSP_SELFTEST			0x85
#define RSP_SIGN_ON				0x86
#define RSP_SPI_DATA			0x88

#define RSP_FAILED				0xA0
#define RSP_ILLEGAL_PARAMETER	0xA1
#define RSP_ILLEGAL_MEMORY_TYPE 0xA2
#define RSP_ILLEGAL_MEMORY_RANGE 0xA3
#define RSP_ILLEGAL_EMULATOR_MODE 0xA4
#define RSP_ILLEGAL_MCU_STATE	0xA5
#define RSP_ILLEGAL_VALUE		0xA6
#define RSP_SET_N_PARAMETERS	0xA7
#define RSP_ILLEGAL_BREAKPOINT	0xA8
#define RSP_ILLEGAL_JTAG_ID		0xA9
#define RSP_ILLEGAL_COMMAND		0xAA
#define RSP_NO_TARGET_POWER		0xAB
#define RSP_DEBUGWIRE_SYNC_FAILED 0xAC
#define RSP_ILLEGAL_POWER_STATE 0xAD

//-----------------------------------------------------------------------------
// mkII events

#define EVT_BREAK				0xE0
#define EVT_RUN					0xE1 //undocumented
#define EVT_TARGET_POWER_ON		0xE4
#define EVT_TARGET_POWER_OFF	0xE5
#define EVT_DEBUG				0xE6 //undocumented
#define EVT_EXT_RESET			0xE7
#define EVT_TARGET_SLEEP		0xE8
#define EVT_TARGET_WAKEUP		0xE9
#define EVT_POWER_ERROR_STATE	0xEA
#define EVT_POWER_OK			0xEB
#define EVT_IDR_DIRTY			0xEC
#define EVT_NONE				0xEF //undocumented

#define EVT_PROGRAM_BREAK		0xF1 //undocumented
#define EVT_PDSB_BREAK			0xF2 //undocumented
#define EVT_PDSMB_BREAK			0xF3 //undocumented

//DebugWire Errors
#define EVT_ERROR_PHY_FORCE_BREAK_TIMEOUT	0xE2
#define EVT_ERROR_PHY_RELEASE_BREAK_TIMEOUT	0xE3
#define EVT_ERROR_PHY_MAX_BIT_LENGTH_DIFF	0xED
#define EVT_ERROR_PHY_SYNC_TIMEOUT			0xF0
#define EVT_ERROR_PHY_SYNC_TIMEOUT_BAUD		0xF4
#define EVT_ERROR_PHY_SYNC_OUT_OF_RANGE		0xF5
#define EVT_ERROR_PHY_SYNC_WAIT_TIMEOUT		0xF6
#define EVT_ERROR_PHY_RECEIVE_TIMEOUT		0xF7
#define EVT_ERROR_PHY_RECEIVED_BREAK		0xF8
#define EVT_ERROR_PHY_OPT_RECEIVE_TIMEOUT	0xF9
#define EVT_ERROR_PHY_OPT_RECEIVED_BREAK	0xFA
#define EVT_RESULT_PHY_NO_ACTIVITY			0xFB

//-----------------------------------------------------------------------------
//Memory types for CMND_READ_MEMORY / CMND_WRITE_MEMORY

//OCD mode
#define MEMO_SRAM		0x20 //OCD SRAM
#define MEMO_EEPROM		0x22 //OCD EEPROM
#define MEMO_IO_SHADOW	0x30 //OCD SRAM for URSEL registers
#define MEMO_EVENT		0x60 //OCD ICE event memory
#define MEMO_BREAK		0x90 //OCD mkI
#define MEMO_SPM		0xA0 //OCD Flash using LPM/SPM

//Programming mode
#define MEMP_FLASH		0xB0
#define MEMP_EEPROM		0xB1
#define MEMP_FUSES		0xB2
#define MEMP_LOCK		0xB3
#define MEMP_SIGNATURE	0xB4
#define MEMP_OSCCAL		0xB5
#define MEMP_CAN		0xB6 //MEMP ???
#define MEMX_APP		0xC0
#define MEMX_BOOT		0xC1
#define MEMX_USERSIG	0xC5
#define MEMX_CALSIG		0xC6

//-----------------------------------------------------------------------------
//Parameters for CMND_GET_PARAMETER / CMND_SET_PARAMETER

#define PAR_HW_VERSION			0x01 //R
#define PAR_FW_VERSION			0x02 //R
#define PAR_EMULATOR_MODE		0x03 //RW
#define EMULATOR_MODE_DEBUGWIRE		0x00
#define EMULATOR_MODE_JTAG			0x01
#define EMULATOR_MODE_HV			0x02 //HVPP or HVSP mode of AVR Dragon
#define EMULATOR_MODE_SPI			0x03
#define EMULATOR_MODE_JTAG_AVR32	0x04
#define EMULATOR_MODE_JTAG_XMEGA	0x05
#define EMULATOR_MODE_PDI			0x06

#define PAR_IREG				0x04 //undocumented
#define PAR_BAUD_RATE			0x05 //RW
#define PAR_BAUD_2400				0x01
#define PAR_BAUD_4800				0x02
#define PAR_BAUD_9600				0x03
#define PAR_BAUD_19200				0x04 //default
#define PAR_BAUD_38400				0x05
#define PAR_BAUD_57600			 	0x06
#define PAR_BAUD_115200				0x07
#define PAR_BAUD_14400				0x08

#define PAR_OCD_VTARGET			0x06 //R
#define PAR_OCD_JTAG_CLK		0x07 //RW
#define PAR_OCD_BREAK_CAUSE		0x08 //R
#define PAR_TIMERS_RUNNING		0x09 //RW
#define PAR_BREAK_CHANGE_FLOW	0x0A //RW
#define PAR_BREAK_ADDR1			0x0B //RW not used
#define PAR_BREAK_ADDR2			0x0C //RW not used
#define PAR_COMBBREAKCTRL		0x0D //RW not used
#define PAR_JTAGID				0x0E //R
#define PAR_UNITS_BEFORE		0x0F //not used
#define PAR_UNITS_AFTER			0x10 //not used
#define PAR_BIT_BEFORE			0x11 //not used
#define PAR_BIT_ATER			0x12 //not used
#define PAR_EXTERNAL_RESET		0x13 //RW
#define PAR_FLASH_PAGE_SIZE		0x14 //RW
#define PAR_EEPROM_PAGE_SIZE	0x15 //RW
#define PAR_UNUSED1				0x16 //undocumented
#define PAR_PSB0				0x17 //RW not used
#define PAR_PSB1				0x18 //RW not used
#define PAR_DEBUG_EVENT			0x19 //undocumented
#define PAR_MCU_STATE			0x1A //R
#define STOPPED						0x00
#define RUNNING						0x01
#define PROGRAMMING					0x02

#define PAR_DAISY_CHAIN_INFO	0x1B //RW
#define PAR_BOOT_ADDRESS		0x1C //RW
#define PAR_TARGET_SIGNATURE	0x1D //R
#define PAR_DEBUGWIRE_BAUDRATE	0x1E //undocumented
#define PAR_PROGRAM_ENTRY_POINT 0x1F //W

#define PAR_CAN_MAILBOX			0x22 //RW
#define DONT_READ_CAN_MAILBOX		0x00
#define READ_CAN_MAILBOX			0x01

#define PAR_ENABLE_IDR_EVENTS	0x23 //W
#define ENABLE_OSCCAL				0x00
#define ENABLE_IDR					0x01

#define PAR_PAGEPROG_IN_CHAIN	0x24 //W
#define PAGEPROG_NOT_ALLOWED		0x00
#define PAGEPROG_ALLOWED			0x01

#define PAR_SOFT_RESET			0x2D //W

#define PAR_PDI_OFFSET_NVM		0X31 //W
#define PAR_PDI_OFFSET_APP		0x32 //W
#define PAR_PDI_OFFSET_BOOT		0x33 //W

#define PAR_AVR32_JTAG_ENABLE	0x37 //W
#define PAR_RUN_AFTER_PROG		0x38 //W

#define PAR_PARSING_ERRORS		0x40 //R
#define PAR_PACKETS_RECEIVED	0x41 //R
#define PAR_TX_FAILURES			0x42 //R
#define PAR_RX_FAILURES			0x43 //R
#define PAR_CRC_ERRORS			0x44 //R
#define PAR_POWER_SOURCE		0x45 //R
#define POWER_EXTERNAL				0x00
#define POWER_USB					0x01


//-----------------------------------------------------------------------------
/* Xmega erase memory types, for CMND_XMEGA_ERASE *//*
#define XMEGA_ERASE_CHIP 0x00
#define XMEGA_ERASE_APP 0x01
#define XMEGA_ERASE_BOOT 0x02
#define XMEGA_ERASE_EEPROM 0x03
#define XMEGA_ERASE_APP_PAGE 0x04
#define XMEGA_ERASE_BOOT_PAGE 0x05
#define XMEGA_ERASE_EEPROM_PAGE 0x06
#define XMEGA_ERASE_USERSIG 0x07*/

/* AVR32 related definitions *//*
#define AVR32_FLASHC_FCR                  0xFFFE1400
#define AVR32_FLASHC_FCMD                 0xFFFE1404
#define   AVR32_FLASHC_FCMD_KEY           0xA5000000
#define   AVR32_FLASHC_FCMD_WRITE_PAGE             1
#define   AVR32_FLASHC_FCMD_ERASE_PAGE             2
#define   AVR32_FLASHC_FCMD_CLEAR_PAGE_BUFFER      3
#define   AVR32_FLASHC_FCMD_LOCK                   4
#define   AVR32_FLASHC_FCMD_UNLOCK                 5
#define AVR32_FLASHC_FSR                  0xFFFE1408
#define   AVR32_FLASHC_FSR_RDY            0x00000001
#define   AVR32_FLASHC_FSR_ERR            0x00000008
#define AVR32_FLASHC_FGPFRHI              0xFFFE140C
#define AVR32_FLASHC_FGPFRLO              0xFFFE1410

#define AVR32_DC                          0x00000008
#define AVR32_DS                          0x00000010
#define AVR32_DINST                       0x00000104
#define AVR32_DCCPU                       0x00000110
#define AVR32_DCEMU                       0x00000114
#define AVR32_DCSR                        0x00000118

#define AVR32_DC_ABORT                    0x80000000
#define AVR32_DC_RESET                    0x40000000
#define AVR32_DC_DBE                      0x00002000
#define AVR32_DC_DBR                      0x00001000

#define AVR32_RESET_READ             0x0001
#define AVR32_RESET_WRITE            0x0002
#define AVR32_RESET_CHIP_ERASE       0x0004
#define AVR32_SET4RUNNING            0x0008
//#define AVR32_RESET_COMMON           (AVR32_RESET_READ | AVR32_RESET_WRITE | AVR32_RESET_CHIP_ERASE )
*/
//-----------------------------------------------------------------------------

#endif

