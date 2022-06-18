PREFIX    := /usr/local
BINPREFIX := $(DESTDIR)$(PREFIX)/bin
MANPREFIX := $(DESTDIR)$(PREFIX)/man/man1

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)
PRE := $(wildcard prev/*.sh)
GEN := gen/prev/scripts.h gen/server.h gen/helpers.h

O    := -O2
LIBS := magic crypto

CFLAGS  += $(O) -MD -Wall -Wextra -Wno-unused-parameter
LDFLAGS += $(LIBS:%=-l%)

all: ctpv README.md doc/ctpv.1

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"

install: install.bin install.man

install.bin: ctpv ctpvclear
	install -d $(BINPREFIX)
	install $^ $(BINPREFIX)

install.man: doc/ctpv.1
	install -d $(MANPREFIX)
	install -m 0644 $^ $(MANPREFIX)

uninstall:
	$(RM) $(BINPREFIX)/ctpv $(BINPREFIX)/ctpvclear

clean:
	$(RM) ctpv $(OBJ) $(DEP) $(GEN)
	$(MAKE) -C embed clean

make_embed:
	$(MAKE) -C embed

ctpv: $(OBJ)
	$(CC) $(LDFLAGS) $+ -o $@

src/ctpv.c: $(GEN)

gen/prev/scripts.h: $(PRE) embed/embed
	@mkdir -p $(@D)
	embed/embed -p prev_scr_ $(PRE) > $@

gen/server.h: sh/clear.sh sh/end.sh embed/embed
	@mkdir -p $(@D)
	embed/embed -p scr_ sh/clear.sh sh/end.sh > $@

gen/helpers.h: sh/helpers.sh embed/embed
	@mkdir -p $(@D)
	embed/embed -p scr_ sh/helpers.sh > $@

README.md: $(PRE)
	./deplist.awk $^ | ./deplist.md.sh | ./deplistadd.sh $@

doc/ctpv.1: $(PRE)
	./deplist.awk $^ | ./deplist.1.sh | ./deplistadd.sh $@

embed/embed: make_embed
	@ # do nothing

-include $(DEP)

.PHONY: all options install install.bin install.man uninstall clean make_embed

.DELETE_ON_ERROR:
