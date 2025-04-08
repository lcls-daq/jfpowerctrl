CROSS	?= bfin-uclinux-
LD		:= $(CROSS)g++
CXX		:= $(CROSS)g++
PREFIX	:= /var/lib/tftpboot
INSTALL	:= install
INCDIRS	:= -I.
CFLAGS	:= -Wall
CXXFLAGS:= -Wall -g
LDFLAGS	:=
LDLIBS	:=
PROGS	:= powerctrl

SRCS	:= powerctrl.cpp Reader.cpp Server.cpp Simulator.cpp
OBJS	:= $(SRCS:.cpp=.o)

rules := all clean install

.PHONY: $(rules)

.SUFFIXES:

all: $(PROGS)

%.o: %.cpp
	$(CXX) $(INCDIRS) $(DEFINES) $(CXXFLAGS) -c $< -o $@

$(PROGS): $(OBJS)
	$(LD) -o $@ $^ $(LDFLAGS) $(LDLIBS)

install: $(PROGS)
	$(INSTALL) -t $(PREFIX) $^

clean:
	$(RM) $(PROGS) *.o *.gdb
