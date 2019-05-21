CROSS = arm-linux-gnueabihf-
CC = gcc
ODIR = .build

CFLAGS = -Wall \
		 -Werror

EXTRAINCDIRS +=

CFLAGS += $(patsubst %,-I %,$(EXTRAINCDIRS))
CFLAGS += -MD -MP -MF .dep/$(@F).d

LFLAGS =

udp_server-obj += udp/server.o
udp_client-obj += udp/client.o
udp_broadcast-obj += udp/broadcast.o

rawsocket-obj += raw/rawsocket.o

UDPOUT := udp/server udp/client udp/broadcast
RAWOUT := raw/rawsocket

all: udp raw

udp: $(UDPOUT)

raw: $(RAWOUT)

udp/server: $(addprefix $(ODIR)/,$(udp_server-obj))
	@mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

udp/client: $(addprefix $(ODIR)/,$(udp_client-obj))
	@mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

udp/broadcast: $(addprefix $(ODIR)/,$(udp_broadcast-obj))
	@mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

raw/rawsocket: $(addprefix $(ODIR)/,$(rawsocket-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

$(ODIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	$(RM) -r $(ODIR) .dep
	$(RM) $(UDPOUT) $(RAWOUT)

.PHONY: all clean
