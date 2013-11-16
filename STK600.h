// vim:ts=4 sts=0 sw=4

#ifndef STK600_H
#define STK600_H

#define USB_DEVICE_AVRISPMKII  0x2104
#define USB_DEVICE_STK600      0x2106

class AVRISP2Commands;

//-----------------------------------------------------------------------------

//Moved to JTAGICEmkII.h to resolve a circular dependency...
//class ProgIF_AVRISPmkII_ISP : public ProgIF_STK500_ISP

//---------------------------------------------------------

class ProgIF_AVRISPmkII_TPI : public ProgIF
{
public:
	ProgIF_AVRISPmkII_TPI() { inm = "tpi"; };
	~ProgIF_AVRISPmkII_TPI() {};
	u32 GetDevIf() { return DIF_TPI; };

	bool Connect(const char *);
	bool Enter();
	bool Leave();

	u8 ReadSig(u8);
	u8 ReadCal(u8);
//	u32 ReadSignature();
	void SetDevice(Device *);

	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

	void ChipErase();
	bool ReadFlashBlock (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock(u8 *p, u32 size, u32 addr);

	AVRISP2Commands *cmdi2;
};

//---------------------------------------------------------

class ProgIF_AVRISPmkII_PDI : public ProgIF_AVRISPmkII_TPI
{
public:
	ProgIF_AVRISPmkII_PDI() { inm = "pdi"; };
	~ProgIF_AVRISPmkII_PDI() {};
	u32 GetDevIf() { return DIF_PDI; };

	bool Connect(const char *);
};

//-----------------------------------------------------------------------------

class ProgAVRISPmkII : public ProgUSB
{
public:
	ProgAVRISPmkII();
	~ProgAVRISPmkII() {};
	ProgIF_AVRISPmkII_ISP isp;
	ProgIF_AVRISPmkII_TPI tpi;
	ProgIF_AVRISPmkII_PDI pdi;

	AVRISP2Commands *cmdi2;
};

//-----------------------------------------------------------------------------

class ProgIF_STK600_ISP : public ProgIF_STK500_ISP
{
public:
	ProgIF_STK600_ISP();
	~ProgIF_STK600_ISP() {};

	//Programming clock
	int SetPClock(int u);
	int ApplyPClock();
	void Hz2P(int u);
	int P2Hz();

	//Clock generator
	int SetEClock(int);
	int ApplyEClock();
	void Hz2E(int u);
	int E2Hz();

	u16 eclk;
	u16 sck;
};

//---------------------------------------------------------

class ProgIF_STK600_HVPP : public ProgIF_STK500_HVPP
{
public:
	ProgIF_STK600_HVPP() { stkxml = "STK600"; };
	~ProgIF_STK600_HVPP() {};
};

//---------------------------------------------------------

class ProgIF_STK600_HVSP : public ProgIF_STK500_HVSP
{
public:
	ProgIF_STK600_HVSP() { stkxml = "STK600"; };
	~ProgIF_STK600_HVSP() {};
};

//---------------------------------------------------------
//These empty classes can be removed once I'm sure it won't be used.

class ProgIF_STK600_MEGA_JTAG : public ProgIF_JTAGICEmkII_MEGA_JTAG
{
public:
	ProgIF_STK600_MEGA_JTAG() {};
	~ProgIF_STK600_MEGA_JTAG() {};

//	Jtag2S6Commands *cmdj2s6;
};

//---------------------------------------------------------

class ProgIF_STK600_TPI : public ProgIF_AVRISPmkII_TPI
{
public:
	ProgIF_STK600_TPI() {};
	~ProgIF_STK600_TPI() {};
};

//---------------------------------------------------------

class ProgIF_STK600_PDI : public ProgIF_AVRISPmkII_PDI
{
public:
	ProgIF_STK600_PDI() {};
	~ProgIF_STK600_PDI() {};
};

//---------------------------------------------------------

class ProgIF_STK600_XMEGA_JTAG : public ProgIF_AVRISPmkII_PDI //ProgIF_JTAGICEmkII_XMEGA_JTAG
{
public:
	ProgIF_STK600_XMEGA_JTAG() { inm = "xj"; };
	~ProgIF_STK600_XMEGA_JTAG() {};
	u32 GetDevIf() { return DIF_XJ; };

	bool Connect(const char *);
};

//---------------------------------------------------------

class ProgIF_STK600_AVR32_JTAG : public ProgIF_JTAGICEmkII_AVR32_JTAG
{
public:
	ProgIF_STK600_AVR32_JTAG() {};
	~ProgIF_STK600_AVR32_JTAG() {};
};

//-----------------------------------------------------------------------------

class STK600Commands;
class Jtag2S6Commands;

class ProgSTK600 : public ProgAVRISPmkII
{
public:
	ProgSTK600();
	~ProgSTK600();

	ProgIF_STK600_ISP isp;
	ProgIF_STK600_HVPP hvpp;
	ProgIF_STK600_HVSP hvsp;
	ProgIF_STK600_MEGA_JTAG mj;
	ProgIF_STK600_TPI tpi;
	ProgIF_STK600_PDI pdi;
	ProgIF_STK600_XMEGA_JTAG xj;
	ProgIF_STK600_AVR32_JTAG aj;
	
	STK600Commands *cmds6;
	Jtag2S6Commands *cmdj2s6;
};

//-----------------------------------------------------------------------------

class AVRISP2Commands : public STK500Commands
{
public:
	 AVRISP2Commands() {};
	~AVRISP2Commands() {};

	bool SignOn() { return SignXn("AVRISP_MK2"); };

	bool XpSetMode(u8);
	bool XpEnter();
	bool XpLeave();
	void XpSetParameter1(u8,u8);
	void XpSetParameter2(u8,u16);
	void XpSetParameter4(u8,u32);
	void XpErase(u8 m, u32 a = 0);
	bool XpWrite(u8 *p, u8 t, u16 c, u32 a, u8 pm = 0);
	bool XpRead(u8 *p, u8 t, u16 c, u32 a);
	u32 XpCrc(u8);

	void StartFrame(u8**m);
	u32 EndFrame(u8**);
};

//---------------------------------------------------------

class STK600Commands : public AVRISP2Commands
{
public:
	 STK600Commands() {};
	~STK600Commands() {};

	bool SignOn() { return SignXn("STK600"); };
};

//---------------------------------------------------------

class Jtag2S6Commands : public Jtag2Commands
{
public:
	 Jtag2S6Commands() {};
	~Jtag2S6Commands() {};

	bool SignOn();// { return cmds6->SignOn(); };
	bool SignOff();// { return cmds6->SignOff(); };

	void StartFrame(u8**);
	u32 EndFrame(u8**);

	STK600Commands *cmds6;
};

//-----------------------------------------------------------------------------

#endif

