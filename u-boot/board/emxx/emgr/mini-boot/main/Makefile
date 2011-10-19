# EM/EV ROM BootLoader Makefile

DEBUG_FLAGS =

# object
MAIN_C_OBJS = $(MAIN_C_SRCS:.c=.o)
MAIN_S_OBJS = $(MAIN_S_SRCS:.S=.o)

#make Rules
all :  $(MAIN_C_OBJS) $(MAIN_S_OBJS)

.c.o :
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $<

.S.o :
	$(CC) $(SFLAGS) $(DEBUG_FLAGS) -c $<


# clean
clean :
	rm -f *.o
