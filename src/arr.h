#ifndef ARR_H
#define ARR_H

#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline size_t
arrOverflowCheck(size_t nmemb, size_t stride)
{
	if (stride == 0 || nmemb == 0)
		return 0;

	assert(nmemb < SIZE_MAX / stride);

	return nmemb * stride;
}

static inline void *
arrMalloc(size_t nmemb, size_t stride, size_t *out)
{

	if (!(*out = arrOverflowCheck(nmemb, stride)))
		return NULL;

	void *ptr;
	if (!(ptr = malloc(*out)))
		return NULL;

	memset(ptr, 0, *out);
	return ptr;
}

#endif // ARR_H
