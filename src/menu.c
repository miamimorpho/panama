#include "menu.h"
#include "terminal.h"

int
menuInventory(struct Dungeon *d, struct Inventory *inven)
{
	struct TermUI ui = {0, 0};
	int alpha_i = 0;
	Handle *cur;
	VECTOR_EACH(inven, cur)
	{
		unsigned char alphabet = 'a' + alpha_i;
		termPut(&ui, utf8Code(alphabet));
		alpha_i++;
		termPuts(&ui, d->entt[cur->type].names[cur->id]);
		if (alpha_i >= 52)
			break;
	}
	ui.x = 0;
	ui.y++;
	termPuts(&ui, "PRESS ANY");
	termFlush();
	termGet();

	return 1;
}
