//
//	edge.h
//

#ifndef __EDGE_H__
#define __EDGE_H__

#include <iostream>

class Vertex;


class Edge
{
public:
	Vertex *v, *w;
	int flow;		// for networks (<= weight)
	int weight;
	bool directed;
	bool selected;
	int selection_colour;

	Edge *next;		// selected chain
	long mark;


	Edge (Vertex *a, Vertex *b, bool dir = false, int wt = 1);
	Edge (const Edge &other);

	void cycle_orientations ();

	Edge &operator= (const Edge &other);
	bool operator== (const Edge &other) const;
	friend std::ostream &operator<< (std::ostream &o, const Edge &e);

	bool incident_to (const Vertex *v1) const;
	bool incident_to (const Vertex *v1, const Vertex *v2) const;
};

#endif	// __EDGE_H__
