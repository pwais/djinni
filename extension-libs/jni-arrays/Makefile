.PHONY: all clean

FORCE_DJINNI := $(shell ./run_djinni.sh >&2)

all:
	ant compile test

clean:
	ant clean
