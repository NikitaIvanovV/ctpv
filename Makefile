PREFIX    := /usr/local
BINPREFIX := $(DESTDIR)$(PREFIX)/bin
MANPREFIX := $(DESTDIR)$(PREFIX)/share/man/man1

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)
DEP := $(OBJ:.o=.d)
PRE := $(wildcard sh/prev/*)
GEN := gen/previews.h gen/server.h gen/helpers.h

O    := -O2
LIBS := magic crypto

CFLAGS  += $(O) -MD -Wall -Wextra -Wno-unused-parameter
LDFLAGS += $(addprefix -l,$(LIBS))

INSTALL := install

all: ctpv

options:
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"

install: install.bin install.man

install.bin: ctpv quit/ctpvquit ctpvclear
	$(INSTALL) -d $(BINPREFIX)
	$(INSTALL) $^ $(BINPREFIX)

install.man: doc/ctpv.1
	$(INSTALL) -d $(MANPREFIX)
	$(INSTALL) -m 0644 $^ $(MANPREFIX)

uninstall:
	$(RM) $(BINPREFIX)/ctpv $(BINPREFIX)/ctpvclear $(BINPREFIX)/ctpvquit \
		$(MANPREFIX)/ctpv.1

clean:
	$(RM) ctpv $(OBJ) $(DEP) $(GEN)
	$(MAKE) -C embed clean
	$(MAKE) -C quit clean

docs: README.md doc/ctpv.1
	deptable/list.awk $(PRE) | deptable/markdown.sed | deptable/insert.awk README.md
	deptable/list.awk $(PRE) | deptable/roff.sed | deptable/insert.awk doc/ctpv.1

ctpv: $(OBJ)
	$(CC) -o $@ $+ $(LDFLAGS)

# Exclicit rules for generated header files
src/ctpv.o: gen/previews.h
src/shell.o: gen/helpers.h
src/server.o: gen/server.h

gen/previews.h: $(PRE) embed/embed
	embed/embed -p prev_scr_ $(PRE) > $@

gen/server.h: sh/clear.sh sh/end.sh embed/embed
	embed/embed -p scr_ sh/clear.sh sh/end.sh > $@

gen/helpers.h: sh/helpers.sh embed/embed
	embed/embed -p scr_ sh/helpers.sh > $@

$(GEN): | gen

gen:
	mkdir $@

embed/embed: .force
	$(MAKE) -C embed

quit/ctpvquit: .force
	$(MAKE) -C quit

-include $(DEP)

.PHONY: all options install install.bin install.man uninstall \
	clean docs .force

.DELETE_ON_ERROR:
