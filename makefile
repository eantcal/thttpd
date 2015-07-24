# makefile for TinyHttpServer 
#
CC = g++

# the linker is also "g++". It might be something else with other compilers.
LD = g++

# Compiler flags
CFLAGS = -g -I. -O2 -pthread  -DMINGW
CPPFLAGS = $(CFLAGS) -std=c++11

# Linker flags
LDFLAGS = -lstdc++ -pthread

RM = /bin/rm -f

# list of generated object files
OBJS = gen_utils.o \
       http_server.o \
       tiny_http_server.o \
       http_mime.o \
       os_dep.o \
       socket_utils.o

# program executable file name
PROG = thttpd

# top-level rule, to compile everything
all: $(PROG)

clean:
	rm -r *.o
	rm -r $(PROG)

# linking rule 
$(PROG): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(PROG)

# meta-rule for compiling any "C" source file.
%.o: %.c
	$(CC) $(CFLAGS) -c $<

# meta-rule for compiling any "C++" source file.
%.o: %.cc
	$(CC) $(CPPFLAGS) -c $<

