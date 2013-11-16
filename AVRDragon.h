// vim:ts=4 sts=0 sw=4

#ifndef AVRDragon_H
#define AVRDragon_H

#define USB_DEVICE_AVRDRAGON   0x2107

//-----------------------------------------------------------------------------

class ProgIF_Dragon_HVPP : public ProgIF_STK500_HVPP
{
public:
	ProgIF_Dragon_HVPP() {};
	~ProgIF_Dragon_HVPP() {};
	bool Connect(const char *);
	Jtag2Commands *cmdj2;
};

//---------------------------------------------------------

class ProgIF_Dragon_HVSP : public ProgIF_STK500_HVSP
{
public:
	ProgIF_Dragon_HVSP() {};
	~ProgIF_Dragon_HVSP() {};
	bool Connect(const char *);
	Jtag2Commands *cmdj2;
};

//-----------------------------------------------------------------------------

class ProgDragon : public ProgJTAGICEmkIIu
{
public:
	ProgDragon();
	~ProgDragon() {};
	ProgIF_Dragon_HVPP hvpp;
	ProgIF_Dragon_HVSP hvsp;
};

//-----------------------------------------------------------------------------

#endif

