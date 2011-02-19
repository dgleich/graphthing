//
//	vertex.h
//

#ifndef __VERTEX_H__
#define __VERTEX_H__

#include "wx/string.h"

#include <iostream>
#include <vector>

class Edge;


class Vertex
{
private:
	std::vector<Edge *> edges;	// Edges adjacent to this vertex

public:

	typedef std::vector<Edge *>::iterator e_iterator;
	typedef std::vector<Edge *>::const_iterator e_const_iterator;

	wxString label;
	int x;
	int y;
	bool selected;
	int selection_colour;
	Vertex *next;
	long mark;

	Vertex *map_to;		// speed up for graph cloning


	Vertex (wxString &lbl, int _x, int _y);
	Vertex (char *lbl, int _x, int _y);
	Vertex (const Vertex &other);

	unsigned int degree () const;
	unsigned int indegree () const;
	unsigned int outdegree () const;

	Vertex *opposite (const Edge *e) const;

	e_iterator e_begin ();
	e_iterator e_end ();
	e_const_iterator e_begin () const;
	e_const_iterator e_end () const;
	void hook (Edge *e);
	void unhook (Edge *e);

	Vertex &operator= (const Vertex &other);
	friend std::ostream &operator<< (std::ostream &o, const Vertex &v);
};

#endif	// __VERTEX_H__
