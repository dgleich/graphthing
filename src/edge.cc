//
//	edge.cc
//

#include <iostream>
#include <stdexcept>
#include "edge.h"
#include "vertex.h"



Edge::Edge (Vertex *a, Vertex *b, bool dir, int wt) : v (a), w (b),
		flow (0), weight (wt), directed (dir), selected (false)
{
	if (a == b)
		throw std::invalid_argument ("Loops not yet supported.");
}

Edge::Edge (const Edge &other) : v (other.v), w (other.w),
		flow (other.flow), weight (other.weight),
		directed (other.directed), selected (false)
{
}

void Edge::cycle_orientations ()
{
	// NOTE: this relies on vertex comparability
	if (!directed)
		directed = true;
	else {
		if (v->label < w->label)
			directed = false;
		Vertex *tmp = v;
		v = w;
		w = tmp;
	}
}

Edge &Edge::operator= (const Edge &other)
{
	if (this == &other)
		return *this;

	v = other.v;
	w = other.w;
	flow = other.flow;
	weight = other.weight;
	directed = other.directed;

	return *this;
}

bool Edge::operator== (const Edge &other) const
{
	if (!directed)
		return (((v == other.v) && (w == other.w)) ||
			((v == other.w) && (w == other.v)));
	else
		return ((v == other.v) && (w == other.w));
}

std::ostream &operator<< (std::ostream &o, const Edge &e)
{
	o << "edge \"" << e.v->label.mb_str (wxConvUTF8) << "\" " <<
		(e.directed ? "-> \"" : "-- \"")
		<< e.w->label.mb_str (wxConvUTF8) << "\"";

	if (e.weight != 1)
		o << " with weight " << e.weight;

	o << '\n';

	return o;
}

bool Edge::incident_to (const Vertex *v1) const
{
	return ((v == v1) || (w == v1));
}

bool Edge::incident_to (const Vertex *v1, const Vertex *v2) const
{
	return (incident_to (v1) && incident_to (v2));
}
