#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <assert.h>
#include <inttypes.h>

#include "heap.h"

#if 0
#include <sys/resource.h>
#else
#ifdef __MACH__
#include <mach/mach_time.h>
#endif
#endif

struct el {
	PHEAP_ENTRY(struct el) link;
	int val;
};

static inline int
el_cmp(const struct el *a, const struct el *b)
{
	return a->val - b->val;
}

PHEAP_HEAD(h, struct el) heap_root;

PHEAP_PROTOTYPE(h, struct el, link, el_cmp, static)

PHEAP_GENERATE(h, struct el, link, el_cmp, static)

static uint64_t
timestamp(void)
{
#if 0
	struct rusage ru;
	getrusage(RUSAGE_SELF, &ru);
	return ((uint64_t)ru.ru_utime.tv_sec * 1000000ULL) + (uint64_t)ru.ru_utime.tv_usec;
#else
	return mach_absolute_time();
#endif
}

static double
ts2ns(uint64_t ts)
{
#if 0
	return (double)ts * 1000.0;
#else
	static mach_timebase_info_data_t tb;
	if (tb.denom == 0)
		mach_timebase_info(&tb);
	return ((double)ts * (double)tb.numer / (double)tb.denom);
#endif
}

#if 0
static int
heap_depth(struct el *el)
{
	int l, r;
	if (el == NULL)
		return 0;
	l = heap_depth(el->link.he_link[0]);
	r = heap_depth(el->link.he_link[1]);

	return 1 + (l > r ? l : r);
}
#endif

void
run_one(int nelem)
{
	struct el *elems;
	struct el *el;
	int lowest = 0;
	uint64_t s;
	uint64_t it, ut, rt;
	int inserts, updates, removes;
	int added;

	if ((elems = calloc(nelem, sizeof(*elems))) == NULL)
		err(1, "calloc");

	PHEAP_INIT(&heap_root);
	inserts = updates = removes = 0;
	it = ut = rt = 0;

	added = 0;
	while (added < nelem || PHEAP_FIRST(&heap_root)) {
		switch (arc4random_uniform(10)) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			if (added < nelem) {
				el = &elems[added++];
				el->val = lowest + arc4random_uniform(nelem);
				s = timestamp();
				PHEAP_INSERT(h, &heap_root, el);
				it += timestamp() - s;
				inserts++;
			}
			break;
		case 6:
		case 7:
			if ((el = PHEAP_FIRST(&heap_root)) != NULL) {
				el->val = lowest + arc4random_uniform(10);
				s = timestamp();
				PHEAP_UPDATE_HEAD(h, &heap_root);
				ut += timestamp() - s;
				updates++;
			}
			break;
		case 8:
		case 9:
			if ((el = PHEAP_FIRST(&heap_root)) != NULL) {
				assert(lowest <= el->val);
				lowest = el->val;
				s = timestamp();
				PHEAP_REMOVE_HEAD(h, &heap_root);
				rt += timestamp() - s;
				removes++;
			}
			break;
		}
	}

	printf("%d %f %f %f\n", nelem, ts2ns(it) / inserts, ts2ns(ut) / updates, ts2ns(rt) / removes);
	fflush(stdout);
	free(elems);
}

#ifndef nitems
#define nitems(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#endif
int tests[] = {
	10, 20, 50, 100, 200, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000
};

int
main(int argc, char **argv)
{
	int i;

	if (argc == 2) {
		run_one(atoi(argv[1]));
		return 0;
	}

	for (i = 0; i < nitems(tests) - 1; i++) {
		int distance = tests[i + 1] - tests[i];
		int steps = distance > 20 ? 20 : distance;
		int j;
		for (j = 0; j < steps; j++) {
			run_one(((tests[i + 1] - tests[i]) / steps) * j + tests[i]);
		}
	}

	return 0;
}
