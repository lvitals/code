/* RHTML */
#include "../lang_def.h"

LANGUAGE(rhtml) {
	.name = "rhtml",
	.title = "RHTML",
	.kind = K_XML,
	.extensions = "erb rhtml",

	.keywords = WORDS(
		"a", "abbr", "address", "area", "article", "aside", "audio", "b",
		"base", "bdi", "bdo", "blockquote", "body", "br", "button", "canvas",
		"caption", "cite", "code", "col", "colgroup", "command", "content",
		"data", "datalist", "dd", "decorator", "del", "details", "dfn", "div",
		"dl", "doctype", "dt", "element", "em", "embed", "fieldset",
		"figcaption", "figure", "footer", "form", "h1", "h2", "h3", "h4", "h5",
		"h6", "head", "header", "hr", "html", "i", "iframe", "img", "input",
		"ins", "kbd", "keygen", "label", "legend", "li", "link", "main", "map",
		"mark", "menu", "menuitem", "meta", "meter", "nav", "noscript",
		"object", "ol", "optgroup", "option", "output", "p", "param", "pre",
		"progress", "q", "rp", "rt", "ruby", "s", "samp", "script", "section",
		"select", "shadow", "small", "source", "spacer", "span", "strong",
		"style", "sub", "summary", "sup", "table", "tbody", "td", "template",
		"textarea", "tfoot", "th", "thead", "time", "title", "tr", "track", "u",
		"ul", "var", "video", "wbr"),

	.preproc = WORDS(
		"accept", "accept-charset", "accesskey", "action", "align", "alt",
		"async", "autocomplete", "autofocus", "autoplay", "bgcolor", "border",
		"buffered", "challenge", "charset", "checked", "cite", "class", "code",
		"codebase", "color", "cols", "colspan", "content", "contenteditable",
		"contextmenu", "controls", "coords", "data", "data-", "datetime",
		"default", "defer", "dir", "dirname", "disabled", "download",
		"draggable", "dropzone", "enctype", "for", "form", "headers", "height",
		"hidden", "high", "href", "hreflang", "http-equiv", "icon", "id",
		"ismap", "itemprop", "keytype", "kind", "label", "lang", "language",
		"list", "loop", "low", "manifest", "max", "maxlength", "media",
		"method", "min", "multiple", "name", "novalidate", "open", "optimum",
		"pattern", "ping", "placeholder", "poster", "preload", "pubdate",
		"radiogroup", "readonly", "rel", "required", "reversed", "role", "rows",
		"rowspan", "sandbox", "scope", "scoped", "seamless", "selected",
		"shape", "size", "sizes", "span", "spellcheck", "src", "srcdoc",
		"srclang", "start", "step", "style", "summary", "tabindex", "target",
		"title", "type", "usemap", "value", "width", "wrap"),

	.block_comments = { { "<!--", "-->" } },
	.quotes = "'\"",

	.blocks = BLOCKS(
		PRE_BLOCK("<%", "%>")),
};
