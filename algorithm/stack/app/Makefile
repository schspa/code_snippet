CROSS =
CC = gcc
ODIR = .build

CFLAGS = -Wall \
		 -Werror

EXTRAINCDIRS +=

CFLAGS += $(patsubst %,-I %,$(EXTRAINCDIRS))
CFLAGS += -MD -MP -MF .dep/$(@F).d

LFLAGS =

stack-obj = seqstack/seqstack.o

bracketMatch-obj += $(stack-obj)
bracketMatch-obj += bracketMatch.o

expressionConvert-obj += $(stack-obj)
expressionConvert-obj += expressionConvert.o

OUT += bracketMatch
OUT += expressionConvert

all: $(OUT)

bracketMatch: $(addprefix $(ODIR)/,$(bracketMatch-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

expressionConvert: $(addprefix $(ODIR)/,$(expressionConvert-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^

$(ODIR)/$(stack-obj): ../seqstack/seqstack.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	$(RM) -r $(ODIR) .dep
	$(RM) $(OUT)

.PHONY: all clean
