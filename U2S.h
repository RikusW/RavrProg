// vim:ts=4 sts=0 sw=4

#ifndef U2S_H
#define U2S_H

class U2SCommands;

//-----------------------------------------------------------------------------

class ProgIF_U2S_ISP : public ProgIF_STK500_ISP
{
public:
	ProgIF_U2S_ISP() {};
	~ProgIF_U2S_ISP() {};
	bool Connect(const char *);
	U2SCommands *cmdu2s;
};

//---------------------------------------------------------

class ProgIF_U2S_BOOT : public ProgIF_U2S_ISP
{
public:
	ProgIF_U2S_BOOT() { inm = "boot"; PClockList = EClockList = 0; };
	~ProgIF_U2S_BOOT() {};
//	u32 GetDevIf() { return DIF_ISP; }; //XXX | DIF_HVPP | DIF_HVSP | DIF_DW | DIF_MJ | DIF_BOOT; };
	bool Connect(const char *);

	//not applicable for the bootloader
	int SetPClock(int) { return 0; };
	int ApplyPClock() { return 0; };

	int SetEClock(int) { return 0; };
	int ApplyEClock() { return 0; };
};

//---------------------------------------------------------

class ProgIF_U2S_FWUP : public ProgIF_U2S_BOOT
{
public:
	ProgIF_U2S_FWUP() { inm = "fwup"; };
	~ProgIF_U2S_FWUP() {};
	bool Connect(const char *);
};

//---------------------------------------------------------

class ProgIF_U2S_HVPP : public ProgIF_STK500_HVPP
{
public:
	ProgIF_U2S_HVPP() {};
	~ProgIF_U2S_HVPP() {};
	bool Connect(const char *);
	U2SCommands *cmdu2s;
};

//---------------------------------------------------------

class ProgIF_U2S_HVSP : public ProgIF_STK500_HVSP
{
public:
	ProgIF_U2S_HVSP() {};
	~ProgIF_U2S_HVSP() {};
	bool Connect(const char *);
	U2SCommands *cmdu2s;
};

//---------------------------------------------------------

class ProgIF_U2S_MEGA_JTAG : public ProgIF_JTAGICEmkI_MEGA_JTAG
{
public:
	ProgIF_U2S_MEGA_JTAG() {};
	~ProgIF_U2S_MEGA_JTAG() {};
	bool Connect(const char *);
	U2SCommands *cmdu2s;
};

//-----------------------------------------------------------------------------

class ProgU2S : public ProgSerial
{
public:
	ProgU2S();
	~ProgU2S();
	U2SCommands *cmdu2s;
	Jtag1Commands *cmdj1;
	ProgIF_U2S_BOOT boot;
	ProgIF_U2S_FWUP fwup;
	ProgIF_U2S_ISP isp;
	ProgIF_U2S_HVPP hvpp;
	ProgIF_U2S_HVSP hvsp;
	ProgIF_U2S_MEGA_JTAG mj;
};

//-----------------------------------------------------------------------------

class U2SCommands : public STK500Commands
{
public:
	 U2SCommands() {};
	~U2SCommands() {};

	bool U2SConnect(const char *c , u8 mode);

	u8 GetMode();
	bool SetMode(u8);

	u8 GetVersion(); //currently 3
	u16 GetAppSize();
	u8 ModUnlock();
};

//-----------------------------------------------------------------------------
#endif

