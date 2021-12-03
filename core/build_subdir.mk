# -*- coding:utf-8 -*-

TOPTARGETS := all clean
SUBDIRS := $(patsubst %/Makefile,%,$(wildcard */Makefile))

$(TOPTARGETS): $(SUBDIRS)

$(SUBDIRS):
	echo Building $@
	ROOT_DIR=$(ROOT_DIR) $(MAKE) -C $@ $(MAKECMDGOALS)

.PHONY: $(TOPTARGETS) $(SUBDIRS)
