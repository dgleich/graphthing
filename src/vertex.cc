//
//	vertex.cc
//

#include "wx/string.h"

#include <iostream>
#include "edge.h"
#include "vertex.h"


Vertex::Vertex (wxString &lbl, int _x, int _y)
	: label (lbl), x (_x), y (_y), selected (false), next (0)
{
}

Vertex::Vertex (char *lbl, int _x, int _y)
	: label (wxString (lbl, wxConvUTF8)), x (_x), y (_y),
						selected (false), next (0)
{
}

Vertex::Vertex (const Vertex &other)
	: label (other.label), x (other.x), y (other.y), selected (false),
								next (0)
{
}

unsigned int Vertex::degree () const
{
	return edges.size ();
}

unsigned int Vertex::indegree () const
{
	e_const_iterator eit;
	int cnt = 0;

	for (eit = e_begin (); eit != e_end (); ++eit)
		if ((*eit)->directed && ((*eit)->w == this))
			++cnt;
	return cnt;
}

unsigned int Vertex::outdegree () const
{
	e_const_iterator eit;
	int cnt = 0;

	for (eit = e_begin (); eit != e_end (); ++eit)
		if ((*eit)->directed && ((*eit)->v == this))
			++cnt;
	return cnt;
}

Vertex *Vertex::opposite (const Edge *e) const
{
	return (e->v == this) ? e->w : e->v;
}

Vertex::e_iterator Vertex::e_begin ()
{
	return edges.begin ();
}

Vertex::e_iterator Vertex::e_end ()
{
	return edges.end ();
}

Vertex::e_const_iterator Vertex::e_begin () const
{
	return edges.begin ();
}

Vertex::e_const_iterator Vertex::e_end () const
{
	return edges.end ();
}

void Vertex::hook (Edge *e)
{
	edges.push_back (e);
}

void Vertex::unhook (Edge *e)
{
	e_iterator eit;

	for (eit = e_begin (); eit != e_end (); ++eit)
		if (*eit == e)
			break;
	if (eit == e_end ())
		return;		// fail silently
	edges.erase (eit);
}

Vertex &Vertex::operator= (const Vertex &other)
{
	if (this == &other)
		return *this;

	label = other.label;
	x = other.x;
	y = other.y;

	return *this;
}

std::ostream &operator<< (std::ostream &o, const Vertex &v)
{
	return (o << "vertex \"" << v.label.mb_str (wxConvUTF8) << "\" at ("
			<< v.x << ',' << v.y << ")\n");
}
