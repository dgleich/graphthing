//
//	graph2.cc
//

#include  "wx/string.h"

#include <list>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include "edge.h"
#include "graph.h"
#include "main.h"
#include "math.h"
#include "matrix.h"
#include "polynomial.h"
#include "vertex.h"


bool Graph::is_bridge (Edge *e) const
{
	Graph *tmp = new Graph (*this);
	bool res;

	tmp->remove (tmp->find (tmp->find (e->v->label),
				tmp->find (e->w->label), e->directed));
	res = tmp->is_connected ();

	delete tmp;
	return !res;
}

bool Graph::is_undirected () const
{
	Graph::e_const_iterator eit;

	for (eit = e_begin (); eit != e_end (); ++eit)
		if ((*eit)->directed)
			return false;
	return true;
}

bool Graph::is_connected (bool dir, Vertex *start) const
{
	bool done;
	unsigned int marks;
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;

	if (V.empty ())
		return true;		// null graph is connected
	if (num_edges () < (order () - 1))
		return false;		// not enough edges

	// Unmark all edges and vertices initially
	for (eit = e_begin (); eit != e_end (); ++eit)
		(*eit)->mark = 0;
	for (vit = v_begin (); vit != v_end (); ++vit)
		(*vit)->mark = 0;

	if (!start)
		start = V[0];
	start->mark = 1;

	// Algorithm: Iterate through edge list, marking vertices adjacent
	//	to already-marked vertices, until no more can be marked
	marks = 1;
	do {
		done = true;
		for (eit = e_begin (); eit != e_end (); ++eit) {
			Edge *e = *eit;

			if (e->mark)
				continue;
			if (!e->v->mark && !e->w->mark)
				continue;

			if (!dir || !e->directed) {
				if (e->v->mark != e->w->mark)
					marks++;
				e->v->mark = e->w->mark = e->mark = 1;
				done = false;
			} else {
				if (!e->v->mark && e->w->mark)
					continue;
				marks++;
				e->w->mark = e->mark = 1;
				done = false;
			}
		}
	} while (!done);

	if (marks < order ())
		return false;
	return true;
}

// TODO: this is horribly slow (but at least correct!)
bool Graph::is_strongly_connected () const
{
	Graph::v_const_iterator vit;

	for (vit = v_begin (); vit != v_end (); ++vit) {
		if (!is_connected (true, *vit))
			return false;
	}
	return true;
}

void Graph::eulericity (bool &euler, bool &semi, wxString &tour) const
{
	int cnt;
	Graph::v_const_iterator vit;
	const Vertex *v, *w;
	bool undir;

	euler = false;
	semi = false;

	if (!is_connected ())
		return;

	undir = is_undirected ();
	if (undir) {
		cnt = 0;
		v = w = 0;
		for (vit = v_begin (); vit != v_end (); ++vit) {
			int odd = (*vit)->degree () & 1;
			if (!odd)
				continue;
			cnt += 1;
			if (cnt == 1)
				v = *vit;
			else if (cnt == 2)
				w = *vit;
			if (cnt > 2)
				break;
		}
		if (cnt == 0)		// Eulerian
			euler = true;
		else if (cnt == 2)	// Semi-Eulerian
			semi = true;
	} else {
		cnt = 0;
		v = w = 0;
		for (vit = v_begin (); vit != v_end (); ++vit) {
			int ind = (*vit)->indegree ();
			int outd = (*vit)->outdegree ();
			if (ind != outd) {
				++cnt;
				if (outd == ind + 1) {
					if (v)
						break;
					v = *vit;
				} else if (ind == outd + 1) {
					if (w)
						break;
					w = *vit;
				} else
					break;
			}
		}
		if (cnt == 0)
			euler = true;
		else if ((cnt == 2) && v && w)
			semi = true;
	}

	// Use Fleury's Algorithm
	Graph *red = new Graph (*this);
	std::vector<const Edge *> tour_e;
	std::vector<const Vertex *> tour_v;
	const Vertex *curr_this, *curr_red;

	// We always start with vertex v when semi-Eulerian; otherwise pick any
	curr_this = semi ? v : V[0];
	curr_red = red->find (curr_this->label);
	tour_v.push_back (curr_this);

	while (1) {
		if ((curr_red->degree () == 0) && (curr_red->outdegree () == 0))
			break;
		// pick an edge
		Edge *sel, *fail = 0;
		Graph::e_const_iterator eit, last = curr_red->e_end ();
		for (eit = curr_red->e_begin (); eit != last; ++eit) {
			if ((*eit)->directed && ((*eit)->v != curr_red))
				continue;
			if (red->is_bridge (*eit)) {
				fail = *eit;
				continue;
			}
			if (!(undir || (*eit)->directed)) {
				fail = *eit;
				continue;
			}
			break;
		}
		sel = (eit == last) ? fail : *eit;
		Vertex *dst_red = curr_red->opposite (sel);
		Edge *e = find (curr_this, find (dst_red->label), sel->directed);
		curr_red = dst_red;
		red->remove (sel);
		curr_this = curr_this->opposite (e);
		tour_e.push_back (e);
		tour_v.push_back (curr_this);
	}

	delete red;

	std::vector<const Vertex *>::const_iterator cvit;
	int num = 0;
	for (cvit = tour_v.begin (); cvit != tour_v.end (); ++cvit) {
		if (num > 0)
			tour += wxT(", ");
		++num;
		tour += (*cvit)->label;
	}
}

void Graph::mark_shortest_path (Vertex *v1, Vertex *v2)
{
	Graph::e_const_iterator eit;
	Graph::v_iterator vit;

	// Method: Dijkstra's Algorithm
	//	(vertex->mark is shortest distance from source + 1)
	//	(if vertex->mark is negative, it's an estimate)

	// Mark all vertices as belonging to the V\S set
	for (vit = v_begin (); vit != v_end (); ++vit)
		(*vit)->mark = 0;
	v1->mark = 1;

	while (1) {
		// If every vertex is marked, break
		for (vit = v_begin (); vit != v_end (); ++vit)
			if ((*vit)->mark < 1)
				break;
		if (vit == v_end ())
			break;		// all marked

		// Revise estimates
		for (vit = v_begin (); vit != v_end (); ++vit) {
			Vertex *v = *vit;

			if (v->mark > 0)
				continue;	// already marked
			v->mark = 0;
			eit = v->e_begin ();
			for (; eit != v->e_end (); ++eit) {
				Edge *e = *eit;
				Vertex *alt = v->opposite (e);
				if (e->directed && (v != e->w))
					continue;

				if (alt->mark < 1)
					continue;
				if (((alt->mark + e->weight) < -v->mark) ||
							(v->mark == 0))
					v->mark = -(alt->mark + e->weight);
			}
		}

		// Find smallest estimate, and mark it permanently
		Vertex *min = v1;
		bool none_yet = true;
		int min_val = 0;
		for (vit = v_begin (); vit != v_end (); ++vit) {
			if ((*vit == v1) || ((*vit)->mark >= 0))
				continue;
			if ((min_val > -(*vit)->mark) || none_yet) {
				min = *vit;
				min_val = -min->mark;
				none_yet = false;
			}
		}
		if (min == v1)
			break;		// filled all possible

		// Mark this vertex permanently
		min->mark = -min->mark;

		if (min == v2)
			break;		// cut search short
	}

	// Adjust marks to be correct distances
	for (vit = v_begin (); vit != v_end (); ++vit) {
		Vertex *v = *vit;

		if (v->mark > 0)
			v->mark -= 1;
		else
			v->mark = -1;
	}
}

// TODO: digraph fixes?
int Graph::diameter (bool sel)
{
	int best_dist;
	Vertex *best_src, *best_dst;
	Graph::v_iterator vit1, vit2;

	if (order () < 2)
		return 0;

	best_dist = -1;
	best_src = best_dst = 0;
	for (vit1 = v_begin (); (vit1 + 1) != v_end (); ++vit1) {
		mark_shortest_path (*vit1, 0);
		for (vit2 = vit1 + 1; vit2 != v_end (); ++vit2) {
			if (best_dist < (*vit2)->mark) {
				best_src = *vit1;
				best_dst = *vit2;
				best_dist = best_dst->mark;
			}
		}
	}

	if (best_dist < 0)
		return 0;

	if (!sel)
		return best_dist;

	// Select longest shortest path marked
	mark_shortest_path (best_src, best_dst);
	unselect_all ();
	select (best_dst);
	while (best_dst != best_src) {
		Graph::e_const_iterator eit;
		Vertex *alt = 0;

		eit = best_dst->e_begin ();
		for (; eit != best_dst->e_end (); ++eit) {
			alt = best_dst->opposite (*eit);
			if (best_dst->mark == (alt->mark + (*eit)->weight))
				break;
		}
		if (eit == best_dst->e_end ())
			break;		// Uh, oh!
		select (*eit);
		select (alt);
		best_dst = alt;
	}

	return best_dist;
}

// TODO: digraph fixes?
int Graph::radius (bool sel)
{
	int best_dist, sub_best_dist;
	Vertex *best_src;
	Graph::v_iterator vit1, vit2;

	if (order () < 2)
		return 0;

	best_dist = -1;
	best_src = 0;
	for (vit1 = v_begin (); vit1 != v_end (); ++vit1) {
		mark_shortest_path (*vit1, 0);
		sub_best_dist = -1;
		for (vit2 = v_begin (); vit2 != v_end (); ++vit2) {
			if (sub_best_dist < (*vit2)->mark)
				sub_best_dist = (*vit2)->mark;
		}
		if (sub_best_dist == -1)
			continue;
		if ((best_dist == -1) || (sub_best_dist < best_dist)) {
			best_src = *vit1;
			best_dist = sub_best_dist;
		}
	}

	if (best_dist < 0)
		return 0;

	if (sel) {
		// Select most central vertex
		unselect_all ();
		select (best_src);
	}

	return best_dist;
}

Matrix Graph::adjacency_matrix () const
{
	Matrix ret (order (), order ());
	unsigned int i, j;

	for (j = 0; j < order (); ++j)
		for (i = 0; i < order (); ++i) {
			if (i == j) {
				ret (i, j) = 0;
				continue;
			}
			Edge *e = find (V[i], V[j], true);
			if (!e) {
				e = find (V[i], V[j], false);
				if (e && e->directed && (e->v == V[j]))
					e = NULL;
			}
			ret (i, j) = e ? e->weight : 0;
		}

	return ret;
}

void Graph::bfs (Vertex *v, wxString &s)
{
	Graph::e_const_iterator eit;
	Graph::v_iterator vit;
	std::queue<Vertex *> vq;
	int cnt = 1;

	// Set all marks to 0
	for (vit = v_begin (); vit != v_end (); ++vit)
		(*vit)->mark = 0;

	traversal_visit (v, s, cnt);
	vq.push (v);

	while (!vq.empty ()) {
		v = vq.front ();
		vq.pop ();

		for (eit = v->e_begin (); eit != v->e_end (); ++eit) {
			if ((*eit)->directed && ((*eit)->v != v))
				continue;
			Vertex *opp = v->opposite (*eit);
			if (opp->mark)
				continue;
			traversal_visit (opp, s, cnt);
			vq.push (opp);
		}
	}
}

void Graph::dfs (Vertex *v, wxString &s)
{
	Graph::v_iterator vit;

	// Set all marks to 0
	for (vit = v_begin (); vit != v_end (); ++vit)
		(*vit)->mark = 0;

	dfs_do (v, s, 1);
}

static bool lt_weight (const Edge *a, const Edge *b)
{
	return a->weight < b->weight;
}

struct lt_vertex
{
	bool operator () (const Vertex *v1, const Vertex *v2) const
	{
		return ((v1->x < v2->x) ||
			((v1->x == v2->x) && (v1->y < v2->y)));
	}
};

void Graph::minimum_spanning_tree (std::set<Edge *> &result) const
{
	// Use Kruskal's Algorithm

	result.clear ();
	// set of sets, each set contains a connected component
	std::set< std::set<Vertex *> > comps;
	// list to contain the edges sorted by their weight
	std::list<Edge *> w_edge;

	Graph::v_const_iterator vit;
	Graph::e_const_iterator eit;

	// firstly, the vertices are in separated components
	for (vit = v_begin (); vit != v_end (); ++vit) {
		std::set<Vertex *> tmp;
		tmp.insert (*vit);
		comps.insert (tmp);
	}

	// add all the edges to the edge set
	for (eit = e_begin (); eit != e_end (); ++eit)
		w_edge.push_back (*eit);
	w_edge.sort (lt_weight);

	// loop while not all edges are processed
	while (!w_edge.empty ()) {
		// get edge with the smallest weight
		Edge *current = w_edge.front ();
		// does it straddle two components?
		std::set< std::set<Vertex *> >::iterator sit = comps.begin (),
				foundv = comps.end (), foundw = comps.end ();
		for (; sit != comps.end (); ++sit) {
			if ((*sit).find (current->v) != (*sit).end ())
				foundv = sit;
			if ((*sit).find (current->w) != (*sit).end ())
				foundw = sit;
		}

		if (foundv != foundw) {
			// in different components => add to MST
			std::set<Vertex *> un;
			result.insert (current);
			un.insert (foundv->begin (), foundv->end ());
			un.insert (foundw->begin (), foundw->end ());
			comps.erase (foundv);
			comps.erase (foundw);
			comps.insert (un);
		}

		// done with this edge
		w_edge.pop_front ();
	}
}

//#define FF_DEBUG
// This implementation is based on what was found on p170 of:
//	"Graphs, Algorithms, and Optimization" (Kocay and Kreher)
// It is an augmenting-path implementation of Ford-Fulkerson,
// using a BFS approach to finding shortest augmenting paths.
int Graph::ford_fulkerson (Vertex *src, Vertex *dest)
{
	Graph *g;
	std::map<Vertex *, Vertex *> PrevPt;
	std::map<Vertex *, int> ResCap;

	std::map<Vertex *, Vertex *>::iterator vvit;
	Graph::v_iterator vit;
	Graph::e_iterator eit;
	int total_flow = 0;

	// Handle undirected edges by making a new graph, replacing all the
	// undirected edges with a pair of directed edges (one each way), of
	// the same capacity
	g = new Graph (*this);
	src = g->find (src->label);
	dest = g->find (dest->label);
	std::list<Edge *> undir_e;	// list of undirected edges
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		if (!(*eit)->directed)
			undir_e.push_back (*eit);
	}
	std::list<Edge *>::iterator u_eit;
	for (u_eit = undir_e.begin (); u_eit != undir_e.end (); ++u_eit) {
		Edge *e = *u_eit;
		Vertex *v = e->v, *w = e->w;
		int weight = e->weight;
		g->remove (e);
		g->add (new Edge (v, w, true, weight));
		g->add (new Edge (w, v, true, weight));
	}

	// Usage of vertex->mark: [bitmask]
	//	1 = is in ScanQ (for speeding up lookup)

	// Set all flow to 0
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;
		e->flow = 0;
	}
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		Vertex *v = *vit;
		PrevPt[v] = 0;
		v->mark = 0;
		//ResCap[v] = 0;
	}

	// Compute residual capacity at source vertex
	int max_cap = 0;
	for (eit = src->e_begin (); eit != src->e_end (); ++eit)
		max_cap += (*eit)->weight;


	while (true) {		// search for an augmenting path
		std::queue<Vertex *> ScanQ;

		for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
			(*vit)->mark &= ~1;	// ScanQ is empty
			ResCap[*vit] = 0;	// XXX: check this
		}
		ResCap[src] = max_cap;
		ScanQ.push (src);
		src->mark |= 1;
#ifdef FF_DEBUG
		std::cerr << "FF: Looking for augmenting path...\n";
#endif
		do {
			Vertex *u = ScanQ.front ();
			ScanQ.pop ();
			for (eit = u->e_begin (); eit != u->e_end (); ++eit) {
				Edge *e = *eit;
				Vertex *v = u->opposite (e);
				if (v->mark & 1)	// v is in ScanQ
					continue;
				// v is not in ScanQ

#ifdef FF_DEBUG
				std::cerr << "\tTrying (" << u->label << "," <<
					v->label << "), " << v->label <<
					"->mark=" << v->mark << ", " <<
					" e->flow=" << e->flow << "\n";
#endif

				// Assume directed edge

				if (e->v == u) {	// forward edge
					if (e->weight > e->flow) {
						ScanQ.push (v);
						v->mark |= 1;
						PrevPt[v] = u;

						int del = e->weight - e->flow;
						if (ResCap[u] < del)
							ResCap[v] = ResCap[u];
						else
							ResCap[v] = del;
						if (v == dest)
							goto ford_fulkerson_augment;
					}
				} else {		// backward edge
					if (e->flow > 0) {
						ScanQ.push (v);
						v->mark |= 1;
						PrevPt[v] = u;

						if (ResCap[u] < e->flow)
							ResCap[v] = ResCap[u];
						else
							ResCap[v] = e->flow;
						if (v == dest)
							goto ford_fulkerson_augment;
					}
				}
			}

		} while (!ScanQ.empty ());

ford_fulkerson_all_done:
		// All done!
		for (eit = e_begin (); eit != e_end (); ++eit) {
			Edge *e_orig = *eit;
			Vertex *v_dup = g->find (e_orig->v->label),
				*w_dup = g->find (e_orig->w->label);
			if (e_orig->directed) {
				Edge *e_dup = g->find (v_dup, w_dup, true);
				e_orig->flow = e_dup->flow;
			} else {
				Edge *e1_dup = g->find (v_dup, w_dup, true),
					*e2_dup = g->find (w_dup, v_dup, true);
				int f1 = e1_dup->flow, f2 = e2_dup->flow;
				if (f1 > f2)
					e_orig->flow = f1 - f2;
				else
					e_orig->flow = f2 - f1;
			}
		}
		return total_flow;

ford_fulkerson_augment:
		// AUGMENTFLOW(t):
		{
			Vertex *v = dest;
			Vertex *u = PrevPt[v];
			int delta = ResCap[dest];
#ifdef FF_DEBUG
			std::cerr << "\tAugmentFlow w/ delta=" << delta << "\n";

			std::cerr << "\t" << v->label;
#endif
			if (delta == 0)
				goto ford_fulkerson_all_done;
			while (u) {
#ifdef FF_DEBUG
				std::cerr << " <- " << u->label;
#endif
				Edge *e = g->find (u, v, true);
				if (e)		// forward edge
					e->flow += delta;
				else {			// backward edge
					e = g->find (v, u, true);
					e->flow -= delta;
				}
				v = u;
				u = PrevPt[v];
			}
			total_flow += delta;
#ifdef FF_DEBUG
			std::cerr << ".\n";
#endif
		}
		for (vit = v_begin (); vit != v_end (); ++vit)
			PrevPt[*vit] = 0;
	}
}

int Graph::chromatic_number () const
{
	switch (order ()) {
		case 0:		return -1;	// undefined!
		case 1:		return 1;
	}

	if (num_edges () == 0)
		return 1;

	// try colouring with only a few colours
	{
		// TODO: not an ideal approach...
		Graph g (*this);
		if (g.try_colouring (2))
			return 2;
		if (g.try_colouring (3))
			return 3;
	}
	// Note that the only value that try_colouring is guaranteed to succeed
	// for (when a colouring is possible) is 2. Thus we can only go up to
	// 3, since try_colouring(3) failing does not preclude the graph from
	// being 3-colourable. Once we've retrofitted a better algorithm into
	// try_colouring(), then we may safely increase this number.

	Polynomial p = chromatic_polynomial ();
	unsigned int i;

	for (i = 1; i < order (); ++i)
		if (p.eval (i) != 0)
			return i;
	return i;
}

void Graph::traversal_visit (Vertex *v, wxString &s, int &cnt)
{
	v->mark = cnt++;
	if (cnt > 2)
		s += wxT(", ");
	s += v->label;
}

int Graph::dfs_do (Vertex *v, wxString &s, int cnt)
{
	Graph::e_const_iterator eit;

	traversal_visit (v, s, cnt);

	for (eit = v->e_begin (); eit != v->e_end (); ++eit) {
		if ((*eit)->directed && ((*eit)->v != v))
			continue;
		Vertex *opp = v->opposite (*eit);
		if (!opp->mark)
			cnt = dfs_do (opp, s, cnt);
	}

	return cnt;
}

Polynomial Graph::chromatic_polynomial () const
{
	Graph *g = flattened ();
	Polynomial p;

	p = g->chromatic_poly (0.0, 1.0);
	delete g;
	return p;
}

Polynomial Graph::chromatic_poly (double startP, double endP)
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	unsigned int edge_num, vertex_num;
	bool subtract;

	edge_num = E.size ();
	vertex_num = order ();

	// Some quick, simple cases
	if (vertex_num == 0)
		return Polynomial (1);
	else if (vertex_num == 1)
		return Polynomial (1, 0);	// x
	else if (vertex_num == 2) {
		if (edge_num == 0)
			return Polynomial (1, 0, 0);	// x^2
		else
			return Polynomial (1, -1, 0);	// x^2 - x
	} else if (vertex_num == 3) {
		switch (edge_num) {
			case 0: return Polynomial (1, 0, 0, 0);
			case 1: return Polynomial (1, -1, 0, 0);
			case 2: return Polynomial (1, -2, 1, 0);
			case 3: return Polynomial (1, -3, 2, 0);
		}
	}
	if (edge_num == 0) {
		Polynomial p;
		p[vertex_num] = 1;
		return p;
	}

	// Remove degree 0 vertices
	std::queue<Vertex *> vq;
	int cnt = 0;
	for (vit = v_begin (); vit != v_end (); ++vit)
		if ((*vit)->degree () == 0)
			vq.push (*vit);
	while (!vq.empty ()) {
		remove (vq.front ());
		vq.pop ();
		++cnt;
	}
	if (cnt) {
		Polynomial p;
		p[cnt] = 1;
		return chromatic_poly (startP, endP) * p;
	}

	// Remove degree 1 vertices
	int tot = 0;
	do {
		std::queue<Vertex *> vq;
		cnt = 0;
		for (vit = v_begin (); vit != v_end (); ++vit)
			if ((*vit)->degree () == 1)
				vq.push (*vit);
		while (!vq.empty ()) {
			if (vq.front ()->degree () == 1) {
				remove (vq.front ());
				++cnt;
			}
			vq.pop ();
		}
		tot += cnt;
	} while (cnt > 0);
	if (tot)
		return chromatic_poly (startP, endP)	// XXX: fine tune
				* Polynomial::binomial (-1, tot);

	// Assumptions: |V| >= 4, |E| >= 1

	// Firstly, if the graph is not connected, calculate polynomials
	// for each component, then compute their product
	if (!is_connected ()) {
		Graph gm;		// marked subgraph
		std::queue<Vertex *> vq;	// marked vertices
		std::queue<Edge *> eq;		// marked edges

		// Extract marked subgraph
		for (vit = v_begin (); vit != v_end (); ++vit) {
			if (!(*vit)->mark)
				continue;
			vq.push (*vit);
			gm.add (new Vertex ((*vit)->label, 0, 0));
		}
		for (eit = e_begin (); eit != e_end (); ++eit) {
			if (!(*eit)->mark)
				continue;
			eq.push (*eit);
			gm.add (new Edge (gm.find ((*eit)->v->label),
					gm.find ((*eit)->w->label)));
		}

		while (!eq.empty ()) {
			remove (eq.front ());
			eq.pop ();
		}
		while (!vq.empty ()) {
			remove (vq.front ());
			vq.pop ();
		}

		// XXX: fine-tune
		double weight = (double) order () / (order () + gm.order ());
		double midP = startP + weight * (endP - startP);

		Polynomial p1 = chromatic_poly (startP, midP);
		Polynomial p2 = gm.chromatic_poly (midP, endP);

		return p1 * p2;
	}

	// Assumptions: |V| >= 4, |E| >= 1, connected

	// Is it a tree?
	if (edge_num == (vertex_num - 1)) {
		// binomial expansion: x(x-1)^(v-1)
		return Polynomial (1, 0) *
			Polynomial::binomial (-1, edge_num);
	}

	// Assumptions: |E| >= |V| >= 4, connected, cyclic

	// Special cases for common graphs
	if ((vertex_num == 4) && (edge_num == 4)) {
		// either C4, or irregular
		if ((V[0]->degree () != 2) ||
		    (V[1]->degree () != 2) ||
		    (V[2]->degree () != 2))
			return Polynomial (1, -4, 5, -2, 0);	// irregular
		else
			return Polynomial (1, -4, 6, -3, 0);	// C4
	}
	if ((vertex_num == 4) && (edge_num == 5))
		return Polynomial (1, -5, 8, -4, 0);	// K4 - 1 edge
	if ((vertex_num == 4) && (edge_num == 6))
		return Polynomial (1, -6, 11, -6, 0);	// K4
	if ((vertex_num == 5) && (edge_num == 10))
		return Polynomial (1, -10, 35, -50, 24, 0);	// K5

	// Assumptions: |E| >= |V| >= 5, connected, cyclic

	// Handle complete graphs
	if ((2 * edge_num) == (vertex_num * (vertex_num - 1))) {
		int i, sign = (vertex_num & 1) ? -1 : 1;
		Polynomial p;

		p[vertex_num] = 1;
		for (i = 0; i < (signed) vertex_num; ++i, sign = -sign)
			p[i] = sign * Math::stirling (vertex_num, i);

		return p;
	}

	// Assumptions: |E| >= |V| >= 5, connected, cyclic, not complete

	// Determine which reduction formula to use. For relatively sparse
	// graphs (4*e < (v^2 + v)), use the subtractive, otherwise use
	// the additive. This probably needs some fine tuning.
	subtract = ((4 * edge_num) < (vertex_num * (vertex_num + 1)));

	setProgress (startP);
	double midP = (startP + endP) / 2;

	if (subtract) {
		// P(G,x) = P(G-e,x) - P(G|e,x)

		Graph gm (*this);

		Edge *gme = 0;
		// Find a lightly connected edge
		for (eit = gm.e_begin (); eit != gm.e_end (); ++eit) {
			int wt;
			wt = (*eit)->v->degree () + (*eit)->w->degree ();
			if (!gme || (wt < gme->mark)) {
				gme = *eit;
				gme->mark = wt;
			}
		}

		remove (find (find (gme->v->label), find (gme->w->label)));
		gm.identify (gme->v, gme->w);

		//return (chromatic_poly () - gm.chromatic_poly ());
		Polynomial a = chromatic_poly (startP, midP);
		setProgress (midP);
		Polynomial b = gm.chromatic_poly (midP, endP);
		setProgress (endP);
		return a - b;
	} else {
		// P(G,x) = P(G+e,x) + P(G|e,x)

		Graph gm (*this);

		// Find two non-complete vertices to add an edge to
		Vertex *v1 = 0, *v2 = 0;
		for (vit = v_begin (); vit != v_end (); ++vit) {
			if ((*vit)->degree () >= (vertex_num - 1))
				continue;
			if (!v1)
				v1 = *vit;
			else if (!are_adjacent (v1, *vit)) {
				v2 = *vit;
				break;
			}
		}

		add (new Edge (v1, v2));
		gm.identify (gm.find (v1->label), gm.find (v2->label));

		//return (chromatic_poly () + gm.chromatic_poly ());
		Polynomial a = chromatic_poly (startP, midP);
		setProgress (midP);
		Polynomial b = gm.chromatic_poly (midP, endP);
		setProgress (endP);
		return a + b;
	}

	// Never reached
}

bool Graph::try_colouring (unsigned int colours)
{
	Graph::v_iterator vit;
	Graph::e_iterator eit;

	if (colours < 2)
		return false;

	for (vit = v_begin (); vit != v_end (); ++vit)
		(*vit)->mark = -1;

	// Special case: 2 colours
	if (colours == 2) {
		bool done = false, next_component = true;
		unsigned int marks_made = 0;
		while (!done) {
			done = true;
			for (vit = v_begin (); vit != v_end (); ++vit) {
				Vertex *v = *vit;
				if (v->mark >= 0)
					continue;
				if (next_component) {
					next_component = false;
					v->mark = 0;
					++marks_made;
					continue;
				}
				for (eit = v->e_begin (); eit != v->e_end ();
								++eit) {
					Vertex *v2 = v->opposite (*eit);
					if (v2->mark < 0)
						continue;
					v->mark = 1 - v2->mark;
					++marks_made;
					done = false;
					break;
				}
			}
			if (done && (marks_made < order ())) {
				done = false;
				next_component = true;
			}
		}
		return check_colouring (colours);
	}

	// Try the greedy algorithm, with some small modifications:
	//	- the next vertex to colour will be the one with the least
	//		number of uncoloured neighbours
	// TODO:
	//	* don't assume a connected graph!
	bool done = false, next_component = true, failed = false;
	bool *can_use = new bool[colours];
	while (!done) {
		// Find vertex with the least number of uncoloured neighbours,
		// but with still 1 coloured neighbour
		Vertex *best_v = 0;
		unsigned int uncol_neigh = order ();
		for (vit = v_begin (); vit != v_end (); ++vit) {
			Vertex *v = *vit;
			unsigned int num_col = 0, num_uncol = 0;
			if (v->mark >= 0)
				continue;
			if (next_component) {
				best_v = v;
				next_component = false;
				break;
			}
			for (eit = v->e_begin (); eit != v->e_end (); ++eit) {
				Vertex *valt = v->opposite (*eit);
				if (valt->mark < 0)
					++num_uncol;
				else
					++num_col;
			}
			if (num_col < 1)
				continue;
			if (num_uncol < uncol_neigh) {
				best_v = v;
				uncol_neigh = num_uncol;
			}
		}
		if (!best_v) {
			if (next_component) {
				done = true;
				break;		// no more components to colour
			}
			next_component = true;
			continue;
		}
		//std::cout << "Colouring " << best_v->label << "\n";
		unsigned int i;
		for (i = 0; i < colours; ++i)
			can_use[i] = true;
		for (eit = best_v->e_begin (); eit != best_v->e_end (); ++eit) {
			Vertex *valt = best_v->opposite (*eit);
			if (valt->mark >= 0)
				can_use[valt->mark] = false;
		}
		for (i = 0; i < colours; ++i)
			if (can_use[i]) {
				best_v->mark = i;
				break;
			}
		if (best_v->mark < 0) {
			// Uh, oh! Failed!
			failed = true;
			break;
		}
	}
	delete can_use;

	if (failed)
		return false;
	return check_colouring (colours);
}

bool Graph::check_colouring (unsigned int colours) const
{
	Graph::e_const_iterator eit;

	for (eit = e_begin (); eit != e_end (); ++eit) {
		if ((*eit)->v->mark == (*eit)->w->mark)
			return false;
	}
	return true;
}
