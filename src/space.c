#include "maths.h"
#include "plist.h"
#include "space.h"

struct SpaceElm {
	PLIST_ENTRY() link;
	vec16 pos;
};
PLIST_HEAD(SpaceHead);

struct Space {
	uint32_t gran; // granularity
	size_t c;
	struct SpaceHead *hash;
	struct SpaceElm *elms;
};

struct Space *
spaceCreate(int len, int c)
{
	struct Space *s = malloc(sizeof(struct Space));
	assert(IS_POWER_OF_TWO(len));
	assert(IS_POWER_OF_TWO(c));
	s->gran = len;
	s->c = c;

	s->hash = calloc(c, sizeof(struct SpaceHead));
	for (size_t i = 0; i < s->c; i++) {
		PLIST_INIT(&s->hash[i]);
	}

	s->elms = calloc(c, sizeof(struct SpaceElm));

	return s;
}

static struct SpaceHead *
headGetQuantized(struct Space *s, vec16 p)
{
	uint32_t key;
	vec16Handle(p, &key);
	size_t i = MODULO_POWER_OF_TWO(hashFunction32(key), s->c);
	return &s->hash[i];
}

static void
quantize(struct Space *s, vec16 in, vec16 out)
{
	out[0] = floorDiv(in[0], s->gran);
	out[1] = floorDiv(in[1], s->gran);
}

static struct SpaceHead *
headGet(struct Space *s, vec16 p)
{
	vec16 qntz;
	quantize(s, p, qntz);
	return headGetQuantized(s, qntz);
}

static struct SpaceElm *
headSearch(struct Space *s, struct SpaceHead *h, vec16 p)
{
	struct SpaceElm *elm;
	PLIST_FOREACH(s->elms, h, elm, link)
	{
		if (vec16Equal(elm->pos, p)) {
			return elm;
		}
	}
	return NULL;
}

int
spaceGet(struct Space *s, HandleID *out, vec16 p)
{
	struct SpaceHead *h = headGet(s, p);
	struct SpaceElm *elm = headSearch(s, h, p);
	if (out && elm) {
		*out = elm - s->elms;
		return 0;
	}
	return 1;
}

void
spaceInsert(struct Space *s, HandleID e, vec16 p)
{
	struct SpaceElm *elm = &s->elms[e];
	struct SpaceHead *h = headGet(s, p);
	vec16Copy(p, elm->pos);
	PLIST_INSERT_HEAD(s->elms, h, elm, link);
}

void
spaceRemove(struct Space *s, HandleID e)
{
	struct SpaceElm *elm = &s->elms[e];
	struct SpaceHead *h = headGet(s, elm->pos);
	PLIST_REMOVE(s->elms, h, elm, link);
}

void
spaceMove(struct Space *s, HandleID e, vec16 next)
{
	struct SpaceElm *elm = &s->elms[e];
	struct SpaceHead *src = headGet(s, elm->pos);
	PLIST_REMOVE(s->elms, src, elm, link);

	vec16Copy(next, elm->pos);
	struct SpaceHead *dst = headGet(s, elm->pos);
	PLIST_INSERT_HEAD(s->elms, dst, elm, link);
}

int
spaceWhere(struct Space *s, HandleID e, vec16 out)
{
	HandleID tmp;
	if (e >= s->c)
		return 1;
	if (spaceGet(s, &tmp, s->elms[e].pos)) {
		return 1;
	}
	vec16Copy(s->elms[e].pos, out);
	return 0;
}

struct SpaceElm *
findFirstElmInHash(struct SpaceFinder *f)
{
	if (f->cur >= f->total)
		return NULL;

	vec16 qntz = {f->origin[0] - (f->range / 2) + (f->cur % f->range),
				  f->origin[1] - (f->range / 2) + (f->cur / f->range)};
	struct SpaceHead *h = headGetQuantized(f->s, qntz);
	if (!h)
		return NULL;

	vec16 elm_hash;
	struct SpaceElm *elm;
	PLIST_FOREACH(f->s->elms, h, elm, link)
	{
		quantize(f->s, elm->pos, elm_hash);
		if (vec16Equal(elm_hash, qntz)) {
			return elm;
		}
	}

	return NULL;
}

HandleID
spaceFindNext(struct SpaceFinder *f, HandleID in)
{
	if (in != NULL_HANDLE) {
		struct SpaceElm *cur = &f->s->elms[in];
		struct SpaceElm *next = PLIST_NEXT(f->s->elms, cur, link);
		if (next) {
			return next - f->s->elms;
		}
	}

	f->cur++;
	while (f->cur < f->total) {
		struct SpaceElm *first = findFirstElmInHash(f);
		if (first) {
			return first - f->s->elms;
		}
		f->cur++;
	}
	return NULL_HANDLE;
}

// todo limit search to real cell values
HandleID
spaceFindStart(struct Space *s, struct SpaceFinder *f, vec16 o, uint32_t range)
{
	*f = (struct SpaceFinder) {0};
	f->s = s;
	quantize(s, o, f->origin);
	uint32_t range_qntz = (range + s->gran) / s->gran;
	f->range = 2 * (range_qntz / 2) + 1;
	f->total = range * range;

	struct SpaceElm *first = findFirstElmInHash(f);
	if (first) {
		return first - s->elms;
	}
	return spaceFindNext(f, NULL_HANDLE);
}
