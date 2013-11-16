// vim:ts=4 sts=0 sw=4

#ifndef STK500_H
#define STK500_H

class STK500Commands;

//-----------------------------------------------------------------------------

class ProgIF_STK500 : public ProgIF //Remove this class ???
{
public:
	ProgIF_STK500() { stkxml = "STK500"; };
	~ProgIF_STK500() {};

	const char *stkxml;

	STK500Commands *cmds5;
	void SetCmd(Programmer *p, STK500Commands *s) { prog = p; cmd = (Commands*)s; cmds5 = s; };
protected:
	bool SetAddress(u32 a);
};

//-----------------------------------------------------------------------------

typedef struct {
	u8 eraseDelay,pollMethod;
} IspChipErase;

typedef struct {
	u8 mode,delay,cmd1,cmd2,cmd3,pollVal1,pollVal2;
	u16 blockSize;
} IspProgram;

typedef struct {
	IspChipErase ce;
	IspProgram pf;
	IspProgram pe;
} xml_isp;

//---------------------------------------------------------

class ProgIF_AVRISPmkI_ISP : public ProgIF_STK500
{
public:
	ProgIF_AVRISPmkI_ISP() { inm = "isp"; };
	~ProgIF_AVRISPmkI_ISP() {};
	u32 GetDevIf() { return DIF_ISP; };
	bool Connect(const char *);
	void SetDevice(Device *);

	bool Enter();
	bool Leave();

	//Programming clock ISP
	int SetPClock(int);
	int ApplyPClock();
	void Hz2P(int i);
	int P2Hz();

	u8 ReadSig(u8);
	u8 ReadCal(u8);

	void ChipErase();
	bool ReadFlashBlock (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock(u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

protected:
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

	u8 sck;
	xml_isp xml;
};

//---------------------------------------------------------

class ProgIF_STK500_ISP : public ProgIF_AVRISPmkI_ISP
{
public:
	ProgIF_STK500_ISP() {};
	~ProgIF_STK500_ISP() {};

	//Clock generator
	int SetEClock(int);
	int ApplyEClock();
	void Hz2E(int i);
	int E2Hz();

	u8 oscp,oscc;
};

//-----------------------------------------------------------------------------

typedef struct {
	u8 stabDelay,progModeDelay,latchCycles,toggleVtg,powerOffDelay,resetDelayMs,resetDelayUs;
} PpEnterProgMode;

typedef struct {
	u8 pulseWidth,pollTimeout;
} PpChipErase;

typedef struct {
	u8 pollTimeout,mode;
	u16 blockSize;
} PpProgramFlash;

typedef struct {
	u8 pollTimeout,mode;
	u16 blockSize;
} PpProgramEeprom;

typedef struct {
	u8 pulseWidth,pollTimeout;
} PpProgramFuse;

typedef struct {
	u8 pulseWidth,pollTimeout;
} PpProgramLock;

typedef struct {
	PpEnterProgMode e;
	PpChipErase ce;
	PpProgramFlash pf;
	PpProgramEeprom pe;
	PpProgramFuse pu;
	PpProgramLock pl;
} xml_hvpp;

//---------------------------------------------------------

class ProgIF_STK500_HVPP : public ProgIF_STK500
{
public:
	ProgIF_STK500_HVPP() { inm = "hvpp"; };
	~ProgIF_STK500_HVPP() {};
	u32 GetDevIf() { return DIF_HVPP; };

	void SetDevice(Device *);
	void SetControlStack(u8*);
	bool Enter();
	bool Leave();

	u8 ReadSig(u8);
	u8 ReadCal(u8);

	void ChipErase();
	bool ReadFlashBlock (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock(u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

protected:
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

	xml_hvpp xml;
};

//-----------------------------------------------------------------------------

typedef struct {
	u8 stabDelay,cmdexeDelay,synchCycles,latchCycles,toggleVtg,powoffDelay,resetDelay1,resetDelay2;
} HvspEnterProgMode;

typedef struct {
	u8 pollTimeout,eraseTime;
} HvspChipErase;

typedef struct {
	u8 mode,pollTimeout;
	u16 blockSize;
} HvspProgramFlash;

typedef struct {
	u8 mode,pollTimeout;
	u16 blockSize;
} HvspProgramEeprom;

typedef struct {
	HvspEnterProgMode e;
	HvspChipErase ce;
	HvspProgramFlash pf;
	HvspProgramEeprom pe;
} xml_hvsp;

//---------------------------------------------------------

class ProgIF_STK500_HVSP : public ProgIF_STK500
{
public:
	ProgIF_STK500_HVSP() { inm = "hvpp"; };
	~ProgIF_STK500_HVSP() {};
	u32 GetDevIf() { return DIF_HVSP; };

	void SetDevice(Device *);
	void SetControlStack(u8*);
	bool Enter();
	bool Leave();

	u8 ReadSig(u8);
	u8 ReadCal(u8);

	void ChipErase();;
	bool ReadFlashBlock (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock(u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

protected:
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

	xml_hvsp xml;
};

//-----------------------------------------------------------------------------

class ProgAVRISPmkI : public ProgSerial
{
public:
	ProgAVRISPmkI();
	~ProgAVRISPmkI() {};
	ProgIF_AVRISPmkI_ISP isp;
};

//---------------------------------------------------------

class ProgSTK500 : public ProgSerial
{
public:
	ProgSTK500();
	~ProgSTK500() {};
	ProgIF_STK500_ISP isp;
	ProgIF_STK500_HVPP hvpp;
	ProgIF_STK500_HVSP hvsp;
};

//-----------------------------------------------------------------------------

class STK500Commands : public Commands
{
public:
	 STK500Commands() {};
	~STK500Commands() {};
	bool Connect(const char *c);

	bool SetParameter(u8 p,u8 v);
	bool GetParameter(u8 p,u8 &v);

	bool Osccal();
	bool SetAddress(u32 a);

	bool SignOn() { return SignXn("STK500_2"); };
	bool SignXn(const char *c);
	bool SignOff();

	void StartFrame(u8**m);
	u32 EndFrame(u8**);

};

//---------------------------------------------------------

class AVRISP1Commands : public STK500Commands
{
public:
	 AVRISP1Commands() {};
	~AVRISP1Commands() {};

	bool SignOn() { return SignXn("AVRISP_2"); };
};

//-----------------------------------------------------------------------------

#endif

