/* Shell Script */
#include "../lang_def.h"

LANGUAGE(bash) {
	.name = "bash",
	.title = "Shell Script",
	.flags = F_VARS,
	.extensions = "ash ebuild yash bash bashrc bash_profile csh ksh mksh sh zsh",
	.filenames = ".bash_logout .bash_profile .bashrc .login .mkshrc .profile .sh.profile .shinit .xprofile .yash_profile .yashrc APKBUILD bash.bash.logout bash.bashrc mkshrc profile configure .zshrc zshrc",
	.interpreters = "sh ash dash bash ksh mksh csh tcsh zsh",

	.keywords = WORDS(
		"case", "coproc", "do", "done", "elif", "else", "esac", "fi", "for",
		"function", "if", "in", "select", "then", "time", "until", "while"),

	.functions = WORDS(
		"BASH", "BASHOPTS", "BASHPID", "BASH_ALIASES", "BASH_ARGC", "BASH_ARGV",
		"BASH_ARGV0", "BASH_CMDS", "BASH_COMMAND", "BASH_COMPAT", "BASH_ENV",
		"BASH_EXECUTION_STRING", "BASH_LINENO", "BASH_LOADABLES_PATH",
		"BASH_REMATCH", "BASH_SOURCE", "BASH_SUBSHELL", "BASH_VERSINFO",
		"BASH_VERSION", "BASH_XTRACEFD", "CDPATH", "CHILD_MAX", "COLUMNS",
		"COMP_CWORD", "COMP_KEY", "COMP_LINE", "COMP_POINT", "COMP_REPLY",
		"COMP_TYPE", "COMP_WORDBREAKS", "COMP_WORDS", "COPROC", "DIRSTACK",
		"EMACS", "ENV", "EPOCHREALTIME", "EPOCHSECONDS", "EUID", "EXECIGNORE",
		"FCEDIT", "FIGNORE", "FUNCNAME", "FUNCNEST", "GLOBIGNORE", "GROUPS",
		"HISTCMD", "HISTCONTROL", "HISTFILE", "HISTFILESIZE", "HISTIGNORE",
		"HISTSIZE", "HISTTIMEFORMAT", "HOME", "HOSTFILE", "HOSTNAME",
		"HOSTTYPE", "IFS", "IGNOREEOF", "INPUTRC", "INSIDE_EMACS", "LANG",
		"LC_ALL", "LC_COLLATE", "LC_CTYPE", "LC_MESSAGES", "LC_NUMERIC",
		"LC_TIME", "LINENO", "LINES", "MACHTYPE", "MAIL", "MAILCHECK",
		"MAILPATH", "MAPFILE", "OLDPWD", "OPTARG", "OPTERR", "OPTIND", "OSTYPE",
		"PATH", "PIPESTATUS", "POSIXLY_CORRECT", "PPID", "PROMPT_COMMAND",
		"PROMPT_DIRTRIM", "PS1", "PS2", "PS3", "PS4", "PSO", "PWD", "RANDOM",
		"READLINE_LINE", "READLINE_MARK", "READLINE_POINT", "REPLY", "SECONDS",
		"SHELL", "SHELLOPTS", "SHLVL", "SRANDOM", "TIMEFORMAT", "TMOUT",
		"TMPDIR", "UID", "alias", "auto_resume", "bg", "bind", "break",
		"builtin", "caller", "cd", "command", "continue", "declare", "dirs",
		"disown", "echo", "enable", "eval", "exec", "exit", "export", "fc",
		"fg", "getopts", "hash", "help", "histchars", "history", "jobs", "kill",
		"let", "local", "logout", "mapfile", "popd", "printf", "pushd", "pwd",
		"read", "readarray", "readonly", "return", "set", "shift", "shopt",
		"source", "suspend", "test", "times", "trap", "type", "typeset",
		"ulimit", "umask", "unalias", "unset", "wait"),

	.line_comments = { "#" },
	.quotes = "'\"",

	.blocks = BLOCKS(
		WORD_BLOCK("if", "fi"),
		WORD_BLOCK("case", "esac"),
		WORD_BLOCK("do", "done"),
		OP_BLOCK("{", "}")),
};
