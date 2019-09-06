CXXFILES = test1.cpp
OBJS = $(CXXFILES:.cpp=.o)
EXEC = test1

CXXFLAGS += -mavx -g -O2 -Wall -std=c++11

CXXFLAGS += -I$(XGREP_DIR)/include

LDLIBS += -L$(XILINX_XRT)/lib -L$(XGREP_DIR)/obj -L$(HOME)/data/p/toolchain/installed/lib \
		  -pthread -lxilinxopencl -lxre -lOpenCL -lre2

#
# The re2 shipped with ubuntu16 (apt install -y libre2-dev) is rather old, and will not
# accept .*final.*.   The following will include/link a customer built re2 lib (as part of
# vitessedata toolchain.)   
#
# CXXFLAGS += -I$(XGREP_DIR)/include -I$(HOME)/data/p/toolchain/installed/include
# 
# LDLIBS += -L$(XILINX_XRT)/lib -L$(XGREP_DIR)/obj -L$(HOME)/data/p/toolchain/installed/lib \
#		  -pthread -lxilinxopencl -lxre -lOpenCL -lre2
#

all: test1

test1: test1.o
	g++ -o $@ $^ $(LDLIBS)

clean:
	rm -f $(EXEC) *.o *.a *.log *.log.*
