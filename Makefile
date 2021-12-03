# -*- coding:utf-8 -*-

ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

include ${ROOT_DIR}/core/build_subdir.mk
