# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -g

# define any directories containing header files other than /usr/include
INCLUDES = -Iheaders

# define library paths in addition to /usr/lib
LFLAGS = -L/usr/lib/x86_64-linux-gnu

# define any libraries to link into executable:
LIBS = -lcurl

# define the C source files
# SRCS = impl/ncmdbcurl.c impl/ncmdbreadline.c ncs.c
SRCS = $(wildcard impl/*.c) server.c

# define the C object files 
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = server

.PHONY: depend clean

all: $(MAIN)
	@echo  Capi has been compiled.

$(MAIN): $(OBJS)
	@echo  Capi main compiling.
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS) -pthread

.c.o:
	@echo  Capi lib compiling.
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@ 

clean:
	$(RM) *.o *~ impl/*.o

cleanall:
	$(RM) *.o *~ impl/*.o $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^
