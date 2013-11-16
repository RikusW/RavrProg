// vim:ts=4 sts=0 sw=4

#ifndef JTAGICEmkI_H
#define JTAGICEmkI_H

class Jtag1Commands;

//-----------------------------------------------------------------------------

class ProgIF_JTAGICEmkI_MEGA_JTAG : public ProgIF
{
public:
	ProgIF_JTAGICEmkI_MEGA_JTAG() { inm = "mj"; bDefaultP = false; jclk = 0xFE; };
	~ProgIF_JTAGICEmkI_MEGA_JTAG() {};
	u32 GetDevIf() { return DIF_MJ; };
	void SetDevice(Device *);

	int SetPClock(int u);
	int ApplyPClock();
	void Hz2P(int);
	int P2Hz();

	u8 ReadSig(u8);
	u8 ReadCal(u8);
	u32 ReadJtagID();

	void ChipErase();
	bool ReadFlashBlock  (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock (u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

	Jtag1Commands *cmdj1;
	void SetCmd(Programmer *p, Jtag1Commands *c) { prog = p; cmd = (Commands*)c; cmdj1 = c; };

protected:
	u8 jclk;
	bool bDefaultP;

	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

};

//-----------------------------------------------------------------------------

class ProgJTAGICEmkI : public ProgSerial
{
public:
	ProgJTAGICEmkI();
	~ProgJTAGICEmkI() {};

	Jtag1Commands *cmdj1;
	ProgIF_JTAGICEmkI_MEGA_JTAG mj;
};

//-----------------------------------------------------------------------------

class Jtag1Commands : public Commands
{
public:
	 Jtag1Commands() {};
	~Jtag1Commands() {};
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

	bool GetParameter (u8 p,u8 &v);
	bool SetParameter (u8 p,u8  v);

//---------------------------------------------------------

	void StartFrame(u8** u);
	u32  EndFrame(u8** u);
	bool EndFrame2(u8** u);
	u32  TxFrame(u8** u);
	bool RxFrame(u8** u,u16 c);

//---------------------------------------------------------
/*
	inline bool SetEmulatorMode(u8 mode);
	bool SetDaisyChain(u8 ub, u8 ua, u8 bb, u8 ba);
	bool GetDaisyChain(u8 &ub, u8 &ua, u8 &bb, u8 &ba);
*/
//---------------------------------------------------------

	bool Go();
	u32  Stop();
	bool Reset();
	bool Step();

	u32 GetPC();
	bool SetPC(u32);

	bool ErasePageSPM(u32);
};

//-----------------------------------------------------------------------------

#define CMND1_GET_SYNC			0x20 //space
#define CMND1_GET_SIGN_ON		0x53 //S -> AVRNOCD
#define CMND1_SET_PARAMETER		0x42 //B
#define CMND1_GET_PARAMETER		0x71 //q
#define CMND1_WRITE_MEMORY		0x57 //W
#define CMND1_READ_MEMORY		0x52 //R
#define CMND1_WRITE_PC			0x33 //3
#define CMND1_READ_PC			0x32 //2
#define CMND1_SINGLE_STEP		0x31 //1
#define CMND1_FORCED_STOP		0x46 //F
#define CMND1_RESET				0x78 //x
#define CMND1_GO				0x47 //G
#define CMND1_DATA				0x68 //h

#define CMND1_SET_DEVICE_DESCRIPTOR 0xA0
#define CMND1_ERASEPAGE_SPM		0xA1
#define CMND1_ENTER_PROGMODE	0xA3
#define CMND1_LEAVE_PROGMODE	0xA4
#define CMND1_CHIP_ERASE		0xA5

#define CMND1_JTAG_INSTR		0xC0 //undocumented
#define CMND1_JTAG_DATA			0xC1 //undocumented
#define CMND1_OSCCAL			0xC2 //undocumented

//---------------------------------------------------------

#define RSP1_OK					0x41
#define RSP1_BREAK				0x42
#define RSP1_SYNC_ERR			0x45
#define RSP1_FAILED				0x46
#define RSP1_INFO				0x47
#define RSP1_SLEEP				0x48
#define RSP1_POWER				0x49

//---------------------------------------------------------

#define PAR1_BAUD					0x62 //RW
#define PAR1_BAUD_9600	0xF4
#define PAR1_BAUD_14400	0xF8
#define PAR1_BAUD_19200	0xFA
#define PAR1_BAUD_38400	0xFD
#define PAR1_BAUD_57600	0xFE
#define PAR1_BAUD_115200	0xFF
#define PAR1_EECR					0x70 //RW in U2S only (1C/1F)
#define PAR1_HW_VER				0x7A //R
#define PAR1_FW_VER				0x7B //R
#define PAR1_IREG_H				0x81 //RW unused
#define PAR1_IREG_L				0x82 //RW unused
#define PAR1_VTG					0x84 //R

#define PAR1_JTAG_CLK				0x86 //RW
#define PAR1_JCLK_1MHZ	0xFF
#define PAR1_JCLK_500k	0xFE
#define PAR1_JCLK_250k	0xFD
#define PAR1_JCLK_166k	0xFC
#define PAR1_JCLK_125k	0xFB
#define PAR1_JCLK_100k	0xFA
#define PAR1_JCLK_50k		0xF3

#define PAR1_BREAK_CAUSE			0x87 //R - BSR low byte
#define PAR1_FLASH_PAGE_L			0x88 //W
#define PAR1_FLASH_PAGE_H			0x89 //W
#define PAR1_EEPROM_PAGE			0x8A //W
#define PAR1_EXT_RST				0x8B //W
#define PAR1_TIMERS_RUN			0xA0 //W
#define PAR1_BOCF					0xA1 //W

#define PAR1_BREAK1_H				0xA2 //W
#define PAR1_BREAK1_L				0xA3 //W
#define PAR1_BREAK2_H				0xA4 //W
#define PAR1_BREAK2_L				0xA5 //W

#define PAR1_COMBBREAKCTRL		0xA6 //W
#define PAR1_CBC_BMASK	0x40
#define PAR1_CBC_EN_PDMSB	0x20
#define PAR1_CBC_EN_PDSB	0x10
#define PAR1_CBC_PDMSB1	0x08
#define PAR1_CBC_PDMSB0	0x04
#define PAR1_CBC_PDSB1	0x02
#define PAR1_CBC_PDSB0	0x01

#define PAR1_JTAGID0				0xA7 //R Get this one FIRST
#define PAR1_JTAGID1				0xA8 //R
#define PAR1_JTAGID2				0xA9 //R
#define PAR1_JTAGID3				0xAA //R

#define PAR1_UB					0xAB //W
#define PAR1_UA					0xAC //W
#define PAR1_BB					0xAD //W
#define PAR1_BA					0xAE //W

#define PAR1_PSB0L				0xAF //W Write high THEN low.
#define PAR1_PSB0H				0xB0 //W
#define PAR1_PSB1L				0xB1 //W
#define PAR1_PSB1H				0xB2 //W These are loaded when the low byte is set

#define PAR1_MCU_MODE				0xB3 //R
#define PAR1_MCU_STOPPED		0
#define PAR1_MCU_RUNNING		1
#define PAR1_MCU_PROGRAMMING	2

/*
0xB4? W Reset Break address
0xB5? W	Reset Break address
0xB6? W ??Changes pml function??
0xB7? W
0xB8? W
*/



//-----------------------------------------------------------------------------

#endif

