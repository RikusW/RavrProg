// vim:ts=4 sts=0 sw=4

#include "Rtk-base/rtypes.h"
#include "Device.h"
#include "Rtk-base/RConfig.h"
#include <stdio.h>
#include <string.h>


//-----------------------------------------------------------------------------

AvrFuse::AvrFuse(Device *d, AvrFuse *f, const char *c, u32 a)
{
	dev = d;
	address = (u8)a;
	mask = value = 0;
	next = child = 0;
	strcpy(name,c);

	if(f->child) {
		for(AvrFuse *t = f->child; t; t = t->next) {
			if(!t->next) {
				t->next = this;
				break;
			}
		}
	}else{
		f->child = this;
	}
}

//---------------------------------------------------------

void AvrFuse::Set(bool b)
{
	if(b) {
		dev->fusebuf[address] &= ~mask;
		dev->fusebuf[address] |= dev->maskbuf[address];
//		printf("AvrFuse::Set %02x > &= %02x\n",address, (u8)~mask);
	}else{
		dev->fusebuf[address] |= mask;
		dev->fusebuf[address] |= dev->maskbuf[address];
//		printf("AvrFuse::Set %02x > |= %02x\n",address, (u8)mask);
	}
}

//---------------------------------------------------------

bool AvrFuse::Get()
{
	return !(dev->fusebuf[address] & mask);
}

//---------------------------------------------------------

void AvrFuse::Select(int i)
{
	AvrFuse *f;
	if(child && (f = child->GetAt(i))) {
		dev->fusebuf[address] &= ~mask;
		dev->fusebuf[address] |= f->value;
		dev->fusebuf[address] |= dev->maskbuf[address];
//		printf("AvrFuse::Select %02x > m%02x v%02x\n",address,f->mask,f->value);
	}else{
//		printf("AvrFuse::Select Invalid index\n");
	}
}

//---------------------------------------------------------

int AvrFuse::GetSelect()
{
	if(!child) {
		return 0;
	}
	int i = 0;
	u8 v = dev->fusebuf[address] & mask;
	for(AvrFuse *f = child; f; f = f->next, i++) {
		if(f->value == v) {
			return i;
		}
	}
	Select(0); //Not found so set to first option
	return 0;
}

//---------------------------------------------------------

AvrFuse *AvrFuse::GetAt(int i)
{
	if(i) {
		if(next) {
			return next->GetAt(i-1);
		}else{
			return 0;
		}
	}else{
		return this;
	}
}

//-----------------------------------------------------------------------------

Device::Device()
{
	name[0] = 0;
	nextl = next = 0;
	signature = 0;
	if_flags = 0;
	jtag_id = 0;
	config = 0;
	fuses = 0;
	fcnt = 4; //XXX tiny/mega XXX
	for(int i = 0; i < 32; i++) {
		fusebuf[i] = 0xFF;
	}
}

//---------------------------------------------------------

Device::~Device()
{
	if(nextl) delete nextl;
	if(fuses) delete fuses;
	if(config) delete config;
}

//---------------------------------------------------------

Device *Device::GetAt(int i)
{
	if(i) {
		if(next) {
			return next->GetAt(i-1);
		}else{
			return 0;
		}
	}else{
		return this;
	}
}

//---------------------------------------------------------

void Device::Load()
{
	if(!strcmp(name,"Auto")) {
		return;
	}
	char buf[100];
#ifdef WIN32
	strcpy(buf,"C:/RavrProgParts/");
#else
	strcpy(buf,"../RavrProgParts/");
#endif
	strcat(buf,name);
	strcat(buf,".rcf");
	printf("Device::Load -> %s\n",buf);
	RConfig rcf;
	config = rcf.ReadRcf(buf);
}

//---------------------------------------------------------

void Device::GetFuseBits(u32 a, AvrFuse *r, RConfigNode *t, const char *nm)
{
	u8 u = 0;
	RString s,z,x;
	RConfigNode *c,*d;

	if(t) {
		AvrFuse *f = new AvrFuse(this,r,nm,a); //description

		s = t->line; //retrieve default value, eg: LOW=$62
		if(s.Contains("=")) {
			x.SplitL(s,'=');
			u = s;
		}else{
			u = 0xFF; //mainly for lockbits and incomplete files
			x = s;
		}
		printf("%s = 0x%02X\n",(const char*)x,u);
		f->value = f->def = u;

		for(c = (RConfigNode*)t->child; c; c = (RConfigNode*)c->next) {
			AvrFuse *g = new AvrFuse(this,r,c->line,a); //XXX r->f ???? ----XXX fixme XXX
			s = c->line;
			z.SplitL(s,'=');
			u8 ss = g->mask = (u8)s;
			g->value = g->def = g->mask & u; //default value

			u8 shift = 0; // list mask fix...
			if(c->child && ss) { //XXX patch until rcf files is fixed, if ever :-P
				while(!(ss & 1)) {
					ss >>= 1;
					shift++;
				}
			}

			for(d = (RConfigNode*)c->child; d; d = (RConfigNode*)d->next) { // combobox items
				AvrFuse *h = new AvrFuse(this,g,d->line,a);
				h->mask = g->mask;
				s = d->line;
				z.SplitL(s,'=');
				h->value = (u8)(((u32)z) << shift);
			}
		}
	}
}

//---------------------------------------------------------

AvrFuse *Device::GetFuses()
{
	if(!config) Load();
	if(!config) return 0;

	AvrFuse *r = new AvrFuse();
	strcpy(r->name,"root fuse");
	if(fuses) {
		delete fuses;
	}
	fuses = r;

	RConfigNode *t = config->GetNode("FUSES");
	if(!t) {
		return 0;
	}

	GetFuseBits(0,r,t->GetNode("LOW"),"<Low Fuse>");
	GetFuseBits(1,r,t->GetNode("HIGH"),"<High Fuse>");
	GetFuseBits(2,r,t->GetNode("EXTENDED"),"<Extended Fuse>");
	GetFuseBits(3,r,config->GetNode("LOCKBITS"),"<Lockbits>");

	for(int i = 0; i < 32; i++) {
		fusebuf[i] = 0xFF;
		maskbuf[i] = 0xFF;
	}

	//compute combined masks
	u8 p = 255; //XXX fixme
	for(AvrFuse *l = r->child; l; l = l->next) {
		maskbuf[l->address] &= ~l->mask;
		if(l->address != p) {
			p = l->address;
			fusebuf[l->address] = l->def; //XXX 2013
		}
	}

	return r;
}

//---------------------------------------------------------

void Device::SetDefaultFuses()
{
	if(!fuses) {
		return;
	}
	u8 p = 255; //XXX fixme
	for(AvrFuse *l = fuses->child; l; l = l->next) {
		if(l->address != p) {
			p = l->address;
			fusebuf[l->address] = l->def; //XXX 2013
		}
	}
}

//---------------------------------------------------------

u8 Device::GetFuseBuf(u8 b[32])
{
	for(int i = 0; i < 32; i++) {
		b[i] = fusebuf[i];
	}
	return fcnt;
}

//---------------------------------------------------------

void Device::SetFuseBuf(u8 b[32])
{
	for(int i = 0; i < 32; i++) {
		fusebuf[i] = b[i];
		fusebuf[i] |= maskbuf[i]; //set all unused/reserved bits to 1
	}
}

//---------------------------------------------------------

Device *FillList(RTreeNode *n)
{
	RString r,s;
	RTreeNode *t;
	Device *p,*dev;
	p = dev = new Device;
	n = n->child;

	strcpy(p->name,"Auto");
	p->if_flags = 0xFFFF; //match all
	p->signature = 0;
	p->jtag_id = 0;
	p->nextl = new Device;
	p = p->nextl;


	while(n) {
		strncpy(p->name,n->line,19);
		//printf("-%s-\n",p->name);
		if((t = n->GetNode("sig"))) {
			s = t->line;
			r.SplitL(s,'=');
			u32 u = s;
			//printf("    sig-> %s --- %X\n",s.GetStr(),u);
			p->signature = u;
		}else{
			p->signature = 0;
		}
		
		if((t = n->GetNode("if"))) {
			s = t->line;
			if(s.Contains("isp")) {
				p->if_flags |= DIF_ISP;
			}
			if(s.Contains("hvpp")) {
				p->if_flags |= DIF_HVPP;
			}
			if(s.Contains("hvsp")) {
				p->if_flags |= DIF_HVSP;
			}
			if(s.Contains("dw")) { //debugWire
				p->if_flags |= DIF_DW;
			}
			if(s.Contains("mj")) { //mega jtag
				p->if_flags |= DIF_MJ;
			}
			if(s.Contains("tpi")) {
				p->if_flags |= DIF_TPI;
			}
			if(s.Contains("pdi")) {
				p->if_flags |= DIF_PDI;
			}
			if(s.Contains("xj")) { //xmega jtag
				p->if_flags |= DIF_XJ;
			}
			if(s.Contains("aj")) { //avr32 jtag
				p->if_flags |= DIF_AJ;
			}
			if(s.Contains("aw")) { //aWire
				p->if_flags |= DIF_AW;
			}

		}else{
			p->if_flags = 0;
		}
		//printf("    if_flags -> %X\n",p->if_flags);

		if((t = n->GetNode("id"))) {
			s = t->line;
			r.SplitL(s,'=');
			u32 u = s;
			//printf("    jtid-> %s --- %X\n",s.GetStr(),u);
			p->jtag_id = u;
		}else{
			p->jtag_id = 0;
		}

		n = n->next;
		if(!n) {
			break;
		}
		p->nextl = new Device;
		p = p->nextl;
	}

	return dev;
}

//---------------------------------------------------------

Device *GetAvrList()
{
	RConfig rcf;
	RTreeNode *t;
#ifdef WIN32
	t = rcf.ReadRcf("C:/RavrProgParts/partlist.rcf");
#else
	t = rcf.ReadRcf("../RavrProgParts/partlist.rcf");
#endif
	if(t) {
		Device *d = FillList(t);
		delete t;
		return d;
	}
	return 0;
}

//-----------------------------------------------------------------------------

/*
int main()
{
	Device *dev = GetAvrList();
	delete dev;
}
*/




/*
//g++ Device.cpp Rtk *.cpp -o dd
void PrintList(RTreeNode *dev, int i)
{
	char buf[20];
	strcpy(buf,"         ");
	buf[i] = 0;
	while(dev) {
		printf("->%s %s <-\n",buf,dev->line);
		if(dev->child) {
			PrintList(dev->child,i+2);
		}
		dev = dev->next;
	}
}*/

