PREFIX    := /usr/local
BINPREFIX := $(PREFIX)/bin

SRC := $(wildcard *.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)
PRE := $(wildcard prev/*.sh)
GEN := gen/prev/scripts.h gen/server.h gen/helpers.h

CFLAGS  += -Os -MD -Wall -Wextra -Wno-unused-parameter
LDFLAGS += -lmagic -lcrypto

all: ctpv

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"

install: ctpv ctpvclear
	install -d $(BINPREFIX)
	install $^ $(BINPREFIX)

uninstall:
	$(RM) $(BINPREFIX)/ctpv $(BINPREFIX)/ctpvclear

clean:
	$(RM) ctpv $(OBJ) $(DEP) $(GEN)
	$(MAKE) -C embed clean

make_embed:
	$(MAKE) -C embed

ctpv: $(OBJ)

ctpv.c: $(GEN)

gen/prev/scripts.h: $(PRE) embed/embed
	@mkdir -p $(@D)
	embed/embed -p prev_scr_ $(PRE) > $@

gen/server.h: clear.sh end.sh embed/embed
	@mkdir -p $(@D)
	embed/embed -p scr_ clear.sh end.sh > $@

gen/helpers.h: helpers.sh embed/embed
	@mkdir -p $(@D)
	embed/embed -p scr_ helpers.sh > $@

embed/embed: make_embed
	@:

-include $(DEP)

.PHONY: all options install uninstall clean make_embed

.DELETE_ON_ERROR:
