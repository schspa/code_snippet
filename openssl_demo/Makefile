CROSS :=
CC := gcc
CFLAGS := -Wall -g
LFLAGS := -lcrypto

out := main
objs := main.o

all: $(out)

$(out):$(objs)
	$(CROSS)$(CC) $< -o $@ $(LFLAGS)

%.o:%.c
	$(CROSS)$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf *.o $(objs) $(out)
	rm -rf hello*
	rm -rf test*

.PHONY: clean
