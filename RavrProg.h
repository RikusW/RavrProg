// vim:ts=4 sts=0 sw=4

#ifndef RavrProg_H
#define RavrProg_H

#include "Rtk/rtypes.h"
#include "Rtk/RHexFile.h"
#include "Device.h"

class Programmer;
class Commands;
class ProgPort;

//-----------------------------------------------------------------------------

//File types
#define HEXFW 0
#define BINFW 1

class ProgIF
{
public:
	ProgIF();
	virtual ~ProgIF() {};
	virtual u32 GetDevIf() { return 0; };

	Device *GetDeviceList(); //must be called to init the list
	Device *GetDevice(int i);
	Device *SetDevice(int i);
	Device *GetDevice(const char *c);
	Device *SetDevice(const char *c);
	void SetDefaults(Device *d);

	//Connecting
	virtual bool Connect(const char *);
	virtual void SetDevice(Device *) {};
	virtual void Disconnect();
	virtual bool Enter();
	virtual bool Leave();
	void Delay(u32);

//---------------------------------------------------------

	//Programming clock (ISP/JTAG)
	virtual bool PSaved() { return true; };
	int *GetPClockList() { return PClockList; };
	virtual int SetPClock(int) { return 0; };
	virtual int ApplyPClock() { return 0; }; //to programmer
	virtual int SetPClockI(int);
	int *PClockList;
	bool bDefaultP;

	//Clock generator
	virtual bool ESaved() { return true; };
	int *GetEClockList() { return EClockList; };
	virtual int SetEClock(int)  { return 0; };
	virtual int ApplyEClock() { return 0; }; //to programmer
	virtual int SetEClockI(int);
	int *EClockList;
	bool bDefaultE;

//---------------------------------------------------------

protected:
	Device *DetectDevice();
	virtual u32 ReadJtagID() { return -1; }; //or dW ID
	virtual u32 ReadSignature();
public:
	u32 GetJtagID() { return jtag_id; };
	u32 GetSignature() { return signature; };
	
	//Sig/Cal
	virtual u8 ReadSig(u8) { return 0; };
	virtual u8 ReadCal(u8) { return 0; };
	virtual void ReadSerial(u8*);

	//Fuse/Lock
	virtual void ReadFuses() { ReadFuses(device->fusebuf); };
	virtual void WriteFuses() { WriteFuses(device->fusebuf); };
	virtual bool VerifyFuses() { return VerifyFuses(device->fusebuf); };

protected:
	virtual u8 ReadFuses(u8 b[32]); //32 byte buffer
	virtual void WriteFuses(u8 b[32]);
	virtual bool VerifyFuses(u8 [32]);

	virtual u8 ReadFuse(u8, const char *) { return 0xFF; };
	virtual void WriteFuse(u8, u8, u8, const char *) {};
public:
//---------------------------------------------------------

	//Flash
	virtual void ChipErase() {};

	void ReadFlash  (const char *n, u8 type = HEXFW);
	void WriteFlash (const char *n, u8 type = HEXFW);
	void VerifyFlash(const char *n, u8 type = HEXFW);

	virtual bool ReadFlash(u8 *, u32);
	virtual bool WriteFlash(u8 *, u32);

	virtual bool ReadFlashBlock (u8 *p, u32 size, u32 addr);
	virtual bool WriteFlashBlock(u8 *p, u32 size, u32 addr);

	//Eeprom
	void ReadEeprom  (const char *n, u8 type = HEXFW);
	void WriteEeprom (const char *n, u8 type = HEXFW);
	void VerifyEeprom(const char *n, u8 type = HEXFW);

	virtual bool ReadEeprom(u8 *, u32);
	virtual bool WriteEeprom(u8 *, u32);

	virtual bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	virtual bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

	bool IsFFBlock(u8 *p, u32 size);

	u32 fsize,esize;
	u32 fpagesize,epagesize;
	u32 frblock,fwblock;
	u32 erblock,ewblock;
	u32 address;

//---------------------------------------------------------

	void *PCBvoid;
	void (*ProgressCallback)(u32 total, u32 current, void*);

	u8 *msg;
	ProgIF *GetAt(int);
	ProgIF *next;
	Programmer *prog;


	bool bConnected;

	Device *device;
	u32 jtag_id,signature;

	Commands *cmd;

	const char* name() { return iname; };
	const char* nm() { return inm; };
	const char *iname,*inm;


protected:

	virtual void StartFrame(u8**);
	virtual u32 EndFrame(u8**);

	//virtual bool SetParameter(u8 ,u8 ) {/* p = 0; v = 0;*/ return false; }; //keep gvv happy :S
	//virtual bool GetParameter(u8 ,u8 &v) {/* p = 0;*/ v = 0; return false; };
	virtual bool SetParameter(u8 p,u8 v) {/* p = 0; v = 0;*/ return false; };
	virtual bool GetParameter(u8 p,u8 &v) {/* p = 0;*/ v = 0; return false; };

	virtual bool Osccal() { return false; };
	virtual bool SetAddress(u32 a) { address = a; return true; };

};

//-----------------------------------------------------------------------------

class Programmer
{
public:
	Programmer();
	 ~Programmer();

	Programmer *GetAt(int);

	Commands *cmd;
	ProgPort *port;
	ProgIF *interface;
	Programmer *next;

	u32 Clock; //Programmer mcu clock
	u8 buf[11000]; // should be enough ?

	const char* name() { return pname; };
	const char* nm() { return pnm; };
	const char *pname,*pnm;
};

//-----------------------------------------------------------------------------

class Commands
{
public:
	Commands() {};
	virtual ~Commands() {};

//	void SetBP(u8 *b, ProgPort *p);
	void SetProg(Programmer *p);

	virtual bool Connect(const char *);
	virtual void Disconnect();

	virtual bool SignOn() { return false; };
	virtual bool SignOff() { return false; };

	virtual bool Enter() { return false; };
	virtual bool Leave() { return false; };

	virtual void StartFrame(u8**m) { *m = buf; };
	virtual u32 EndFrame(u8**) { return 0; };

	u8 *msg;
	u8 *buf;
	ProgPort *port;
};

//-----------------------------------------------------------------------------

class ProgPort
{
public:
	ProgPort() {};
	virtual ~ProgPort() {};

	virtual bool Open(const char *) { return false; };
	virtual void SetBaud(u32) {};
	virtual void Close() {};
	virtual u32 Read(u8*,u32) { return 0; };
	virtual u32 Write(u8*,u32) { return 0; };

	virtual const char *GetPort(u32) { return 0; };
};

//---------------------------------------------------------

class ProgPortSerial : public ProgPort
{
public:
	ProgPortSerial() {};
	~ProgPortSerial() {};

	bool Open(const char *);
	void SetBaud(u32);
	void Close();
	u32 Read(u8*,u32);
	u32 Write(u8*,u32);
	const char *GetPort(u32);
};

//---------------------------------------------------------

class ProgSerial : public Programmer
{
public:
	ProgSerial() { port = &serial; };
	~ProgSerial() {};

	ProgPortSerial serial;
};

//---------------------------------------------------------

#define USB_VENDOR_ATMEL 1003

class ProgPortUSB : public ProgPort
{
public:
	ProgPortUSB() {
		vid = USB_VENDOR_ATMEL;
		ep_read = 0x82;
		ep_write = 0x02;
		packet_size = 64;
		hu = 0;
	};
	~ProgPortUSB() {};

	bool Open(const char *);
	void SetBaud(u32) {};
	void Close();
	u32 Read(u8*,u32);
	u32 Write(u8*,u32);
	const char *GetPort(u32 u);

	void *hu;
	u16 vid,pid;
	u8 ep_read,ep_write,packet_size;
};

//---------------------------------------------------------

class ProgUSB : public Programmer
{
public:
	ProgUSB() { port = &usb; };
	~ProgUSB() {};

	ProgPortUSB usb;
};

//-----------------------------------------------------------------------------

#include "STK500.h"
#include "command.h"
#include "JTAGICEmkI.h"
#include "JTAGICEmkII.h"
#include "AVRDragon.h"
#include "STK600.h"
#include "U2S.h"
#include "USBasp.h"
#include "LUFACDC.h"

//---------------------------------------------------------

Programmer *GetProgrammerList();
void FreeProgrammerList();

//-----------------------------------------------------------------------------

#endif

