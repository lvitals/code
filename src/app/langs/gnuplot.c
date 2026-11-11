/* Gnuplot */
#include "../lang_def.h"

LANGUAGE(gnuplot) {
	.name = "gnuplot",
	.title = "Gnuplot",
	.extensions = "dem plt",

	.keywords = WORDS(
		"call", "cd", "clear", "every", "exit", "fit", "help", "history", "if",
		"index", "load", "pause", "plot", "print", "pwd", "quit", "replot",
		"reread", "reset", "save", "set", "shell", "show", "smooth", "splot",
		"system", "test", "thru", "unset", "update", "using", "with"),

	.functions = WORDS(
		"abs", "acos", "acosh", "angles", "arg", "arrow", "asin", "asinh",
		"atan", "atan2", "atanh", "autoscale", "bars", "besj0", "besj1",
		"besy0", "besy1", "bmargin", "border", "boxwidth", "cbdata", "cbdtics",
		"cblabel", "cbmtics", "cbrange", "cbtics", "ceil", "clabel", "clip",
		"cntrparam", "colorbox", "column", "contour", "cos", "cosh", "datafile",
		"decimalsign", "defined", "dgrid3d", "dummy", "encoding", "erf", "erfc",
		"exp", "fit", "floor", "fontpath", "format", "function", "functions",
		"gamma", "grid", "hidden3d", "historysize", "ibeta", "igamma", "imag",
		"int", "inverf", "invnorm", "isosamples", "key", "label", "lambertw",
		"lgamma", "lmargin", "loadpath", "locale", "log", "log10", "logscale",
		"mapping", "margin", "mouse", "multiplot", "mx2tics", "mxtics",
		"my2tics", "mytics", "mztics", "norm", "offsets", "origin", "output",
		"palette", "parametric", "plot", "pm3d", "pointsize", "polar", "print",
		"rand", "real", "rmargin", "rrange", "samples", "sgn", "sin", "sinh",
		"size", "sqrt", "style", "surface", "tan", "tanh", "terminal", "tics",
		"ticscale", "ticslevel", "timefmt", "timestamp", "title", "tm_hour",
		"tm_mday", "tm_min", "tm_mon", "tm_sec", "tm_wday", "tm_yday",
		"tm_year", "tmargin", "trange", "urange", "valid", "variables",
		"version", "view", "vrange", "x2data", "x2dtics", "x2label", "x2mtics",
		"x2range", "x2tics", "x2zeroaxis", "xdata", "xdtics", "xlabel",
		"xmtics", "xrange", "xtics", "xzeroaxis", "y2data", "y2dtics",
		"y2label", "y2mtics", "y2range", "y2tics", "y2zeroaxis", "ydata",
		"ydtics", "ylabel", "ymtics", "yrange", "ytics", "yzeroaxis", "zdata",
		"zdtics", "zero", "zeroaxis", "zlabel", "zmtics", "zrange", "ztics"),

	.line_comments = { "#" },
	.quotes = "'\"",
};
