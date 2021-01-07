#CROSS = arm-unknown-linux-uclibcgnueabi-
CC = gcc
ODIR = .build

CFLAGS = -Wall \
		 -Werror

EXTRAINCDIRS += intf \
				lib

CFLAGS += $(patsubst %,-I %,$(EXTRAINCDIRS))
CFLAGS += -MD -MP -MF .dep/$(@F).d

LFLAGS =

server-obj += server.o

client-obj += client.o

all: server client

server: $(addprefix $(ODIR)/,$(server-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

client: $(addprefix $(ODIR)/,$(client-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

$(ODIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	$(RM) -r $(ODIR) .dep
	$(RM) server client

.PHONY: all clean
