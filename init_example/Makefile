CROSS:=
CC:= gcc
CFLAGS:= -Wall -c -Werror
LDFLAGS:=
OBJS:= main.o a.o b.o
OUT:= test
ifeq ($(DEBUG),1)
CFLAGS+= -g
endif

# try-run
# Usage: option = $(call try-run, $(CC)...-o "$$TMP",option-ok,otherwise)
# Exit code chooses option. "$$TMP" serves as a temporary file and is
# automatically cleaned up.
try-run = $(shell set -e;		\
	TMP=$(TMPOUT)/tmp;		\
	TMPO=$(TMPOUT)/tmp.o;		\
	mkdir -p $(TMPOUT);		\
	trap "rm -rf $(TMPOUT)" EXIT;	\
	if ($(1)) >/dev/null 2>&1;	\
	then echo "$(2)";		\
	else echo "$(3)";		\
	fi)

is_gnu_linux = $(call try-run, test $OSTYPE = linux-gnu,1,0)

ifeq ($(is_gnu_linux),1)
$(OUT): $(OBJS)
	$(CROSS)$(CC) $^ -o $@ $(LDFLAGS)
%.o: %.c
	$(CROSS)$(CC) $^ -o $@ $(CFLAGS)

.PHONY: clean
clean:
	-rm -rf $(OBJS) $(OUT)
else
All:
	echo "Skip for non-gnu system"
endif
