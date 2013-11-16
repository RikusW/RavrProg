// vim:ts=4 sts=0 sw=4

#ifndef BITBANG_H
#define BITBANG_H


#include "STK500.h"


class ProgIF_BitBang_ISP : public ProgIF_AVRISPmkI_ISP  //ProgIF_STK500
{
public:
	ProgIF_BitBang_ISP() { inm = "isp"; };
	~ProgIF_BitBang_ISP() {};
	u32 GetDevIf() { return DIF_ISP; };
	bool Connect(const char *);
//	void SetDevice(Device *);

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



	void DelayMs(u32 u) {};
	void WriteByte(IspProgram *ip,u8 d);
	void WritePage(IspProgram *ip);
	bool PollValue(IspProgram *ip);
	bool RdyBsy(IspProgram *ip);


protected:
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);

	u8 sck;
	xml_isp xml;

	u32 address;
};




#endif
