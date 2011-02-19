//
//	graph.cc
//

#include "wx/string.h"

#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string.h>
#include <time.h>
#include <vector>
#include "config.h"
#include "edge.h"
#include "graph.h"
#include "vertex.h"


void Graph::reindex ()
{
	Vmap.erase (Vmap.begin (), Vmap.end ());

	Graph::v_const_iterator vit;
	for (vit = v_begin (); vit != v_end (); ++vit)
		Vmap[(*vit)->label] = *vit;
}

Graph::Graph ()
{
	e_selected_head = 0;
	v_selected_head = 0;
}

Graph::Graph (const Graph &other)
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	Vertex *v;

	e_selected_head = 0;
	v_selected_head = 0;

	// Copy vertices across
	for (vit = other.v_begin (); vit != other.v_end (); ++vit) {
		v = new Vertex (**vit);
		(*vit)->map_to = v;
		add (v);
	}

	// Now copy edges
	for (eit = other.e_begin (); eit != other.e_end (); ++eit) {
		Edge *e = new Edge ((*eit)->v->map_to, (*eit)->w->map_to,
					(*eit)->directed, (*eit)->weight);
		add (e);
	}
}

Graph::~Graph ()
{
	clear ();
}

std::ostream &operator<< (std::ostream &o, const Graph &g)
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	Graph::tag_iterator tit;

	// Info section - empty at the moment
	o << "info {\n";
	for (tit = g.tag_begin (); tit != g.tag_end (); ++tit) {
		o << "\t" << tit->first.mb_str (wxConvUTF8) << " = \""
			<< tit->second.mb_str (wxConvUTF8) << "\"\n";
	}
	o << "}\n\n";

	// Dump vertices
	for (vit = g.v_begin (); vit != g.v_end (); ++vit)
		o << **vit;
	o << '\n';

	// Dump edges
	for (eit = g.e_begin (); eit != g.e_end (); ++eit)
		o << **eit;

	return o;
}

Graph *Graph::load (const wxString &fname, bool &success)
{
	return (load (fname.mb_str (wxConvUTF8), success));
}

Graph *Graph::load (const char *fname, bool &success)
{
	extern Graph *new_graph;	// in gt-bison.y
	extern std::fstream *yy_gt_fs;
#if 0
	// debug
	extern int yy_gt_debug;
	yy_gt_debug = 1;
#endif
	extern int yy_gt_parse (void);

	std::fstream fs;

	fs.open (fname, std::fstream::in);
	if (!fs.is_open ()) {
		// std::cerr << "*** Couldn't open \"" << fname << "\"\n";
		return 0;
	}

	// We don't want anything skipped
#ifdef USING_FREEBSD
#include <osreldate.h>
#if __FreeBSD_version >= 500035
	fs.setf (std::ios_base::fmtflags (0));
#else
	fs.setf (0);
#endif
#else
	fs.setf (std::ios_base::fmtflags (0));
#endif

	new_graph = new Graph ();
	yy_gt_fs = &fs;
	success = (yy_gt_parse () == 0);

	fs.close ();

	return new_graph;
}

void Graph::save (const wxString &fname) const
{
	save (fname.mb_str (wxConvUTF8));
}

void Graph::save (const char *fname) const
{
	std::fstream fs;

	fs.open (fname, std::fstream::out);
	if (!fs.is_open ()) {
		// std::cerr << "*** Couldn't open \"" << fname << "\"\n";
		throw std::runtime_error ("Couldn't open file.");
	}

	fs << *this;
	fs.close ();
}

wxString Graph::get_tag (const char *tag) const
{
	return get_tag (wxString (tag, wxConvUTF8));
}

wxString Graph::get_tag (const wxString tag) const
{
	std::map<const wxString, wxString>::const_iterator it;

	it = tags.find (tag);
	if (it == tags.end ())
		return wxString (wxT(""));
	return it->second;
}

void Graph::set_tag (const char *tag, wxString value)
{
	set_tag (wxString (tag, wxConvUTF8), value);
}

void Graph::set_tag (const wxString tag, wxString value)
{
	tags[tag] = value;
}

void Graph::clear ()
{
	for (; !E.empty (); E.pop_back ())
		delete E.back ();
	for (; !V.empty (); V.pop_back ())
		delete V.back ();
	Vmap.erase (Vmap.begin (), Vmap.end ());
}

void Graph::add (Edge *e)
{
	if (are_adjacent (e->v, e->w, e->directed)) {
		delete e;
		return;		// fail silently
	}
	e->selected = false;
	e->selection_colour = 0;
	E.push_back (e);
	e->v->hook (e);
	e->w->hook (e);
}

void Graph::add (Vertex *v)
{
	if (v->label.empty () || find (v->label))
		v->label = unique_label ();
	v->selected = false;
	v->selection_colour = 0;
	V.push_back (v);
	Vmap[v->label] = v;
}

void Graph::rename (Vertex *v, const wxString &new_label)
{
	Vmap.erase (v->label);
	v->label = new_label;
	Vmap[v->label] = v;
}

Vertex *Graph::find (const char *label) const
{
	return find (wxString (label, wxConvUTF8));
}

Vertex *Graph::find (const wxString &label) const
{
	std::map<const wxString, Vertex *>::const_iterator it;

	it = Vmap.find (label);
	if (it == Vmap.end ())
		return 0;
	return it->second;
}

Edge *Graph::find (const Vertex *v1, const Vertex *v2, bool dir) const
{
	Graph::e_const_iterator eit;

	if (!dir && (v1->degree () > v2->degree ())) {
		const Vertex *tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	if (!dir) {
		for (eit = v1->e_begin (); eit != v1->e_end (); ++eit) {
			if ((*eit)->incident_to (v2))
				return *eit;
		}
	} else {
		for (eit = v1->e_begin (); eit != v1->e_end (); ++eit) {
			if ((*eit)->v != v1)
				continue;
			if ((*eit)->w == v2)
				return *eit;
		}
	}
	return 0;
}

void Graph::remove (Edge *e)
{
	Graph::e_iterator eit;

	if (e->selected)
		unselect (e);

	e->v->unhook (e);
	e->w->unhook (e);

	for (eit = e_begin (); eit != e_end (); ++eit) {
		if (**eit == *e) {
			E.erase (eit);
			delete e;
			break;
		}
	}
}

void Graph::remove (Vertex *v)
{
	Graph::e_iterator eit;
	Graph::v_iterator vit;

	if (v->selected)
		unselect (v);

	// Remove edges incident to v
	for (eit = v->e_begin (); v->degree (); )
		remove (*eit);

	// Remove v
	Vmap.erase (v->label);
	for (vit = v_begin (); vit != v_end (); ++vit) {
		if (*vit == v) {
			V.erase (vit);
			delete v;
			break;
		}
	}
}

void Graph::select (Edge *e)
{
	if (e->selected)
		return;

	e->selected = true;

	e->next = e_selected_head;
	e_selected_head = e;
}

void Graph::select (Vertex *v)
{
	if (v->selected)
		return;

	v->selected = true;

	v->next = v_selected_head;
	v_selected_head = v;
}

void Graph::unselect (Edge *e)
{
	if (!e->selected)
		return;

	e->selected = false;
	e->selection_colour = 0;

	if (e_selected_head == e)
		e_selected_head = e->next;
	else {
		Edge *prev = e_selected_head;
		while (prev->next != e)
			prev = prev->next;
		prev->next = e->next;
	}
	e->next = 0;
}

void Graph::unselect (Vertex *v)
{
	if (!v->selected)
		return;

	v->selected = false;
	v->selection_colour = 0;

	if (v_selected_head == v)
		v_selected_head = v->next;
	else {
		Vertex *prev = v_selected_head;
		while (prev->next != v)
			prev = prev->next;
		prev->next = v->next;
	}
	v->next = 0;
}

void Graph::unselect_all ()
{
	while (e_selected_head)
		unselect (e_selected_head);
	while (v_selected_head)
		unselect (v_selected_head);
}

wxString Graph::unique_label (int level) const
{
	wxString buf (wxT("A"));
	int i;

	for (i = 0; i < level; ++i)
		buf += wxT("-dup");

	do {
		if (!find (buf))
			return buf;
		buf[0]++;
		//buf.set (0, buf[0] + 1);
	} while (buf[0] <= 'Z');

	// Failed - try next level
	return (unique_label (level + 1));
}

bool Graph::are_adjacent (const Vertex *v1, const Vertex *v2, bool dir) const
{
	Graph::e_const_iterator eit;

	if (!dir && (v1->degree () > v2->degree ())) {
		const Vertex *tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	if (!dir) {
		for (eit = v1->e_begin (); eit != v1->e_end (); ++eit) {
			if ((*eit)->incident_to (v2))
				return true;
		}
	} else {
		for (eit = v1->e_begin (); eit != v1->e_end (); ++eit) {
			if ((*eit)->directed) {
				if (((*eit)->v != v1))
					continue;
				if ((*eit)->w == v2)
					return true;
			} else if ((*eit)->incident_to (v2))
				return true;
		}
	}

	return false;
}

void Graph::identify (Vertex *v1, Vertex *v2)
{
	Edge *e;
	Graph::e_const_iterator eit;

	unselect (v1);
	unselect (v2);

	// Swap vertices, so least-degree vertex is v1
	if (v1->degree () > v2->degree ()) {
		Vertex *tmp = v1;
		v1 = v2;
		v2 = tmp;
	}

	// Remove any edge between them
	while ((e = find (v1, v2, false)))
		remove (e);

	// Now move all edges to v2, then destroy v1
	for (eit = v1->e_begin (); eit != v1->e_end (); ++eit) {
		Vertex *valt;

		valt = v1->opposite (*eit);
		if ((*eit)->directed) {
			if ((*eit)->v == v1) {
				if (are_adjacent (v2, valt, true))
					continue;
				add (new Edge (v2, valt, true));
			} else {
				if (are_adjacent (valt, v2, true))
					continue;
				add (new Edge (valt, v2, true));
			}
		} else {
			if (are_adjacent (valt, v2, false))
				continue;
			add (new Edge (valt, v2));
		}
	}

	remove (v1);
}

// Return a subgraph induced by the marked (mark != 0) vertices
Graph *Graph::subgraph_marked () const
{
	Graph *ret;
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;

	ret = new Graph;
	for (vit = v_begin (); vit != v_end (); ++vit)
		if ((*vit)->mark)
			ret->add (new Vertex (**vit));
	for (eit = e_begin (); eit != e_end (); ++eit) {
		Edge *e = *eit;
		if (e->v->mark && e->w->mark)
			ret->add (new Edge (ret->find (e->v->label),
						ret->find (e->w->label)));
	}

	return ret;
}

Graph *Graph::line_graph () const
{
	Graph *ret = new Graph ();
	Graph::e_const_iterator eit, eit2;
	Graph::v_const_iterator vit;

	for (eit = e_begin (); eit != e_end (); ++eit) {
		Edge *e = *eit;
		//wxString s = e->v->label + "," + e->w->label;
		Vertex *v = new Vertex ("", (e->v->x + e->w->x) / 2,
						(e->v->y + e->w->y) / 2);
		e->mark = (long) v;
		ret->add (v);
	}

	for (vit = v_begin (); vit != v_end (); ++vit) {
		Vertex *v = *vit;
		for (eit = v->e_begin (); eit != v->e_end (); ++eit) {
			Vertex *lg_1 = (Vertex *) (*eit)->mark;
			for (eit2 = eit + 1; eit2 != v->e_end (); ++eit2) {
				Vertex *lg_2 = (Vertex *) (*eit2)->mark;
				if (!are_adjacent (lg_1, lg_2))
					ret->add (new Edge (lg_1, lg_2));
			}
		}
	}

	return ret;
}

Graph *Graph::flattened () const
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	Graph *g = new Graph ();

	// Copy vertices across
	for (vit = v_begin (); vit != v_end (); ++vit)
		g->add (new Vertex (**vit));

	// Now copy edges
	for (eit = e_begin (); eit != e_end (); ++eit) {
		Edge *e = new Edge (	g->find ((*eit)->v->label),
					g->find ((*eit)->w->label),
					false, (*eit)->weight);
		g->add (e);
	}

	return g;
}

Graph::e_iterator Graph::e_begin ()
{
	return E.begin ();
}

Graph::e_iterator Graph::e_end ()
{
	return E.end ();
}

Graph::v_iterator Graph::v_begin ()
{
	return V.begin ();
}

Graph::v_iterator Graph::v_end ()
{
	return V.end ();
}

Graph::e_const_iterator Graph::e_begin () const
{
	return E.begin ();
}

Graph::e_const_iterator Graph::e_end () const
{
	return E.end ();
}

Graph::v_const_iterator Graph::v_begin () const
{
	return V.begin ();
}

Graph::v_const_iterator Graph::v_end () const
{
	return V.end ();
}

Vertex *Graph::operator[] (unsigned int i)
{
	return V[i];
}

const Vertex *Graph::operator[] (unsigned int i) const
{
	return V[i];
}

Graph::tag_iterator Graph::tag_begin () const
{
	return tags.begin ();
}

Graph::tag_iterator Graph::tag_end () const
{
	return tags.end ();
}

unsigned int Graph::order () const
{
	return V.size ();
}

unsigned int Graph::num_edges () const
{
	return E.size ();
}
