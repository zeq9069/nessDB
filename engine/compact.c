#include <stdlib.h>

#include "compact.h"
#include "config.h"
#include "debug.h"

struct compact *cpt_new()
{
	struct compact *cpt = calloc(1, sizeof(struct compact));
	if (!cpt)
		return NULL;

	cpt->nodes = calloc(NESSDB_MAX_VAL_SIZE, sizeof(struct compact*));

	return cpt;
}

int cpt_add(struct compact *cpt, int v_len, uint64_t offset)
{
	struct cpt_node *n;

	if (v_len >= NESSDB_MAX_VAL_SIZE) {
		__ERROR("val length#%d more than NESSDB_MAX_VAL_SIZE#%d", v_len, NESSDB_MAX_VAL_SIZE);
		return 0;
	}

	n = calloc(1, sizeof(struct cpt_node));
	n->offset = offset;
	n->nxt = cpt->nodes[v_len];
	cpt->nodes[v_len] = n;

	cpt->count++;

	return 1;
}

uint64_t cpt_get(struct compact *cpt, int v_len)
{
	uint64_t off = 0UL;
	struct cpt_node *n;

	if (v_len >= NESSDB_MAX_VAL_SIZE) {
		__ERROR("val length#%d more than NESSDB_MAX_VAL_SIZE#%d", v_len, NESSDB_MAX_VAL_SIZE);
		return off;
	}

	n = cpt->nodes[v_len];

	if (n) {
		off = n->offset;
		cpt->nodes[v_len] = n->nxt;
		free(n);
		cpt->count--;
	} else {
		int i;

		/* find next one, gap maybe v_len + x */
		for (i = v_len + 1; i < NESSDB_MAX_VAL_SIZE; i++) {
			n = cpt->nodes[i];
			if (n) {
				off = n->offset;
				cpt->nodes[i] = n->nxt;
				free(n);
				cpt->count--;
				break;
			}
		}
	}

	return off;
}

void cpt_free(struct compact *cpt)
{
	if (cpt) {
		int i;

		for (i = 0; i < NESSDB_MAX_VAL_SIZE; i++) {
			struct cpt_node *n = cpt->nodes[i], *nxt;

			while (n) {
				nxt = n->nxt;
				free(n);
				n = nxt;
			}
		}

		free(cpt->nodes);
		free(cpt);
	}
}
