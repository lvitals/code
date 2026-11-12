# Code Editor - a text editor in portable C99, in two faces from the same
# sources: cedit, for the terminal, and xcedit, in a window of its own
# (GLFW and OpenGL)
#
#   make            build/cedit and build/xcedit
#   make cedit      the terminal one alone (no GLFW needed)
#   make xcedit     the window one
#   make icon       data/xcedit.svg, from src/platform/icon.h (tools/icon-svg.c)
#   make enc-tables src/core/enc_data.c, the tables of the encodings, from
#                   the iconv of glibc (tools/enc-tables.c): only to make
#                   them again; the editor needs no iconv
#   make install    into $(PREFIX) (/usr/local; PREFIX=~/.local for a user),
#                   with the manual pages, the menu entry and the icon of
#                   xcedit; DESTDIR too
#   make man-lint   check the manual pages (docs/man) with mandoc
#   make check-langs  the languages listed here are those of src/app/langs
#   make rules      the rules of the objects, here, made again from the
#                   sources (tools/make-rules.c): after a .c is added
#   make uninstall
#   make clean
#
# A POSIX make (IEEE Std 1003.1): the make of Linux (GNU), of macOS, of
# FreeBSD, OpenBSD and NetBSD alike.  Any C99 compiler: make CC=clang,
# make CC=alefc ...  All that is made is in build/: the programs, and the
# objects in build/obj (src/app/main.c: build/obj/app/main.o).  A POSIX
# make cannot put an object in another folder by an inference rule, so
# each has a rule of its own, written by tools/make-rules.c (make rules).
#
# The sources:
#   src/core      the text and what is done to it (no screen)
#   src/tui       the screen buffer and the widgets (button, list, ...)
#   src/app       the editor: menus, text area, status bar, dialogs
#   src/app/langs the languages, a file each (listed in langs/all.h, and
#                 here: LANGS)
#   src/platform  the files (posix/files.c), the terminal (posix/term.c),
#                 the window (glfw/gui.c, with stb_truetype.h), the colours,
#                 the icon

.POSIX:
.SUFFIXES:

CC       = cc
CFLAGS   = -O2
# the search of the workspace's text: a thread on each core (POSIX)
LDFLAGS  = -pthread
WFLAGS   = -std=c99 -Wall -Wextra -pedantic
PREFIX   = /usr/local
BINDIR   = $(PREFIX)/bin
DATADIR  = $(PREFIX)/share
MANDIR   = $(DATADIR)/man
# the window's libraries (GLFW, OpenGL), asked when they are needed
GUI_CFLAGS = `sh tools/gui-flags --cflags`
GUI_LIBS   = `sh tools/gui-flags --libs`

# the editor, then its two faces: cedit (a terminal) and xcedit (a window)
CORE = build/obj/core/mem.o build/obj/core/buf.o build/obj/core/enc.o build/obj/core/enc_data.o \
	build/obj/core/regex.o \
	build/obj/tui/screen.o build/obj/tui/ui.o build/obj/tui/uilist.o \
	build/obj/app/main.o build/obj/app/start.o build/obj/app/editor.o build/obj/app/menu.o \
	build/obj/app/menus.o build/obj/app/status.o build/obj/app/complete.o build/obj/app/finder.o \
	build/obj/app/walk.o build/obj/app/fuzzy.o build/obj/app/scan.o build/obj/app/jumps.o \
	build/obj/app/filter.o build/obj/app/lang.o build/obj/app/search.o build/obj/app/dialogs.o \
	build/obj/app/picker.o build/obj/app/pickfile.o build/obj/app/explorer.o build/obj/app/paths.o build/obj/app/settings.o \
	build/obj/app/syntax.o build/obj/app/langs.o \
	build/obj/platform/posix/files.o build/obj/platform/posix/run.o \
	build/obj/platform/posix/thread.o build/obj/platform/colours.o
# the languages: src/app/langs/NAME.c, each in langs/all.h too (a new one:
# a line there and one here, then make rules; make check-langs tells of
# one forgotten)
LANGS = build/obj/app/langs/actionscript.o build/obj/app/langs/ada.o \
	build/obj/app/langs/alef.o build/obj/app/langs/antlr.o build/obj/app/langs/apdl.o \
	build/obj/app/langs/apl.o build/obj/app/langs/applescript.o \
	build/obj/app/langs/asm.o build/obj/app/langs/asp.o build/obj/app/langs/autohotkey.o \
	build/obj/app/langs/autoit.o build/obj/app/langs/awk.o build/obj/app/langs/bash.o \
	build/obj/app/langs/batch.o build/obj/app/langs/bibtex.o build/obj/app/langs/boo.o \
	build/obj/app/langs/c.o build/obj/app/langs/caml.o build/obj/app/langs/chuck.o \
	build/obj/app/langs/clojure.o build/obj/app/langs/cmake.o \
	build/obj/app/langs/cobol.o build/obj/app/langs/coffeescript.o \
	build/obj/app/langs/container.o build/obj/app/langs/context.o \
	build/obj/app/langs/cpp.o build/obj/app/langs/crystal.o build/obj/app/langs/csharp.o \
	build/obj/app/langs/css.o build/obj/app/langs/cuda.o build/obj/app/langs/d.o \
	build/obj/app/langs/dart.o build/obj/app/langs/desktop.o build/obj/app/langs/diff.o \
	build/obj/app/langs/django.o build/obj/app/langs/dockerfile.o \
	build/obj/app/langs/dot.o build/obj/app/langs/dsv.o build/obj/app/langs/eiffel.o \
	build/obj/app/langs/elixir.o build/obj/app/langs/elm.o build/obj/app/langs/erlang.o \
	build/obj/app/langs/factor.o build/obj/app/langs/fantom.o \
	build/obj/app/langs/faust.o build/obj/app/langs/fennel.o build/obj/app/langs/fish.o \
	build/obj/app/langs/forth.o build/obj/app/langs/fortran.o \
	build/obj/app/langs/fsharp.o build/obj/app/langs/fstab.o build/obj/app/langs/gap.o \
	build/obj/app/langs/gemini.o build/obj/app/langs/gettext.o \
	build/obj/app/langs/gherkin.o build/obj/app/langs/git_rebase.o \
	build/obj/app/langs/gleam.o build/obj/app/langs/glsl.o build/obj/app/langs/gnuplot.o \
	build/obj/app/langs/go.o build/obj/app/langs/groovy.o build/obj/app/langs/gtkrc.o \
	build/obj/app/langs/hare.o build/obj/app/langs/haskell.o build/obj/app/langs/html.o \
	build/obj/app/langs/icon.o build/obj/app/langs/idl.o build/obj/app/langs/inform.o \
	build/obj/app/langs/ini.o build/obj/app/langs/io_lang.o build/obj/app/langs/janet.o \
	build/obj/app/langs/java.o build/obj/app/langs/javascript.o build/obj/app/langs/jq.o \
	build/obj/app/langs/json.o build/obj/app/langs/jsp.o build/obj/app/langs/julia.o \
	build/obj/app/langs/latex.o build/obj/app/langs/ledger.o build/obj/app/langs/less.o \
	build/obj/app/langs/lilypond.o build/obj/app/langs/lisp.o \
	build/obj/app/langs/litcoffee.o build/obj/app/langs/logtalk.o \
	build/obj/app/langs/lua.o build/obj/app/langs/makefile.o build/obj/app/langs/man.o \
	build/obj/app/langs/markdown.o build/obj/app/langs/matlab.o \
	build/obj/app/langs/mediawiki.o build/obj/app/langs/meson.o \
	build/obj/app/langs/moonscript.o build/obj/app/langs/myrddin.o \
	build/obj/app/langs/nemerle.o build/obj/app/langs/networkd.o \
	build/obj/app/langs/nim.o build/obj/app/langs/nix.o build/obj/app/langs/nsis.o \
	build/obj/app/langs/objeck.o build/obj/app/langs/objective_c.o \
	build/obj/app/langs/org.o build/obj/app/langs/output.o build/obj/app/langs/pascal.o \
	build/obj/app/langs/perl.o build/obj/app/langs/php.o build/obj/app/langs/pico8.o \
	build/obj/app/langs/pike.o build/obj/app/langs/pkgbuild.o build/obj/app/langs/pony.o \
	build/obj/app/langs/powershell.o build/obj/app/langs/prolog.o \
	build/obj/app/langs/props.o build/obj/app/langs/protobuf.o build/obj/app/langs/ps.o \
	build/obj/app/langs/pure.o build/obj/app/langs/python.o build/obj/app/langs/r.o \
	build/obj/app/langs/rails.o build/obj/app/langs/rc.o build/obj/app/langs/reason.o \
	build/obj/app/langs/rebol.o build/obj/app/langs/rest.o build/obj/app/langs/rexx.o \
	build/obj/app/langs/rhtml.o build/obj/app/langs/routeros.o \
	build/obj/app/langs/rpmspec.o build/obj/app/langs/ruby.o build/obj/app/langs/rust.o \
	build/obj/app/langs/sass.o build/obj/app/langs/scala.o build/obj/app/langs/scheme.o \
	build/obj/app/langs/smalltalk.o build/obj/app/langs/sml.o \
	build/obj/app/langs/snobol4.o build/obj/app/langs/spin.o build/obj/app/langs/sql.o \
	build/obj/app/langs/strace.o build/obj/app/langs/systemd.o \
	build/obj/app/langs/taskpaper.o build/obj/app/langs/tcl.o build/obj/app/langs/tex.o \
	build/obj/app/langs/texinfo.o build/obj/app/langs/todotxt.o \
	build/obj/app/langs/toml.o build/obj/app/langs/troff.o \
	build/obj/app/langs/txt2tags.o build/obj/app/langs/typescript.o \
	build/obj/app/langs/vala.o build/obj/app/langs/vb.o build/obj/app/langs/vcard.o \
	build/obj/app/langs/verilog.o build/obj/app/langs/vhdl.o build/obj/app/langs/wsf.o \
	build/obj/app/langs/xml.o build/obj/app/langs/xs.o build/obj/app/langs/xtend.o \
	build/obj/app/langs/yaml.o build/obj/app/langs/zig.o
TERM   = build/obj/platform/posix/term.o
WINDOW = build/obj/platform/glfw/gui.o
HDRS = src/app/app.h src/app/finder.h src/app/lang_def.h src/app/langs/all.h \
	src/app/syntax.h src/core/buf.h src/core/enc.h src/core/enc_data.h src/core/mem.h \
	src/core/regex.h src/platform/colours.h src/platform/icon.h \
	src/platform/platform.h src/tui/screen.h src/tui/ui.h

all: build/cedit build/xcedit

cedit: build/cedit
xcedit: build/xcedit

# an object: build/obj/X.o from src/X.c, its folder made first
CCO = $(CC) $(WFLAGS) $(CFLAGS) -c -o $@
OBJDIRS = build/obj/core build/obj/tui build/obj/app/langs \
	build/obj/platform/posix build/obj/platform/glfw

$(CORE) $(LANGS) $(TERM) $(WINDOW): $(HDRS) build/obj/.dirs
build/obj/platform/glfw/stb.o: build/obj/.dirs

build/obj/.dirs:
	mkdir -p $(OBJDIRS)
	touch $@

rules: build/make-rules
	build/make-rules Makefile src

build/make-rules: tools/make-rules.c
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/make-rules.c

# --- the objects: made by tools/make-rules (make rules) ---
build/obj/app/complete.o: src/app/complete.c
	$(CCO) src/app/complete.c
build/obj/app/dialogs.o: src/app/dialogs.c
	$(CCO) src/app/dialogs.c
build/obj/app/editor.o: src/app/editor.c
	$(CCO) src/app/editor.c
build/obj/app/explorer.o: src/app/explorer.c
	$(CCO) src/app/explorer.c
build/obj/app/filter.o: src/app/filter.c
	$(CCO) src/app/filter.c
build/obj/app/finder.o: src/app/finder.c
	$(CCO) src/app/finder.c
build/obj/app/fuzzy.o: src/app/fuzzy.c
	$(CCO) src/app/fuzzy.c
build/obj/app/jumps.o: src/app/jumps.c
	$(CCO) src/app/jumps.c
build/obj/app/lang.o: src/app/lang.c
	$(CCO) src/app/lang.c
build/obj/app/langs.o: src/app/langs.c
	$(CCO) src/app/langs.c
build/obj/app/langs/actionscript.o: src/app/langs/actionscript.c
	$(CCO) src/app/langs/actionscript.c
build/obj/app/langs/ada.o: src/app/langs/ada.c
	$(CCO) src/app/langs/ada.c
build/obj/app/langs/alef.o: src/app/langs/alef.c
	$(CCO) src/app/langs/alef.c
build/obj/app/langs/antlr.o: src/app/langs/antlr.c
	$(CCO) src/app/langs/antlr.c
build/obj/app/langs/apdl.o: src/app/langs/apdl.c
	$(CCO) src/app/langs/apdl.c
build/obj/app/langs/apl.o: src/app/langs/apl.c
	$(CCO) src/app/langs/apl.c
build/obj/app/langs/applescript.o: src/app/langs/applescript.c
	$(CCO) src/app/langs/applescript.c
build/obj/app/langs/asm.o: src/app/langs/asm.c
	$(CCO) src/app/langs/asm.c
build/obj/app/langs/asp.o: src/app/langs/asp.c
	$(CCO) src/app/langs/asp.c
build/obj/app/langs/autohotkey.o: src/app/langs/autohotkey.c
	$(CCO) src/app/langs/autohotkey.c
build/obj/app/langs/autoit.o: src/app/langs/autoit.c
	$(CCO) src/app/langs/autoit.c
build/obj/app/langs/awk.o: src/app/langs/awk.c
	$(CCO) src/app/langs/awk.c
build/obj/app/langs/bash.o: src/app/langs/bash.c
	$(CCO) src/app/langs/bash.c
build/obj/app/langs/batch.o: src/app/langs/batch.c
	$(CCO) src/app/langs/batch.c
build/obj/app/langs/bibtex.o: src/app/langs/bibtex.c
	$(CCO) src/app/langs/bibtex.c
build/obj/app/langs/boo.o: src/app/langs/boo.c
	$(CCO) src/app/langs/boo.c
build/obj/app/langs/c.o: src/app/langs/c.c
	$(CCO) src/app/langs/c.c
build/obj/app/langs/caml.o: src/app/langs/caml.c
	$(CCO) src/app/langs/caml.c
build/obj/app/langs/chuck.o: src/app/langs/chuck.c
	$(CCO) src/app/langs/chuck.c
build/obj/app/langs/clojure.o: src/app/langs/clojure.c
	$(CCO) src/app/langs/clojure.c
build/obj/app/langs/cmake.o: src/app/langs/cmake.c
	$(CCO) src/app/langs/cmake.c
build/obj/app/langs/cobol.o: src/app/langs/cobol.c
	$(CCO) src/app/langs/cobol.c
build/obj/app/langs/coffeescript.o: src/app/langs/coffeescript.c
	$(CCO) src/app/langs/coffeescript.c
build/obj/app/langs/container.o: src/app/langs/container.c
	$(CCO) src/app/langs/container.c
build/obj/app/langs/context.o: src/app/langs/context.c
	$(CCO) src/app/langs/context.c
build/obj/app/langs/cpp.o: src/app/langs/cpp.c
	$(CCO) src/app/langs/cpp.c
build/obj/app/langs/crystal.o: src/app/langs/crystal.c
	$(CCO) src/app/langs/crystal.c
build/obj/app/langs/csharp.o: src/app/langs/csharp.c
	$(CCO) src/app/langs/csharp.c
build/obj/app/langs/css.o: src/app/langs/css.c
	$(CCO) src/app/langs/css.c
build/obj/app/langs/cuda.o: src/app/langs/cuda.c
	$(CCO) src/app/langs/cuda.c
build/obj/app/langs/d.o: src/app/langs/d.c
	$(CCO) src/app/langs/d.c
build/obj/app/langs/dart.o: src/app/langs/dart.c
	$(CCO) src/app/langs/dart.c
build/obj/app/langs/desktop.o: src/app/langs/desktop.c
	$(CCO) src/app/langs/desktop.c
build/obj/app/langs/diff.o: src/app/langs/diff.c
	$(CCO) src/app/langs/diff.c
build/obj/app/langs/django.o: src/app/langs/django.c
	$(CCO) src/app/langs/django.c
build/obj/app/langs/dockerfile.o: src/app/langs/dockerfile.c
	$(CCO) src/app/langs/dockerfile.c
build/obj/app/langs/dot.o: src/app/langs/dot.c
	$(CCO) src/app/langs/dot.c
build/obj/app/langs/dsv.o: src/app/langs/dsv.c
	$(CCO) src/app/langs/dsv.c
build/obj/app/langs/eiffel.o: src/app/langs/eiffel.c
	$(CCO) src/app/langs/eiffel.c
build/obj/app/langs/elixir.o: src/app/langs/elixir.c
	$(CCO) src/app/langs/elixir.c
build/obj/app/langs/elm.o: src/app/langs/elm.c
	$(CCO) src/app/langs/elm.c
build/obj/app/langs/erlang.o: src/app/langs/erlang.c
	$(CCO) src/app/langs/erlang.c
build/obj/app/langs/factor.o: src/app/langs/factor.c
	$(CCO) src/app/langs/factor.c
build/obj/app/langs/fantom.o: src/app/langs/fantom.c
	$(CCO) src/app/langs/fantom.c
build/obj/app/langs/faust.o: src/app/langs/faust.c
	$(CCO) src/app/langs/faust.c
build/obj/app/langs/fennel.o: src/app/langs/fennel.c
	$(CCO) src/app/langs/fennel.c
build/obj/app/langs/fish.o: src/app/langs/fish.c
	$(CCO) src/app/langs/fish.c
build/obj/app/langs/forth.o: src/app/langs/forth.c
	$(CCO) src/app/langs/forth.c
build/obj/app/langs/fortran.o: src/app/langs/fortran.c
	$(CCO) src/app/langs/fortran.c
build/obj/app/langs/fsharp.o: src/app/langs/fsharp.c
	$(CCO) src/app/langs/fsharp.c
build/obj/app/langs/fstab.o: src/app/langs/fstab.c
	$(CCO) src/app/langs/fstab.c
build/obj/app/langs/gap.o: src/app/langs/gap.c
	$(CCO) src/app/langs/gap.c
build/obj/app/langs/gemini.o: src/app/langs/gemini.c
	$(CCO) src/app/langs/gemini.c
build/obj/app/langs/gettext.o: src/app/langs/gettext.c
	$(CCO) src/app/langs/gettext.c
build/obj/app/langs/gherkin.o: src/app/langs/gherkin.c
	$(CCO) src/app/langs/gherkin.c
build/obj/app/langs/git_rebase.o: src/app/langs/git_rebase.c
	$(CCO) src/app/langs/git_rebase.c
build/obj/app/langs/gleam.o: src/app/langs/gleam.c
	$(CCO) src/app/langs/gleam.c
build/obj/app/langs/glsl.o: src/app/langs/glsl.c
	$(CCO) src/app/langs/glsl.c
build/obj/app/langs/gnuplot.o: src/app/langs/gnuplot.c
	$(CCO) src/app/langs/gnuplot.c
build/obj/app/langs/go.o: src/app/langs/go.c
	$(CCO) src/app/langs/go.c
build/obj/app/langs/groovy.o: src/app/langs/groovy.c
	$(CCO) src/app/langs/groovy.c
build/obj/app/langs/gtkrc.o: src/app/langs/gtkrc.c
	$(CCO) src/app/langs/gtkrc.c
build/obj/app/langs/hare.o: src/app/langs/hare.c
	$(CCO) src/app/langs/hare.c
build/obj/app/langs/haskell.o: src/app/langs/haskell.c
	$(CCO) src/app/langs/haskell.c
build/obj/app/langs/html.o: src/app/langs/html.c
	$(CCO) src/app/langs/html.c
build/obj/app/langs/icon.o: src/app/langs/icon.c
	$(CCO) src/app/langs/icon.c
build/obj/app/langs/idl.o: src/app/langs/idl.c
	$(CCO) src/app/langs/idl.c
build/obj/app/langs/inform.o: src/app/langs/inform.c
	$(CCO) src/app/langs/inform.c
build/obj/app/langs/ini.o: src/app/langs/ini.c
	$(CCO) src/app/langs/ini.c
build/obj/app/langs/io_lang.o: src/app/langs/io_lang.c
	$(CCO) src/app/langs/io_lang.c
build/obj/app/langs/janet.o: src/app/langs/janet.c
	$(CCO) src/app/langs/janet.c
build/obj/app/langs/java.o: src/app/langs/java.c
	$(CCO) src/app/langs/java.c
build/obj/app/langs/javascript.o: src/app/langs/javascript.c
	$(CCO) src/app/langs/javascript.c
build/obj/app/langs/jq.o: src/app/langs/jq.c
	$(CCO) src/app/langs/jq.c
build/obj/app/langs/json.o: src/app/langs/json.c
	$(CCO) src/app/langs/json.c
build/obj/app/langs/jsp.o: src/app/langs/jsp.c
	$(CCO) src/app/langs/jsp.c
build/obj/app/langs/julia.o: src/app/langs/julia.c
	$(CCO) src/app/langs/julia.c
build/obj/app/langs/latex.o: src/app/langs/latex.c
	$(CCO) src/app/langs/latex.c
build/obj/app/langs/ledger.o: src/app/langs/ledger.c
	$(CCO) src/app/langs/ledger.c
build/obj/app/langs/less.o: src/app/langs/less.c
	$(CCO) src/app/langs/less.c
build/obj/app/langs/lilypond.o: src/app/langs/lilypond.c
	$(CCO) src/app/langs/lilypond.c
build/obj/app/langs/lisp.o: src/app/langs/lisp.c
	$(CCO) src/app/langs/lisp.c
build/obj/app/langs/litcoffee.o: src/app/langs/litcoffee.c
	$(CCO) src/app/langs/litcoffee.c
build/obj/app/langs/logtalk.o: src/app/langs/logtalk.c
	$(CCO) src/app/langs/logtalk.c
build/obj/app/langs/lua.o: src/app/langs/lua.c
	$(CCO) src/app/langs/lua.c
build/obj/app/langs/makefile.o: src/app/langs/makefile.c
	$(CCO) src/app/langs/makefile.c
build/obj/app/langs/man.o: src/app/langs/man.c
	$(CCO) src/app/langs/man.c
build/obj/app/langs/markdown.o: src/app/langs/markdown.c
	$(CCO) src/app/langs/markdown.c
build/obj/app/langs/matlab.o: src/app/langs/matlab.c
	$(CCO) src/app/langs/matlab.c
build/obj/app/langs/mediawiki.o: src/app/langs/mediawiki.c
	$(CCO) src/app/langs/mediawiki.c
build/obj/app/langs/meson.o: src/app/langs/meson.c
	$(CCO) src/app/langs/meson.c
build/obj/app/langs/moonscript.o: src/app/langs/moonscript.c
	$(CCO) src/app/langs/moonscript.c
build/obj/app/langs/myrddin.o: src/app/langs/myrddin.c
	$(CCO) src/app/langs/myrddin.c
build/obj/app/langs/nemerle.o: src/app/langs/nemerle.c
	$(CCO) src/app/langs/nemerle.c
build/obj/app/langs/networkd.o: src/app/langs/networkd.c
	$(CCO) src/app/langs/networkd.c
build/obj/app/langs/nim.o: src/app/langs/nim.c
	$(CCO) src/app/langs/nim.c
build/obj/app/langs/nix.o: src/app/langs/nix.c
	$(CCO) src/app/langs/nix.c
build/obj/app/langs/nsis.o: src/app/langs/nsis.c
	$(CCO) src/app/langs/nsis.c
build/obj/app/langs/objeck.o: src/app/langs/objeck.c
	$(CCO) src/app/langs/objeck.c
build/obj/app/langs/objective_c.o: src/app/langs/objective_c.c
	$(CCO) src/app/langs/objective_c.c
build/obj/app/langs/org.o: src/app/langs/org.c
	$(CCO) src/app/langs/org.c
build/obj/app/langs/output.o: src/app/langs/output.c
	$(CCO) src/app/langs/output.c
build/obj/app/langs/pascal.o: src/app/langs/pascal.c
	$(CCO) src/app/langs/pascal.c
build/obj/app/langs/perl.o: src/app/langs/perl.c
	$(CCO) src/app/langs/perl.c
build/obj/app/langs/php.o: src/app/langs/php.c
	$(CCO) src/app/langs/php.c
build/obj/app/langs/pico8.o: src/app/langs/pico8.c
	$(CCO) src/app/langs/pico8.c
build/obj/app/langs/pike.o: src/app/langs/pike.c
	$(CCO) src/app/langs/pike.c
build/obj/app/langs/pkgbuild.o: src/app/langs/pkgbuild.c
	$(CCO) src/app/langs/pkgbuild.c
build/obj/app/langs/pony.o: src/app/langs/pony.c
	$(CCO) src/app/langs/pony.c
build/obj/app/langs/powershell.o: src/app/langs/powershell.c
	$(CCO) src/app/langs/powershell.c
build/obj/app/langs/prolog.o: src/app/langs/prolog.c
	$(CCO) src/app/langs/prolog.c
build/obj/app/langs/props.o: src/app/langs/props.c
	$(CCO) src/app/langs/props.c
build/obj/app/langs/protobuf.o: src/app/langs/protobuf.c
	$(CCO) src/app/langs/protobuf.c
build/obj/app/langs/ps.o: src/app/langs/ps.c
	$(CCO) src/app/langs/ps.c
build/obj/app/langs/pure.o: src/app/langs/pure.c
	$(CCO) src/app/langs/pure.c
build/obj/app/langs/python.o: src/app/langs/python.c
	$(CCO) src/app/langs/python.c
build/obj/app/langs/r.o: src/app/langs/r.c
	$(CCO) src/app/langs/r.c
build/obj/app/langs/rails.o: src/app/langs/rails.c
	$(CCO) src/app/langs/rails.c
build/obj/app/langs/rc.o: src/app/langs/rc.c
	$(CCO) src/app/langs/rc.c
build/obj/app/langs/reason.o: src/app/langs/reason.c
	$(CCO) src/app/langs/reason.c
build/obj/app/langs/rebol.o: src/app/langs/rebol.c
	$(CCO) src/app/langs/rebol.c
build/obj/app/langs/rest.o: src/app/langs/rest.c
	$(CCO) src/app/langs/rest.c
build/obj/app/langs/rexx.o: src/app/langs/rexx.c
	$(CCO) src/app/langs/rexx.c
build/obj/app/langs/rhtml.o: src/app/langs/rhtml.c
	$(CCO) src/app/langs/rhtml.c
build/obj/app/langs/routeros.o: src/app/langs/routeros.c
	$(CCO) src/app/langs/routeros.c
build/obj/app/langs/rpmspec.o: src/app/langs/rpmspec.c
	$(CCO) src/app/langs/rpmspec.c
build/obj/app/langs/ruby.o: src/app/langs/ruby.c
	$(CCO) src/app/langs/ruby.c
build/obj/app/langs/rust.o: src/app/langs/rust.c
	$(CCO) src/app/langs/rust.c
build/obj/app/langs/sass.o: src/app/langs/sass.c
	$(CCO) src/app/langs/sass.c
build/obj/app/langs/scala.o: src/app/langs/scala.c
	$(CCO) src/app/langs/scala.c
build/obj/app/langs/scheme.o: src/app/langs/scheme.c
	$(CCO) src/app/langs/scheme.c
build/obj/app/langs/smalltalk.o: src/app/langs/smalltalk.c
	$(CCO) src/app/langs/smalltalk.c
build/obj/app/langs/sml.o: src/app/langs/sml.c
	$(CCO) src/app/langs/sml.c
build/obj/app/langs/snobol4.o: src/app/langs/snobol4.c
	$(CCO) src/app/langs/snobol4.c
build/obj/app/langs/spin.o: src/app/langs/spin.c
	$(CCO) src/app/langs/spin.c
build/obj/app/langs/sql.o: src/app/langs/sql.c
	$(CCO) src/app/langs/sql.c
build/obj/app/langs/strace.o: src/app/langs/strace.c
	$(CCO) src/app/langs/strace.c
build/obj/app/langs/systemd.o: src/app/langs/systemd.c
	$(CCO) src/app/langs/systemd.c
build/obj/app/langs/taskpaper.o: src/app/langs/taskpaper.c
	$(CCO) src/app/langs/taskpaper.c
build/obj/app/langs/tcl.o: src/app/langs/tcl.c
	$(CCO) src/app/langs/tcl.c
build/obj/app/langs/tex.o: src/app/langs/tex.c
	$(CCO) src/app/langs/tex.c
build/obj/app/langs/texinfo.o: src/app/langs/texinfo.c
	$(CCO) src/app/langs/texinfo.c
build/obj/app/langs/todotxt.o: src/app/langs/todotxt.c
	$(CCO) src/app/langs/todotxt.c
build/obj/app/langs/toml.o: src/app/langs/toml.c
	$(CCO) src/app/langs/toml.c
build/obj/app/langs/troff.o: src/app/langs/troff.c
	$(CCO) src/app/langs/troff.c
build/obj/app/langs/txt2tags.o: src/app/langs/txt2tags.c
	$(CCO) src/app/langs/txt2tags.c
build/obj/app/langs/typescript.o: src/app/langs/typescript.c
	$(CCO) src/app/langs/typescript.c
build/obj/app/langs/vala.o: src/app/langs/vala.c
	$(CCO) src/app/langs/vala.c
build/obj/app/langs/vb.o: src/app/langs/vb.c
	$(CCO) src/app/langs/vb.c
build/obj/app/langs/vcard.o: src/app/langs/vcard.c
	$(CCO) src/app/langs/vcard.c
build/obj/app/langs/verilog.o: src/app/langs/verilog.c
	$(CCO) src/app/langs/verilog.c
build/obj/app/langs/vhdl.o: src/app/langs/vhdl.c
	$(CCO) src/app/langs/vhdl.c
build/obj/app/langs/wsf.o: src/app/langs/wsf.c
	$(CCO) src/app/langs/wsf.c
build/obj/app/langs/xml.o: src/app/langs/xml.c
	$(CCO) src/app/langs/xml.c
build/obj/app/langs/xs.o: src/app/langs/xs.c
	$(CCO) src/app/langs/xs.c
build/obj/app/langs/xtend.o: src/app/langs/xtend.c
	$(CCO) src/app/langs/xtend.c
build/obj/app/langs/yaml.o: src/app/langs/yaml.c
	$(CCO) src/app/langs/yaml.c
build/obj/app/langs/zig.o: src/app/langs/zig.c
	$(CCO) src/app/langs/zig.c
build/obj/app/main.o: src/app/main.c
	$(CCO) src/app/main.c
build/obj/app/menu.o: src/app/menu.c
	$(CCO) src/app/menu.c
build/obj/app/menus.o: src/app/menus.c
	$(CCO) src/app/menus.c
build/obj/app/paths.o: src/app/paths.c
	$(CCO) src/app/paths.c
build/obj/app/picker.o: src/app/picker.c
	$(CCO) src/app/picker.c
build/obj/app/pickfile.o: src/app/pickfile.c
	$(CCO) src/app/pickfile.c
build/obj/app/scan.o: src/app/scan.c
	$(CCO) src/app/scan.c
build/obj/app/search.o: src/app/search.c
	$(CCO) src/app/search.c
build/obj/app/settings.o: src/app/settings.c
	$(CCO) src/app/settings.c
build/obj/app/start.o: src/app/start.c
	$(CCO) src/app/start.c
build/obj/app/status.o: src/app/status.c
	$(CCO) src/app/status.c
build/obj/app/syntax.o: src/app/syntax.c
	$(CCO) src/app/syntax.c
build/obj/app/walk.o: src/app/walk.c
	$(CCO) src/app/walk.c
build/obj/core/buf.o: src/core/buf.c
	$(CCO) src/core/buf.c
build/obj/core/enc.o: src/core/enc.c
	$(CCO) src/core/enc.c
build/obj/core/enc_data.o: src/core/enc_data.c
	$(CCO) src/core/enc_data.c
build/obj/core/mem.o: src/core/mem.c
	$(CCO) src/core/mem.c
build/obj/core/regex.o: src/core/regex.c
	$(CCO) src/core/regex.c
build/obj/platform/colours.o: src/platform/colours.c
	$(CCO) src/platform/colours.c
build/obj/platform/posix/files.o: src/platform/posix/files.c
	$(CCO) src/platform/posix/files.c
build/obj/platform/posix/run.o: src/platform/posix/run.c
	$(CCO) src/platform/posix/run.c
build/obj/platform/posix/term.o: src/platform/posix/term.c
	$(CCO) src/platform/posix/term.c
build/obj/platform/posix/thread.o: src/platform/posix/thread.c
	$(CCO) src/platform/posix/thread.c
build/obj/tui/screen.o: src/tui/screen.c
	$(CCO) src/tui/screen.c
build/obj/tui/ui.o: src/tui/ui.c
	$(CCO) src/tui/ui.c
build/obj/tui/uilist.o: src/tui/uilist.c
	$(CCO) src/tui/uilist.c
# --- the end of the objects ---

build/cedit: $(CORE) $(LANGS) $(TERM)
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CORE) $(LANGS) $(TERM)

# the window: its file with the flags of GLFW; stb_truetype's code (not
# ours: no warnings); the font, a C array made by tools/bin2c
build/obj/platform/glfw/gui.o: src/platform/glfw/gui.c src/platform/glfw/stb_truetype.h
	$(CC) $(WFLAGS) $(CFLAGS) $(GUI_CFLAGS) -c -o $@ src/platform/glfw/gui.c

build/obj/platform/glfw/stb.o: src/platform/glfw/stb.c src/platform/glfw/stb_truetype.h
	$(CC) -std=c99 -O2 -w -c -o $@ src/platform/glfw/stb.c

build/bin2c: tools/bin2c.c
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/bin2c.c

build/font.c: build/bin2c data/fonts/FantasqueSansMono-Regular.ttf
	build/bin2c font_regular data/fonts/FantasqueSansMono-Regular.ttf > $@

build/font.o: build/font.c
	$(CC) -std=c99 -O2 -c -o $@ build/font.c

build/xcedit: $(CORE) $(LANGS) $(WINDOW) build/obj/platform/glfw/stb.o build/font.o
	mkdir -p build
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(CORE) $(LANGS) $(WINDOW) \
	    build/obj/platform/glfw/stb.o build/font.o $(GUI_LIBS)

# the icon, from its data (as tools/mascot-svg.c of inkaur): in the
# sources, made again when its data or its tool change
icon: data/xcedit.svg

# the tables of the encodings beyond the editor's own, in the sources:
# made again from the iconv of glibc (only this needs it; the editor not)
enc-tables: build/enc-tables
	build/enc-tables > src/core/enc_data.c

build/enc-tables: tools/enc-tables.c src/core/enc_data.h
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/enc-tables.c

build/icon-svg: tools/icon-svg.c src/platform/icon.h
	mkdir -p build
	$(CC) $(WFLAGS) $(CFLAGS) -o $@ tools/icon-svg.c

data/xcedit.svg: build/icon-svg
	build/icon-svg > $@.tmp && mv $@.tmp $@

# each language of src/app/langs is in LANGS, and each of LANGS is there
check-langs:
	@ok=1; \
	for f in src/app/langs/*.c; do \
		o="build/obj/$${f#src/}"; o="$${o%.c}.o"; \
		case " `echo $(LANGS)` " in *" $$o "*) ;; *) echo "not in LANGS: $$o"; ok=0;; esac; \
	done; \
	for o in $(LANGS); do \
		c="src/$${o#build/obj/}"; c="$${c%.o}.c"; \
		[ -f "$$c" ] || { echo "no source: $$c"; ok=0; }; \
	done; \
	[ $$ok = 1 ] && echo "LANGS: as src/app/langs"

# the programs; for xcedit its entry in the menus of the system and its
# icon (the caches of the desktop brought up to date, when there)
install: all data/xcedit.svg
	mkdir -p $(DESTDIR)$(BINDIR) $(DESTDIR)$(DATADIR)/applications \
	    $(DESTDIR)$(MANDIR)/man1 $(DESTDIR)$(MANDIR)/pt_BR/man1 \
	    $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps $(DESTDIR)$(DATADIR)/licenses/cedit
	install -m 755 build/cedit build/xcedit $(DESTDIR)$(BINDIR)/
	install -m 644 docs/man/cedit.1 $(DESTDIR)$(MANDIR)/man1/
	ln -sf cedit.1 $(DESTDIR)$(MANDIR)/man1/xcedit.1
	install -m 644 docs/man/pt_BR/cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/
	ln -sf cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/xcedit.1
	install -m 644 data/xcedit.desktop $(DESTDIR)$(DATADIR)/applications/
	install -m 644 data/xcedit.svg $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/
	install -m 644 LICENSE $(DESTDIR)$(DATADIR)/licenses/cedit/
	-[ -n "$(DESTDIR)" ] || update-desktop-database -q $(DATADIR)/applications 2>/dev/null
	-[ -n "$(DESTDIR)" ] || gtk-update-icon-cache -q -t $(DATADIR)/icons/hicolor 2>/dev/null
	-[ -n "$(DESTDIR)" ] || makewhatis $(MANDIR) 2>/dev/null || mandb -q 2>/dev/null

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/cedit $(DESTDIR)$(BINDIR)/xcedit \
	    $(DESTDIR)$(DATADIR)/applications/xcedit.desktop \
	    $(DESTDIR)$(DATADIR)/icons/hicolor/scalable/apps/xcedit.svg \
	    $(DESTDIR)$(MANDIR)/man1/cedit.1 $(DESTDIR)$(MANDIR)/man1/xcedit.1 \
	    $(DESTDIR)$(MANDIR)/pt_BR/man1/cedit.1 $(DESTDIR)$(MANDIR)/pt_BR/man1/xcedit.1
	rm -rf $(DESTDIR)$(DATADIR)/licenses/cedit
	-[ -n "$(DESTDIR)" ] || update-desktop-database -q $(DATADIR)/applications 2>/dev/null
	-[ -n "$(DESTDIR)" ] || makewhatis $(MANDIR) 2>/dev/null || mandb -q 2>/dev/null

# the manual pages, as mandoc reads them (its warnings and errors)
man-lint:
	mandoc -Tlint -Wwarning docs/man/cedit.1 docs/man/pt_BR/cedit.1

clean:
	rm -rf build

.PHONY: all cedit xcedit icon enc-tables rules install uninstall man-lint check-langs clean
