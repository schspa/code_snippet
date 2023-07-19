CROSS:=
CC:= gcc
CFLAGS:= -Wall -c -Werror
LDFLAGS:=
OBJS:= hook.o
OUT:= hook.so
ifeq ($(DEBUG),1)
CFLAGS+= -g
endif

is_gnu_linux=$(shell if [[ $OSTYPE = linux-gnu ]]; then echo -n 1; else echo -n 0; fi)

ifeq ($(is_gnu_linux),1)
$(OUT): $(OBJS)
	$(CROSS)$(CC) -shared $^ -o $@ $(LDFLAGS)
%.o: %.c
	$(CROSS)$(CC) $^ -o $@ $(CFLAGS)

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(OUT)
else
All:
	echo "Skip for non-gnu system"
endif
