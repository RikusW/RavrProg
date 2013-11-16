// vim:ts=4 sts=0 sw=4

#ifndef LUFACDC_H
#define LUFACDC_H

class LUFACDCCommands;

//-----------------------------------------------------------------------------

class ProgIF_LUFACDC_Boot : public ProgIF
{
public:
	ProgIF_LUFACDC_Boot();
	~ProgIF_LUFACDC_Boot() {};
	u32 GetDevIf() { return DIF_ISP; };
	void SetDevice(Device *);
	bool SetAddress(u32 a);
	void Disconnect();

	bool Enter();
	bool Leave();

	u32 ReadSignature();

	u8 ReadSig(u8) { return 0x55; };
	u8 ReadCal(u8);

	void ChipErase();
	bool ReadFlashBlock  (u8 *p, u32 size, u32 addr);
	bool WriteFlashBlock (u8 *p, u32 size, u32 addr);
	bool ReadEepromBlock (u8 *p, u32 size, u32 addr);
	bool WriteEepromBlock(u8 *p, u32 size, u32 addr);

	LUFACDCCommands *cmdlc;

protected:
	u32 EndFrame(u8** u, u16 c);
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);
};

//-----------------------------------------------------------------------------

class ProgLUFACDC : public ProgSerial
{
public:
	ProgLUFACDC();
	~ProgLUFACDC() {};
//	bool Connect(const char *) { return false; };

	LUFACDCCommands *cmdlc;
	ProgIF_LUFACDC_Boot boot;
};

//-----------------------------------------------------------------------------

class LUFACDCCommands : public Commands
{
public:
	 LUFACDCCommands() {};
	~LUFACDCCommands() {};

	bool SignOn();// { return true; };
	bool SignOff();// { return true; };

	void StartFrame(u8 **);
	u32 EndFrame(u8** u, u16 c);
	bool RxFrame(u8** u, u16 c);
};

//-----------------------------------------------------------------------------

#endif /* LUFACDC_H */

