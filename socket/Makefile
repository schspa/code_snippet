#CROSS = arm-unknown-linux-uclibcgnueabi-
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

raw_recv_and_check_lost-obj += raw/recv_and_check_lost.o
raw_recv_and_send-obj += raw/recv_and_send.o

UDPOUT := udp/server udp/client
RAWOUT := raw/recv_and_check_lost raw/recv_and_send

all: udp raw

udp: $(UDPOUT)

raw: $(RAWOUT)

udp/server: $(addprefix $(ODIR)/,$(udp_server-obj))
	@mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

udp/client: $(addprefix $(ODIR)/,$(udp_client-obj))
	@mkdir -p $(@D)
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

raw/recv_and_check_lost: $(addprefix $(ODIR)/,$(raw_recv_and_check_lost-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

raw/recv_and_send: $(addprefix $(ODIR)/,$(raw_recv_and_send-obj))
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
