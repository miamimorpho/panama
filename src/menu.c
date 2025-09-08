// #include "menu.h"
// #include "terminal.h"

/*
void
menuInventory(struct Dungeon *d, struct HandleTyped *inventory)
{
	struct TermUI ui = {0, 0};
	int alpha_i = 0;
	struct Mobile *mob;
	PLIST_FOREACH(d->items->mobs.root, inventory, mob, link)
	{
		size_t i = mob - d->items->mobs.root;
		unsigned char alphabet = 'a' + alpha_i;
		termPut(&ui, utf8Code(alphabet));
		alpha_i++;
		termPuts(&ui, d->items->names[i]);
		if (alpha_i >= 52)
			break;
	}
	ui.x = 0;
	ui.y++;
	termPuts(&ui, "PRESS ANY");
	termRefresh();
	termGet();
}

*/
