#CROSS = arm-unknown-linux-uclibcgnueabi-
CC = gcc
ODIR = .build

CFLAGS = -Wall \
		 -Werror

EXTRAINCDIRS +=

CFLAGS += $(patsubst %,-I %,$(EXTRAINCDIRS))
CFLAGS += -MD -MP -MF .dep/$(@F).d

LFLAGS = -lcrypto

encrypt_decrypt-obj += encrypt_decrypt.o

decrypt-obj += decrypt.o

TARGET := encrypt_decrypt decrypt

all: $(TARGET)

encrypt_decrypt: $(addprefix $(ODIR)/,$(encrypt_decrypt-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

decrypt: $(addprefix $(ODIR)/,$(decrypt-obj))
	$(CROSS)$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(ODIR)/%.o: %.c
	@mkdir -p $(@D)
	@echo ">>> $<"
	$(CROSS)$(CC) $(CFLAGS) -c $< -o $@

include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)

clean:
	$(RM) -r $(ODIR) .dep
	$(RM) $(TARGET)
	$(RM) hello*
	$(RM) test*

.PHONY: all clean
