# define the C compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -g -std=c99 $(mysql_config --cflags --libs) -D_DEFAULT_SOURCE 

# define any directories containing header files other than /usr/include
INCLUDES = -Ilua -Iheaders -IcJSON

# define library paths in addition to /usr/lib
LFLAGS = -llua -Llua -lm -ldl -L/usr/lib/x86_64-linux-gnu/ -D_POSIX_C_SOURCE=199309L

# define any libraries to link into executable:
LIBS = -Llua -ldl -lm -lmysqlclient 

# define the C source files
# SRCS = impl/ncmdbcurl.c impl/ncmdbreadline.c ncs.c
SRCS = $(wildcard impl/*.c) server.c cJSON/cJSON.c

# define the C object files 
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = server

.PHONY: depend clean

all: $(MAIN)
	@echo  Capi has been compiled.

$(MAIN): $(OBJS)
	@echo  Capi main compiling.
	$(CC) $(CFLAGS) -o $(MAIN) $(LFLAGS) $(INCLUDES) $(OBJS) $(LFLAGS) $(LIBS) -pthread

.c.o:
	@echo  Capi lib compiling.
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@ 

clean:
	$(RM) *.o *~ impl/*.o

cleanall:
	$(RM) *.o *~ impl/*.o $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^
