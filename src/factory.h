//
//	factory.h
//

#ifndef __FACTORY_H__
#define __FACTORY_H__

#include "wx/string.h"

class Graph;
class Vertex;


class Factory
{
private:
	static Graph *grid (int rows, int columns);
	static void construct_hanoi (Graph *g, int n, Vertex *varray[3],
							const wxString &suffix);

public:
	static int width;
	static int height;

	static Graph *C (int n);		// cycle
	static Graph *G (int n);		// gear
	static Graph *H (int n);		// hanoi
	static Graph *K (int n);		// complete
	static Graph *K (int n, int m);		// complete bipartite
	static Graph *L (int n);		// ladder
	static Graph *N (int n, double radius = -1);	// null
	static Graph *S (int n);		// star
	static Graph *W (int n, Vertex **hub = 0);	// wheel

	static Graph *Lattice (int n, int m);
	static Graph *Petersen ();

	// Platonic solids
	static Graph *Tetrahedral ();
	static Graph *Cubical ();
	static Graph *Octahedral ();
	static Graph *Dodecahedral ();
	static Graph *Icosahedral ();
};

#endif	// __FACTORY_H__
