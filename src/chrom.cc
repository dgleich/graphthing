#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "edge.h"
#include "factory.h"
#include "graph.h"
#include "polynomial.h"
#include "vertex.h"

#define RAND_SEED	109
#define VERTICES	12



// Kludge - fixes linking errors
#include <fstream>
std::fstream *yy_gt_fs;
int yy_gt_parse (void) { return 0; }
int yy_gt_debug;
Graph *new_graph;
void setProgress (double frac) { }



Graph *gen_graph (unsigned int vertices, unsigned int edges)
{
	if ((2 * edges) > (vertices * (vertices - 1))) {
		std::cerr << "FATAL: Too many edges requested!\n";
		return 0;
	}
	if ((2 * edges) == (vertices * (vertices - 1)))
		return Factory::K (vertices);

	Graph *g = Factory::N (vertices);

	while (g->num_edges () < edges) {
		// Pick two random vertices
		unsigned int n1, n2;

		n1 = int (double (vertices) * rand () / (RAND_MAX+1.0));
		n2 = int (double (vertices) * rand () / (RAND_MAX+1.0));
		if (n1 == n2)
			continue;
		//std::cerr << "[Picked (" << n1 << ", " << n2 << ")]";

		Vertex *v1 = (*g)[n1], *v2 = (*g)[n2];

		if (g->are_adjacent (v1, v2))
			continue;
		g->add (new Edge (v1, v2));
	}

	return g;
}


int main (void)
{
	std::cerr << "\n\tChromatic Polynomial tester\n\n";

	srand (RAND_SEED);

	for (unsigned int v = VERTICES; v <= VERTICES; ++v) {
	std::cerr << "v=" << v << ": ";
	for (unsigned int e = 0; e <= v*(v-1)/2; ++e) {
		Graph *g = gen_graph (v, e);
		struct timeval start_t, end_t;
		long total;
		Polynomial p;

		std::cerr << e << " ";

		gettimeofday (&start_t, 0);
		p = g->chromatic_polynomial ();
		gettimeofday (&end_t, 0);

		total = (end_t.tv_sec - start_t.tv_sec) * 1000 +
			(end_t.tv_usec - start_t.tv_usec) / 1000;
		std::cout << e << " " << total << "\n";

		delete g;
	}
	std::cerr << "\n";
	}

	return 0;
}
