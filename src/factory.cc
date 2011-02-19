//
//	factory.cc
//

#include "wx/string.h"

#include <math.h>
#include <queue>
#include "edge.h"
#include "factory.h"
#include "graph.h"
#include "vertex.h"


int Factory::width = 100, Factory::height = 100;


Graph *Factory::grid (int rows, int columns)
{
	Graph *g;
	Vertex *v;
	int sx, sy, x, y;

	g = new Graph ();

	sx = width / (columns + 1);
	sy = height / (rows + 1);

	for (y = 0; y < rows; ++y) {
		for (x = 0; x < columns; ++x) {
			v = new Vertex ("", (x + 1) * sx, (y + 1) * sy);
			g->add (v);
		}
	}

	return g;
}

void Factory::construct_hanoi (Graph *g, int n, Vertex *varray[3],
							const wxString &suffix)
{
	if (n < 1)
		return;
	if (n == 1) {
		// just need to rename vertices and connect edges
		g->add (new Edge (varray[0], varray[1]));
		g->add (new Edge (varray[1], varray[2]));
		g->add (new Edge (varray[2], varray[0]));
		g->rename (varray[0], wxT("0") + suffix);
		g->rename (varray[1], wxT("1") + suffix);
		g->rename (varray[2], wxT("2") + suffix);
		return;
	}

	// create partial vertices
	Vertex *v01a, *v01b, *v12a, *v12b, *v20a, *v20b;
	int x0 = varray[0]->x, x1 = varray[1]->x, x2 = varray[2]->x,
	    y0 = varray[0]->y, y1 = varray[1]->y, y2 = varray[2]->y;
	v01a = new Vertex ("", (4 * x0 + 3 * x1) / 7, (4 * y0 + 3 * y1) / 7);
	v01b = new Vertex ("", (3 * x0 + 4 * x1) / 7, (3 * y0 + 4 * y1) / 7);
	v12a = new Vertex ("", (4 * x1 + 3 * x2) / 7, (4 * y1 + 3 * y2) / 7);
	v12b = new Vertex ("", (3 * x1 + 4 * x2) / 7, (3 * y1 + 4 * y2) / 7);
	v20a = new Vertex ("", (4 * x2 + 3 * x0) / 7, (4 * y2 + 3 * y0) / 7);
	v20b = new Vertex ("", (3 * x2 + 4 * x0) / 7, (3 * y2 + 4 * y0) / 7);
	g->add (v01a);
	g->add (v01b);
	g->add (v12a);
	g->add (v12b);
	g->add (v20a);
	g->add (v20b);

	// connect partial vertices
	g->add (new Edge (v01a, v01b));
	g->add (new Edge (v12a, v12b));
	g->add (new Edge (v20a, v20b));

	// recurse
	Vertex *varray0[3] = { varray[0], v20b, v01a };
	Vertex *varray1[3] = { v12a, varray[1], v01b };
	Vertex *varray2[3] = { v12b, v20a, varray[2] };
	construct_hanoi (g, n - 1, varray0, wxT("0") + suffix);
	construct_hanoi (g, n - 1, varray1, wxT("1") + suffix);
	construct_hanoi (g, n - 1, varray2, wxT("2") + suffix);
}

Graph *Factory::C (int n)
{
	Graph *g;
	Edge *e;
	Vertex *first, *prev;
	Graph::v_const_iterator vit;

	g = Factory::N (n);

	if (n < 2)
		return g;

	first = prev = 0;
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		if (prev) {
			e = new Edge (prev, *vit);
			g->add (e);
		} else
			first = *vit;
		prev = *vit;
	}

	if (n > 2) {
		e = new Edge (prev, first);
		g->add (e);
	}

	return g;
}

Graph *Factory::G (int n)
{
	Graph *g;
	Vertex *hub;

	g = Factory::W (n + 1, &hub);

	// Not quite correct, but we'll ignore trivial cases
	if (n < 3)
		return g;

	Graph::e_const_iterator eit;
	std::queue<Edge *> eq;

	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;
		if (e->incident_to (hub))
			continue;
		eq.push (e);
	}
	while (!eq.empty ()) {
		Edge *e = eq.front ();
		eq.pop ();

		Vertex *a = e->v, *b = e->w;
		g->remove (e);
		Vertex *mid = new Vertex ("", (a->x + b->x) / 2,
							(a->y + b->y) / 2);
		g->add (mid);
		g->add (new Edge (a, mid));
		g->add (new Edge (mid, b));
	}

	return g;
}

Graph *Factory::H (int n)
{
	Graph *g;

#if 0
	//g = Factory::N (3, 0.45 * (width > height ? height : width));
	g = Factory::N (3);
	Vertex *varray[3] = { (*g)[0], (*g)[2], (*g)[1] };	// deliberate!
	construct_hanoi (g, n, varray, "");
#else
	g = new Graph;
	wxString em = wxT("");
	g->add (new Vertex (em, int (width * 0.5), int (height * 0.1)));
	g->add (new Vertex (em, int (width * 0.9), int (height * 0.9)));
	g->add (new Vertex (em, int (width * 0.1), int (height * 0.9)));
	Vertex *varray[3] = { (*g)[0], (*g)[2], (*g)[1] };	// deliberate!
	construct_hanoi (g, n, varray, wxT(""));
#endif

	return g;
}

Graph *Factory::K (int n)
{
	Graph *g;
	Edge *e;
	Graph::v_const_iterator vit1, vit2;

	g = Factory::N (n);

	if (n < 2)
		return g;

	for (vit1 = g->v_begin (); vit1 != g->v_end (); ++vit1)
		for (vit2 = vit1 + 1; vit2 != g->v_end (); ++vit2) {
			e = new Edge (*vit1, *vit2);
			g->add (e);
		}

	return g;
}

Graph *Factory::K (int n, int m)
{
	Graph *g;
	Vertex *v;
	int i, j, sx, sy, x;

	g = new Graph ();

	// Create vertices in two rows
	sy = height / 3;

	sx = width / (n + 1);
	for (x = 0; x < n; ++x) {
		v = new Vertex ("", (x + 1) * sx, sy);
		g->add (v);
	}
	sx = width / (m + 1);
	for (x = 0; x < m; ++x) {
		v = new Vertex ("", (x + 1) * sx, 2 * sy);
		g->add (v);
	}


	// Add edges
	for (i = 0; i < n; ++i) {
		for (j = 0; j < m; ++j)
			g->add (new Edge ((*g)[i], (*g)[n + j]));
	}

	return g;
}

Graph *Factory::L (int n)
{
	Graph *g;
	Edge *e;
	Graph::v_const_iterator vit1, vit2;
	int i;

	g = grid (2, n);

	// vit1 follows the top row, vit2 follows the bottom row
	vit1 = g->v_begin ();
	for (vit2 = vit1, i = 0; i < n; ++i)
		++vit2;

	for (i = 0; i < n; ++i, ++vit1, ++vit2) {
		e = new Edge (*vit1, *vit2);
		g->add (e);
	}

	return g;
}

Graph *Factory::N (int n, double radius)
{
	Graph *g;
	double del_theta, phi;
	int i, cx, cy, x, y;

	g = new Graph ();

	// Null graph - arrange n vertices in a circle

	cx = width / 2;
	cy = height / 2;
	if (radius <= 0)
		radius = ((width > height) ? height : width) / 3;
	del_theta = 2 * M_PI / (double) n;
	phi = -M_PI_2;
	if (n == 4)
		phi = -0.75 * M_PI;

	if (n < 2)
		radius = 0;		// place single vertex in middle

	for (i = 0; i < n; ++i) {
		x = cx + (int) ceil (radius * cos (i * del_theta + phi));
		y = cy + (int) ceil (radius * sin (i * del_theta + phi));

		g->add (new Vertex ("", x, y));
	}

	return g;
}

Graph *Factory::S (int n)
{
	Graph *g;
	Edge *e;
	Vertex *v;
	Graph::v_const_iterator vit;

	g = N (n - 1);

	v = new Vertex ("", width / 2, height / 2);
	g->add (v);

	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		if (*vit == v)
			continue;
		e = new Edge (v, *vit);
		g->add (e);
	}

	return g;
}

Graph *Factory::W (int n, Vertex **hub)
{
	Graph *g;
	Edge *e;
	Vertex *v;
	Graph::v_const_iterator vit;

	g = C (n - 1);

	v = new Vertex ("", width / 2, height / 2);
	g->add (v);
	if (hub)
		*hub = v;

	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		if (*vit == v)
			continue;
		e = new Edge (v, *vit);
		g->add (e);
	}

	return g;
}

Graph *Factory::Lattice (int n, int m)
{
	Graph *g;
	Edge *e;
	int i, j;

	g = grid (n, m);

	// Add horizontal edges
	for (j = 0; j < n; ++j) {
		for (i = 1; i < m; ++i) {
			e = new Edge ((*g)[j * m + i - 1], (*g)[j * m + i]);
			g->add (e);
		}
	}

	// Add vertical edges
	for (i = 0; i < m; ++i) {
		for (j = 0; j < (n - 1); ++j) {
			e = new Edge ((*g)[i + m * j], (*g)[i + m * (j+1)]);
			g->add (e);
		}
	}

	return g;
}

Graph *Factory::Petersen ()
{
	Graph *g;
	Edge *e;
	Vertex *v;
	double del_theta, r;
	int i, cx, cy, x, y;

	g = C (5);

	// Create the 5 inner vertices
	cx = width / 2;
	cy = height / 2;
	r = ((width < height) ? height : width) / 6;
	del_theta = M_PI * 0.4;
	for (i = 0; i < 5; ++i) {
		x = cx + (int) ceil (r * cos (i * del_theta - M_PI_2));
		y = cy + (int) ceil (r * sin (i * del_theta - M_PI_2));

		v = new Vertex ("", x, y);
		g->add (v);

		// Connect an edge to the corresponding vertices
		e = new Edge ((*g)[i], v);
		g->add (e);
	}

	// Make the star
	g->add (new Edge ((*g)[5], (*g)[7]));
	g->add (new Edge ((*g)[7], (*g)[9]));
	g->add (new Edge ((*g)[9], (*g)[6]));
	g->add (new Edge ((*g)[6], (*g)[8]));
	g->add (new Edge ((*g)[8], (*g)[5]));


	return g;
}

Graph *Factory::Tetrahedral ()
{
	return W (4);
}

Graph *Factory::Cubical ()
{
	Graph *g;
	Vertex *v;
	double del_theta, phi, r;
	int i, cx, cy, x, y;

	g = C (4);

	// Create the 4 inner vertices
	cx = width / 2;
	cy = height / 2;
	r = ((width < height) ? height : width) / 8;
	del_theta = M_PI_2;
	phi = -0.75 * M_PI;
	for (i = 0; i < 4; ++i) {
		x = cx + (int) ceil (r * cos (i * del_theta + phi));
		y = cy + (int) ceil (r * sin (i * del_theta + phi));

		v = new Vertex ("", x, y);
		g->add (v);

		// Connect an edge to the corresponding vertices
		g->add (new Edge ((*g)[i], v));
	}

	// Join the inner vertices into a square
	g->add (new Edge ((*g)[4], (*g)[5]));
	g->add (new Edge ((*g)[5], (*g)[6]));
	g->add (new Edge ((*g)[6], (*g)[7]));
	g->add (new Edge ((*g)[7], (*g)[4]));


	return g;
}

Graph *Factory::Octahedral ()
{
	Graph *g;
	double del_theta, phi, r;
	int i, cx, cy, x, y;

	g = C (3);

	// Create the 3 inner vertices
	cx = width / 2;
	cy = height / 2;
	r = ((width < height) ? height : width) / 20;
	del_theta = M_PI * 2 / 3;
	phi = -M_PI * 5 / 6;
	for (i = 0; i < 3; ++i) {
		x = cx + (int) ceil (r * cos (i * del_theta + phi));
		y = cy + (int) ceil (r * sin (i * del_theta + phi));

		g->add (new Vertex ("", x, y));
	}

	// Join the inner vertices into a cycle
	g->add (new Edge ((*g)[3], (*g)[4]));
	g->add (new Edge ((*g)[4], (*g)[5]));
	g->add (new Edge ((*g)[5], (*g)[3]));

	// Join the outer to the inner vertices
	g->add (new Edge ((*g)[0], (*g)[3]));
	g->add (new Edge ((*g)[0], (*g)[4]));
	g->add (new Edge ((*g)[1], (*g)[4]));
	g->add (new Edge ((*g)[1], (*g)[5]));
	g->add (new Edge ((*g)[2], (*g)[5]));
	g->add (new Edge ((*g)[2], (*g)[3]));


	return g;
}

Graph *Factory::Dodecahedral ()
{
	Graph *g;
	double del_theta, phi, r, rr;
	int layer, i, cx, cy, x, y;
	double layer_scale[4] = { 1, 0.8, 0.5, 0.3 };
	struct {
		int a, b;
	} edges[30] = {
		{0,1}, {1,2}, {2,3}, {3,4}, {4,0},
		{0,5}, {1,6}, {2,7}, {3,8}, {4,9},
		{5,10}, {10,6}, {6,11}, {11,7}, {7,12},
		{12,8}, {8,13}, {13,9}, {9,14}, {14,5},
		{10,15}, {11,16}, {12,17}, {13,18}, {14,19},
		{15,16}, {16,17}, {17,18}, {18,19}, {19,15}
	};

	g = new Graph ();

	cx = width / 2;
	cy = height / 2;
	r = ((width < height) ? height : width) / 3.5;
	del_theta = M_PI * 2 / 5;
	phi = -M_PI_2;

	for (layer = 0; layer < 4; ++layer) {
		if (layer == 2)
			phi += del_theta / 2;
		rr = r * layer_scale[layer];
		for (i = 0; i < 5; ++i) {
			x = cx + (int) ceil
					(rr * cos (i * del_theta + phi));
			y = cy + (int) ceil
					(rr * sin (i * del_theta + phi));
			g->add (new Vertex ("", x, y));
		}
	}

	// Add edges
	for (i = 0; i < 30; ++i)
		g->add (new Edge ((*g)[edges[i].a], (*g)[edges[i].b]));

	return g;
}

Graph *Factory::Icosahedral ()
{
	Graph *g;
	double del_theta, phi, r, rr;
	int layer, i, cx, cy, x, y;
	double layer_scale[4] = { 1, 0.35, 0.35, 0.1 };
	struct {
		int a, b;
	} edges[30] = {
		{0,1}, {1,2}, {2,0},
		{0,8}, {0,3}, {0,6},
		{1,6}, {1,4}, {1,7},
		{2,7}, {2,5}, {2,8},
		{3,6}, {6,4}, {4,7}, {7,5}, {5,8}, {8,3},
		{3,11}, {3,9}, {4,9}, {4,10}, {5,10}, {5,11},
		{6,9}, {7,10}, {8,11},
		{9,10}, {10,11}, {11,9}
	};

	g = new Graph ();

	cx = width / 2;
	cy = int (height * 0.6);
	//r = ((width < height) ? height : width) / 3.5;
	r = ((width < height) ? width : height) * 0.5;
	del_theta = M_PI * 2 / 3;
	phi = -M_PI_2;

	for (layer = 0; layer < 4; ++layer) {
		if (layer == 2)
			phi += del_theta / 2;
		rr = r * layer_scale[layer];
		for (i = 0; i < 3; ++i) {
			x = cx + (int) ceil
					(rr * cos (i * del_theta + phi));
			y = cy + (int) ceil
					(rr * sin (i * del_theta + phi));
			g->add (new Vertex ("", x, y));
		}
	}

	// Add edges
	for (i = 0; i < 30; ++i)
		g->add (new Edge ((*g)[edges[i].a], (*g)[edges[i].b]));

	return g;
}
