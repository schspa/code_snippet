CROSS:=
CC:= gcc
CFLAGS:= -Wall -c -Werror
LDFLAGS:=
OBJS:= main.o a.o b.o
OUT:= test
ifeq ($(DEBUG),1)
CFLAGS+= -g
endif

$(OUT): $(OBJS)
	$(CROSS)$(CC) $^ -o $@ $(LDFLAGS)
%.o: %.c
	$(CROSS)$(CC) $^ -o $@ $(CFLAGS)

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(OUT)
