/* Perl */
#include "../lang_def.h"

LANGUAGE(perl) {
	.name = "perl",
	.title = "Perl",
	.flags = F_VARS,
	.extensions = "al perl pl pm pod",
	.interpreters = "perl",

	.keywords = WORDS(
		"BEGIN", "CHECK", "END", "INIT", "STDERR", "STDIN", "STDOUT",
		"__FILE__", "__LINE__", "__PACKAGE__", "and", "break", "continue", "do",
		"each", "else", "elsif", "eq", "for", "foreach", "ge", "gt", "if",
		"last", "le", "local", "lt", "my", "ne", "next", "not", "or", "our",
		"package", "require", "return", "sub", "unless", "until", "use", "while"),

	.constants = WORDS(
		"ARGV", "ARGVOUT", "DATA", "ENV", "INC", "SIG", "STDERR", "STDIN",
		"STDOUT"),

	.functions = WORDS(
		"abs", "accept", "alarm", "atan2", "bind", "binmode", "bless", "caller",
		"chdir", "chmod", "chomp", "chop", "chown", "chr", "chroot", "close",
		"closedir", "connect", "cos", "crypt", "dbmclose", "dbmopen", "defined",
		"delete", "die", "dump", "each", "endgrent", "endhostent", "endnetent",
		"endprotoent", "endpwent", "endservent", "eof", "eval", "exec",
		"exists", "exit", "exp", "fcntl", "fileno", "flock", "fork", "format",
		"formline", "getc", "getgrent", "getgrgid", "getgrnam", "gethostbyaddr",
		"gethostbyname", "gethostent", "getlogin", "getnetbyaddr",
		"getnetbyname", "getnetent", "getpeername", "getpgrp", "getppid",
		"getpriority", "getprotobyname", "getprotobynumber", "getprotoent",
		"getpwent", "getpwnam", "getpwuid", "getservbyname", "getservbyport",
		"getservent", "getsockname", "getsockopt", "glob", "gmtime", "goto",
		"grep", "hex", "import", "index", "int", "ioctl", "join", "keys",
		"kill", "lc", "lcfirst", "length", "link", "listen", "localtime", "log",
		"lstat", "map", "mkdir", "msgctl", "msgget", "msgrcv", "msgsnd", "new",
		"oct", "open", "opendir", "ord", "pack", "pipe", "pop", "pos", "print",
		"printf", "prototype", "push", "quotemeta", "rand", "read", "readdir",
		"readlink", "recv", "redo", "ref", "rename", "reset", "reverse",
		"rewinddir", "rindex", "rmdir", "scalar", "seek", "seekdir", "select",
		"semctl", "semget", "semop", "send", "setgrent", "sethostent",
		"setnetent", "setpgrp", "setpriority", "setprotoent", "setpwent",
		"setservent", "setsockopt", "shift", "shmctl", "shmget", "shmread",
		"shmwrite", "shutdown", "sin", "sleep", "socket", "socketpair", "sort",
		"splice", "split", "sprintf", "sqrt", "srand", "stat", "study",
		"substr", "symlink", "syscall", "sysread", "sysseek", "system",
		"syswrite", "tell", "telldir", "tie", "tied", "time", "times",
		"truncate", "uc", "ucfirst", "umask", "undef", "unlink", "unpack",
		"unshift", "untie", "utime", "values", "vec", "wait", "waitpid",
		"wantarray", "warn", "write"),

	.line_comments = { "#" },
	.block_comments = { { "=", "=cut" } },
	.quotes = "'\"`",

	.blocks = BLOCKS(
		OP_BLOCK("[", "]"),
		OP_BLOCK("{", "}")),
};
