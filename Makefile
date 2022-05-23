PREFIX    := /usr/local
BINPREFIX := $(PREFIX)/bin

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)
PRE := $(wildcard prev/*.sh)
GEN := gen/prev/scripts.h

CFLAGS  += -Os -MD -Wall -Wextra -Wno-unused-parameter
LDFLAGS += -lmagic

all: ctpv

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"

install: ctpv
	install -d $(BINPREFIX)
	install $< $(BINPREFIX)

uninstall:
	$(RM) $(BINPREFIX)/ctpv

clean:
	$(RM) ctpv $(OBJ) $(DEP) $(GEN)
	$(MAKE) -C embed clean

make_embed:
	$(MAKE) -C embed

ctpv: $(OBJ)

ctpv.c: $(GEN)

gen/prev/scripts.h: embed/embed $(PRE)
	@mkdir -p $(@D)
	embed/embed -p prev_scr_ -h helpers.sh $(PRE) > $@

embed/embed: make_embed
	@:

-include $(DEP)

.PHONY: all options install uninstall clean make_embed

.DELETE_ON_ERROR:
