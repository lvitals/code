/* What of the file picker the rest calls: open it, close it, do what it
 * was for; and its entries. */
#include <string.h>
#include "app.h"
#include "../core/mem.h"

char *pk_names;		/* the entries, one after another */
unsigned *pk_ents;	/* where each starts */
int pk_n, pk_loaded, pk_sel, pk_top;

void
picker_open(int kind)
{
	st.picker = kind;
	pk_loaded = 0;
	if(kind == PICK_SAVEAS)
		edl_set(&st.pname, docs ? docs->name : "Untitled.txt");
	else
		edl_set(&st.pname, "");
	ui_again = 1;
}

void
close_picker(void)
{
	st.picker = PICK_NONE;
	mem_free(st.overwrite);
	st.overwrite = 0;
	pk_loaded = 0;
	mem_free(pk_names);
	mem_free(pk_ents);
	pk_names = 0;
	pk_ents = 0;
	pk_n = 0;
	ui_again = 1;
}

void
picker_finish(const char *path)
{
	int ok;

	if(st.picker == PICK_OPEN)
		ok = doc_open(path) != 0;
	else if(st.picker == PICK_FOLDER)
		ok = project_set(path);
	else
		ok = docs && doc_save(path);
	if(ok)
		close_picker();
}

