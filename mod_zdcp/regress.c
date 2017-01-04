#include <stdio.h>
#include "zdcp.h"

int main()
{
	const char *name;
	const double *probs;
	int sz, i;
	void *zdcp;
	void *zdcp2 = NULL;

	zdcp = zdcp_create("/opt/zdcp/models/industry/config.cfg");
        zdcp2 = zdcp_create("/opt/zdcp/models/genre/config.cfg");

	if (zdcp == NULL) {
		printf("error on creating zdcp: %s", zdcp_error());
		return -1;
	}
	printf("zdcp = %p zdcp2 = %p\n", zdcp, zdcp2);

	if (zdcp_classify(zdcp, "<subject: a><content: acontent><url: ><category: >", &name) < 0) {
		printf("error on classify: %s\n", zdcp_error());

		return -1;
	}

	printf("name = %s\n", name);

	sz = zdcp_classify_with_prob(zdcp, "<subject: a><content: acontent><url: ><category: >", &probs);
	if (sz < 0) {
		printf("error on classify: %s\n", zdcp_error());

		return -1;
	}
	printf("num of class = %d\n", sz);
	if (sz > 0) {
		for (i = 0; i < sz; i++) {
			printf("%s: %f\n", zdcp_getclassname(zdcp, i + 1), probs[i + 1]);
		}
	}

	if (zdcp2) {
		sz = zdcp_classify_with_prob(zdcp2, "<subject: a><content: acontent><url: ><category: >", &probs);
		if (sz < 0) {
			printf("error on classify: %s\n", zdcp_error());

			return -1;
		}
		printf("num of class = %d\n", sz);
		if (sz > 0) {
			for (i = 0; i < sz; i++) {
				printf("%s: %f\n", zdcp_getclassname(zdcp2, i + 1), probs[i + 1]);
			}
		}
	}

	return 0;
}	

