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

canrecv-obj += canrecv.o

cansend-obj += cansend.o

TARGET = canrecv cansend

all: $(TARGET)

canrecv: $(addprefix $(ODIR)/,$(canrecv-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

cansend: $(addprefix $(ODIR)/,$(cansend-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

$(ODIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	$(RM) -r $(ODIR) .dep
	$(RM) $(TARGET)

.PHONY: all clean
