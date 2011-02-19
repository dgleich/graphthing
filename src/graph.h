//
//	graph.h
//

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "wx/string.h"

#include <iostream>
#include <iterator>
#include <map>
#include <set>
#include <vector>
#include "matrix.h"


class Edge;
class Polynomial;
class Vertex;


class Graph
{
private:
	std::vector<Edge *> E;
	std::vector<Vertex *> V;
	std::map<const wxString, wxString> tags;

	std::map<const wxString, Vertex *> Vmap;

	void reindex ();
public:

	typedef std::vector<Edge *>::iterator e_iterator;
	typedef std::vector<Vertex *>::iterator v_iterator;
	typedef std::vector<Edge *>::const_iterator e_const_iterator;
	typedef std::vector<Vertex *>::const_iterator v_const_iterator;

	typedef std::map<const wxString, wxString>::const_iterator tag_iterator;


	Edge *e_selected_head;
	Vertex *v_selected_head;

	Graph ();
	Graph (const Graph &other);
	~Graph ();

	friend std::ostream &operator<< (std::ostream &o, const Graph &g);

	static Graph *load (const wxString &fname, bool &success);
	static Graph *load (const char *fname, bool &success);
	void save (const wxString &fname) const;
	void save (const char *fname) const;

	wxString get_tag (const char *tag) const;
	wxString get_tag (const wxString tag) const;
	void set_tag (const char *tag, const wxString value);
	void set_tag (const wxString tag, const wxString value);

	void clear ();
	void add (Edge *e);
	void add (Vertex *v);
	void rename (Vertex *v, const wxString &new_label);
	Vertex *find (const char *label) const;
	Vertex *find (const wxString &label) const;
	Edge *find (const Vertex *v1, const Vertex *v2, bool dir = false) const;
	void remove (Edge *e);
	void remove (Vertex *v);
	void select (Edge *e);
	void select (Vertex *v);
	void unselect (Edge *e);
	void unselect (Vertex *v);
	void unselect_all ();

	wxString unique_label (int level = 0) const;
	bool are_adjacent (const Vertex *v1, const Vertex *v2,
							bool dir = false) const;
	void identify (Vertex *v1, Vertex *v2);
	Graph *subgraph_marked () const;
	Graph *line_graph () const;
	Graph *flattened () const;


	e_iterator e_begin ();
	e_iterator e_end ();
	v_iterator v_begin ();
	v_iterator v_end ();
	e_const_iterator e_begin () const;
	e_const_iterator e_end () const;
	v_const_iterator v_begin () const;
	v_const_iterator v_end () const;
	Vertex *operator[] (unsigned int i);
	const Vertex *operator[] (unsigned int i) const;

	tag_iterator tag_begin () const;
	tag_iterator tag_end () const;

	unsigned int order () const;
	unsigned int num_edges () const;


	//*********************************************
	// The following methods are all in graph2.cc *
	//*********************************************


private:
	bool is_bridge (Edge *e) const;
public:
	bool is_undirected () const;
	bool is_connected (bool dir = false, Vertex *start = 0) const;
	bool is_strongly_connected () const;
	void eulericity (bool &euler, bool &semi, wxString &tour) const;
	void mark_shortest_path (Vertex *v1, Vertex *v2);
	int diameter (bool sel = false);
	int radius (bool sel = false);
	Matrix adjacency_matrix () const;

	void bfs (Vertex *v, wxString &s);
	void dfs (Vertex *v, wxString &s);

	void minimum_spanning_tree (std::set<Edge *> &result) const;

	int ford_fulkerson (Vertex *src, Vertex *dest);

	int chromatic_number () const;
	Polynomial chromatic_polynomial () const;
	bool try_colouring (unsigned int colours);

private:
	void traversal_visit (Vertex *v, wxString &s, int &cnt);
	int dfs_do (Vertex *v, wxString &s, int cnt);
	bool check_colouring (unsigned int colours) const;

	// Destructive!
	Polynomial chromatic_poly (double startP, double endP);
};


#endif	// __GRAPH_H__
