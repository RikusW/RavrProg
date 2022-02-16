
CXXFLAGS = -DXLIB -fPIC -DPIC -g -Wall -Wextra -Wno-unused-parameter -I../

lib-objs := AVRDragon.o Device.o JTAGICEmkI.o JTAGICEmkII.o LUFACDC.o RavrProg.o STK500.o STK600.o U2S.o

all: libRavrProg.so

clean:
	rm -fv *~
	rm -fv *.o
	rm -fv *.so
	rm -fv *.dep

#=========================================================#
#dependencies

%.dep: %.cpp
	@echo Making $@
	@echo -n "$@ " > $@
	@$(CXX) -M $(CXXFLAGS) $< >> $@

-include $(subst .o,.dep,$(lib-objs))

#=========================================================#

libRavrProg.so: $(lib-objs)
	g++ -shared $(CXXFLAGS) -o libRavrProg.so $^ -lusb -lRtk-base -Wl,-soname -Wl,libRavrProg.so.0.1
# -lefence

linkso:
	ln -s `pwd`/libRavrProg.so /usr/lib/libRavrProg.so
	ln -s `pwd`/libRavrProg.so /usr/lib/libRavrProg.so.1
	ln -s `pwd`/libRavrProg.so /usr/lib/libRavrProg.so.0.1

rmlinkso:
	rm /usr/lib/libRavrProg.so
	rm /usr/lib/libRavrProg.so.1
	rm /usr/lib/libRavrProg.so.0.1

#=========================================================#

wc:
	wc *.h *.cpp

#=========================================================#

