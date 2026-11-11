# Writing a language

Syntax highlighting in `cedit` and `xcedit` is plain C99. Each language is
one file in `src/app/langs/`. The file describes the language and does not
contain any code. A single lexer, `src/app/syntax.c`, reads every one of
these files. No other program or file is needed to build or run them.

```
src/app/langs/cobol.c     one language: its words, comments, strings, files
src/app/langs/all.h       the list: one line, LANG(cobol), per language
src/app/lang_def.h        the macros a language is written with
src/app/syntax.h          struct syn: every field, and the flags
src/app/syntax.c          the lexer that reads them
src/app/lang.c            how a file's language is told (name, #!, text)
docs/lang_template.c      a model with every field, to copy
```

## In five steps

1. Copy the model: `cp docs/lang_template.c src/app/langs/foo.c`.
2. Fill it in, and delete the fields the language does not use.
3. Add `LANG(foo)` to `src/app/langs/all.h`, keeping the list in
   alphabetical order.
4. Run `make`.
5. Check the result with `build/cedit --lex example.foo`, or open the file
   in the editor.

A small language looks like this:

```c
/* Foo */
#include "../lang_def.h"

LANGUAGE(foo) {
	.name = "foo",
	.title = "Foo",
	.extensions = "foo",

	.keywords = WORDS("if", "then", "else", "end", "func", "return"),
	.types = WORDS("int", "string"),
	.constants = WORDS("true", "false", "nil"),

	.line_comments = { "//" },
	.quotes = "\"",

	.blocks = BLOCKS(WORD_BLOCK("func", "end"), WORD_BLOCK("if", "end")),
};
```

## The fields

All fields are optional except `.name` and `.title`.

| Field | What it holds | Example |
|---|---|---|
| `.name` | The language's name: lowercase and unique. Use `_` in place of `-` in `LANGUAGE()` and in the file name. | `"cobol"` |
| `.title` | The name shown in the status bar. | `"COBOL"` |
| `.flags` | The flags below, joined with `\|`. | `F_NOCASE \| F_DASH` |
| `.extensions` | File suffixes, separated by spaces. Matching ignores case. | `"cbl cob cpy"` |
| `.filenames` | Whole file names. | `"Makefile GNUmakefile"` |
| `.paths` | The end of a file's path. | `".git/config"` |
| `.interpreters` | Programs named on a `#!` line. A version number is ignored, so `python3.11` matches `python`. | `"python python3"` |
| `.keywords` | Keywords: `if`, `while`, `return`. | `WORDS("if", "while")` |
| `.types` | Types. | `WORDS("int", "char")` |
| `.constants` | Built-in constants. | `WORDS("true", "nil")` |
| `.functions` | Built-in functions and variables. | `WORDS("print", "len")` |
| `.preproc` | The preprocessor's words. | `WORDS("#include")` |
| `.line_comments` | Comments that run to the end of the line (up to 4). | `{ "//", "#" }` |
| `.column_comments` | A character in a given column that makes the whole line a comment (up to 3). | `{ COLUMN(7, "*/") }` |
| `.block_comments` | Comments that open and close (up to 3). | `{ { "(*", "*)" } }` |
| `.quotes` | The characters that open and close a string on one line. | `"'\""` |
| `.long_strings` | Strings that can span lines (up to 4). | `{ { "\"\"\"", "\"\"\"" } }` |
| `.blocks` | What opens and closes a block (see below). | `BLOCKS(WORD_BLOCK("if", "fi"))` |
| `.kind` | Leave this out to use the table lexer. Other values are listed under "When the table is not enough". | `K_MD` |

Word lists can be in any order. `WORDS()` adds the terminating `0`. Words
may contain characters other than letters, such as `-` in CSS or `?` in
Ruby.

### Flags

| Flag | Meaning |
|---|---|
| `F_NOCASE` | Words match in any case: `IF`, `If`, `if` (COBOL, Ada, SQL, Fortran). |
| `F_DASH` | `-` is part of a name, so `WORKING-STORAGE` and `let-values` are one word each (COBOL, the Lisps). |
| `F_HASHPRE` | A `#` at the start of a line begins a preprocessor line (C, C++). |
| `F_VARS` | `$name` and `${name}` are variables (shells, Perl, PHP). |
| `F_ATTR` | `@name` and `#[...]` get the preprocessor's colour (Java, Python, Rust). |
| `F_BACKTICK` | `` `...` `` is a string that can span lines (Go, JavaScript). |
| `F_LUALONG` | `[[ ]]` is a string and `--[[ ]]` is a comment, as in Lua. |
| `F_NOQ1` | `'` does not quote a string (Rust lifetimes). |

### Comments by column

Some languages decide that a line is a comment by its column instead of by
a marker:

- **Fortran (fixed form):** a `C`, `c` or `*` in column 1.
  Write `COLUMN(1, "Cc*")`.
- **COBOL:** a `*` or `/` in column 7. Write `COLUMN(7, "*/")`.

A letter counts only when the next character is not a letter or digit. So
`C  a comment` is a comment, but `CALL x` at the start of a line is not.

### Blocks

**Highlight Matching Brackets** shows the two ends of the block at the
cursor. **Go to Matching Bracket** (`Ctrl+]`) jumps from one end to the
other. `( )`, `[ ]` and `{ }` work in every language. Add the language's
own blocks with these macros:

```c
.blocks = BLOCKS(
	WORD_BLOCK("if", "fi"),        /* keywords: only where they are keywords */
	OP_BLOCK("begin(", ")end"),    /* operators */
	PRE_BLOCK("#if", "#endif")),   /* the preprocessor's */
```

The lexer only matches blocks in code. Blocks inside strings or comments
are ignored, and a bracket only pairs with one of its own kind.

## How a file's language is chosen

The editor tries each of these in order and stops at the first match:

1. The whole file name, from `.filenames` and `.extensions`
   (`Makefile`, or a file named `makefile`).
2. Each suffix, from `.extensions`. For `x.cmake.in` it tries `cmake.in`
   first, then `in`.
3. The end of the path, from `.paths`.
4. The `#!` line, from `.interpreters`. This handles `#!/usr/bin/env python3`.
5. The first lines of the text (`#include`, `public class`, …). This step
   is code in `src/app/lang.c`, not a field of the table.

In each step, a match with the exact case comes before a match in any
case. Only one language should claim a given extension.

## Testing

`--lex` prints each line of a file. Under it, one letter per character
shows how the line is highlighted:

```
$ build/cedit --lex hello.cbl
# hello.cbl: COBOL (cobol)
# . text  K keyword  T type, function  S string  C comment  N number, constant  P preprocessor  D deleted
000300* a comment in column 7
CCCCCCCCCCCCCCCCCCCCCCCCCCCCC
       01 WS-TOTAL PIC 9(5) COMP-3 VALUE ZERO.
.......NN..........TTT.N.N..TTTTTT.KKKKK.NNNN.
           DISPLAY 'Hello' DISPLAY-TOTAL *> inline
...........KKKKKKK.SSSSSSS...............CCCCCCCCC
```

- **Force a language:** add its name after the file:
  `build/cedit --lex file.txt cobol`.
- **Check detection:** the first line of the output shows the language the
  editor chose for the file.
- **In the editor:** the status bar shows the language. View › Syntax
  Highlighting turns the colours on and off.

Each class has its own colour in the editor's palette
(`src/platform/colours.c`): `S_KEYWORD`, `S_TYPE`, `S_STRING`,
`S_COMMENT`, `S_NUMBER` and `S_PREPROC`. On the cursor's line the colours
come from the `S_CL_*` variants.

## When the table is not enough

The table covers words, comments (including comments by column), strings,
numbers and blocks. Some formats need code of their own in
`src/app/syntax.c`. These already have it, chosen with `.kind`:

| `.kind` | Lexer |
|---|---|
| `K_DIFF` | Diff and patch files. |
| `K_MD` | Markdown. |
| `K_XML` | XML and HTML. |
| `K_KEYVAL` | `key = value` files (INI, TOML, systemd). |

A language that needs regular expressions, another language embedded in it
(JavaScript inside HTML), heredocs or interpolation inside strings also
needs a lexer in `syntax.c`: a new `K_...` value and its function. Lexers
receive one line at a time and the state left by the line before (an open
comment or string). They must not read text before that line, which keeps
large files fast.

## Checklist

- [ ] `src/app/langs/NAME.c` has `.name`, `.title` and at least one of
      `.extensions`, `.filenames` or `.interpreters`.
- [ ] `LANG(NAME)` is in `src/app/langs/all.h`, in alphabetical order.
- [ ] `make` builds with no warnings, under gcc and under clang.
- [ ] `build/cedit --lex` on a real file of the language shows what is
      expected: keywords, strings, comments, and the language on the first
      line.
- [ ] No other language already claims the same extension or file name
      (`grep -rnw EXT src/app/langs/`).
