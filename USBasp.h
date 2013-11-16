// vim:ts=4 sts=0 sw=4

#ifndef USBASP_H
#define USBASP_H

class USBaspCommands;

//-----------------------------------------------------------------------------

class ProgIF_USBasp_ISP : public ProgIF
{
public:
	ProgIF_USBasp_ISP();
	~ProgIF_USBasp_ISP() {};
	u32 GetDevIf() { return DIF_ISP; };

	bool Enter();
	bool Leave();

	int SetPClock(int u);
	int ApplyPClock();
	void Hz2P(int u);
	int P2Hz();
	u8 sck;

	u8 ReadSig(u8);
	u8 ReadCal(u8);


	void ChipErase();
	bool SetAddress(u32 a);

	USBaspCommands *cmdua;

protected:
	u8 ReadFuse(u8, const char *);
	void WriteFuse(u8, u8, u8, const char *);
};

//---------------------------------------------------------

class ProgIF_USBasp_TPI : public ProgIF
{
public:
	ProgIF_USBasp_TPI() { inm = "tpi"; };
	~ProgIF_USBasp_TPI() {};
	u32 GetDevIf() { return DIF_TPI; };

	bool Enter() { return false; };
	bool Leave() { return false; };


	USBaspCommands *cmdua;
};

//-----------------------------------------------------------------------------

class ProgUSBasp : public ProgUSB
{
public:
	ProgUSBasp();
	~ProgUSBasp() {};
	bool Connect(const char *) { return false; };

	USBaspCommands *cmdua;
	ProgIF_USBasp_ISP isp;
	ProgIF_USBasp_TPI tpi;
};

//-----------------------------------------------------------------------------

class USBaspCommands : public Commands
{
public:
	 USBaspCommands() {};
	~USBaspCommands() {};

	bool SignOn() { return true; };
	bool SignOff() { return true; };
};

//-----------------------------------------------------------------------------
/*
 * usbasp.h - part of USBasp
 *
 * Autor..........: Thomas Fischl <tfischl@gmx.de>
 * Description....: Definitions and macros for usbasp
 * Licence........: GNU GPL v2 (see Readme.txt)
 * Creation Date..: 2009-02-28
 * Last change....: 2011-10-11
 * Changed by.....: Rikus Wessels to add RavrProg classes
 */

/* USB function call identifiers */
#define USBASP_FUNC_CONNECT     1
#define USBASP_FUNC_DISCONNECT  2
#define USBASP_FUNC_TRANSMIT    3
#define USBASP_FUNC_READFLASH   4
#define USBASP_FUNC_ENABLEPROG  5
#define USBASP_FUNC_WRITEFLASH  6
#define USBASP_FUNC_READEEPROM  7
#define USBASP_FUNC_WRITEEEPROM 8
#define USBASP_FUNC_SETLONGADDRESS 9
#define USBASP_FUNC_SETISPSCK 10
#define USBASP_FUNC_TPI_CONNECT      11
#define USBASP_FUNC_TPI_DISCONNECT   12
#define USBASP_FUNC_TPI_RAWREAD      13
#define USBASP_FUNC_TPI_RAWWRITE     14
#define USBASP_FUNC_TPI_READBLOCK    15
#define USBASP_FUNC_TPI_WRITEBLOCK   16
#define USBASP_FUNC_GETCAPABILITIES 127

/* USBASP capabilities */
#define USBASP_CAP_0_TPI    0x01

/* programming state
#define PROG_STATE_IDLE         0
#define PROG_STATE_WRITEFLASH   1
#define PROG_STATE_READFLASH    2
#define PROG_STATE_READEEPROM   3
#define PROG_STATE_WRITEEEPROM  4
#define PROG_STATE_TPI_READ     5
#define PROG_STATE_TPI_WRITE    6*/

/* Block mode flags */
#define PROG_BLOCKFLAG_FIRST    1
#define PROG_BLOCKFLAG_LAST     2

/* ISP SCK speed identifiers */
#define USBASP_ISP_SCK_AUTO   0
#define USBASP_ISP_SCK_0_5    1   /* 500 Hz */
#define USBASP_ISP_SCK_1      2   /*   1 kHz */
#define USBASP_ISP_SCK_2      3   /*   2 kHz */
#define USBASP_ISP_SCK_4      4   /*   4 kHz */
#define USBASP_ISP_SCK_8      5   /*   8 kHz */
#define USBASP_ISP_SCK_16     6   /*  16 kHz */
#define USBASP_ISP_SCK_32     7   /*  32 kHz */
#define USBASP_ISP_SCK_93_75  8   /*  93.75 kHz */
#define USBASP_ISP_SCK_187_5  9   /* 187.5  kHz */
#define USBASP_ISP_SCK_375    10  /* 375 kHz   */
#define USBASP_ISP_SCK_750    11  /* 750 kHz   */
#define USBASP_ISP_SCK_1500   12  /* 1.5 MHz   */

#define	USBASP_VID   0x16C0  // VOTI
#define	USBASP_PID   0x05DC  // Obdev's free shared PID

#define	USBASP_OLD_VID      0x03EB  // ATMEL
#define	USBASP_OLD_PID	    0xC7B4  // (unoffical) USBasp

#endif /* USBASP_H_ */

