//
//	gui_cb.cc
//

#include "wx/event.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/numdlg.h"
#include "wx/string.h"

#include <set>
#include <sstream>
#include <vector>
#include "aboutbox.h"
#include "canvas.h"
#include "config.h"
#include "edge.h"
#include "exporter.h"
#include "factory.h"
#include "fancyfileselection.h"
#include "graph.h"
#include "gui.h"
#include "lang.h"
#include "matrix.h"
#include "matrixdialog.h"
#include "paramdialog.h"
#include "polynomial.h"
#include "undo.h"
#include "vertex.h"


void GTFrame::cb_File_Load (wxCommandEvent &event)
{
	FancyFileSelection fs (this, _("Load File"), wxOPEN);
	bool res;

	if (fs.ShowModal () == wxID_CANCEL)
		return;

	Graph *g = graph->load (fs.GetFileName (), res);
	if (!g) {
		msg (_("Loading failed"), _("Loading failed"));
		return;
	}
	if (!res)
		msg (_("Warning"), _("WARNING:\n\n"
			"The loading partially failed;\n"
			"this graph may be incomplete."));

	undoableAction (_("Load graph"));
	delete graph;
	graph = g;

	canvas->redraw ();
}

void GTFrame::cb_File_Save (wxCommandEvent &event)
{
	FancyFileSelection fs (this, _("Save File"), wxSAVE, true);

	if (fs.ShowModal () == wxID_CANCEL)
		return;

	// Put in creator tag if omitted
	if (graph->get_tag ("creator").IsEmpty ())
		graph->set_tag ("creator", wxT("GraphThing " GT_VERSION));

	Exporter *exp = Exporter::make (fs.GetFileType (), fs.GetFileName ());
	bool labels = view_Labels->IsChecked ();
	bool weights = view_Weights->IsChecked ();
	exp->output (graph, labels, weights);
	delete exp;
}

void GTFrame::cb_File_Exit (wxCommandEvent &event)
{
	Close ();
}

void GTFrame::cb_Edit_Undo (wxCommandEvent &event)
{
	if (undoStack.empty ())
		return;
	delete graph;
	graph = undoStack.pop ().getGraph ();
	canvas->redraw ();
	if (undoStack.empty ()) {
		edit_Undo->Enable (false);
		setUndoText (wxT(""));
	} else
		setUndoText (undoStack.top ().getMessage ());
}

void GTFrame::cb_Edit_SelectAll (wxCommandEvent &event)
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;

	for (eit = graph->e_begin (); eit != graph->e_end (); ++eit)
		graph->select (*eit);
	for (vit = graph->v_begin (); vit != graph->v_end (); ++vit)
		graph->select (*vit);

	canvas->redraw ();
}

void GTFrame::cb_Edit_SelectNone (wxCommandEvent &event)
{
	graph->unselect_all ();
	canvas->redraw ();
}

void GTFrame::cb_Edit_InvertSelectionVertices (wxCommandEvent &event)
{
	Graph::v_const_iterator vit;

	for (vit = graph->v_begin (); vit != graph->v_end (); ++vit) {
		if ((*vit)->selected)
			graph->unselect (*vit);
		else
			graph->select (*vit);
	}

	canvas->redraw ();
}

void GTFrame::cb_Edit_InvertSelectionEdges (wxCommandEvent &event)
{
	Graph::e_const_iterator eit;

	for (eit = graph->e_begin (); eit != graph->e_end (); ++eit) {
		if ((*eit)->selected)
			graph->unselect (*eit);
		else
			graph->select (*eit);
	}

	canvas->redraw ();
}

void GTFrame::cb_Edit_InvertSelectionAll (wxCommandEvent &event)
{
	cb_Edit_InvertSelectionVertices (event);
	cb_Edit_InvertSelectionEdges (event);
}

void GTFrame::cb_View (wxCommandEvent &event)
{
	// Flows are only enabled when weights are
	view_Flows->Enable (view_Weights->IsChecked ());

	canvas->setParam (view_Labels->IsChecked (),
			view_Weights->IsChecked (),
			view_Flows->IsChecked ());
	canvas->redraw ();
}

void GTFrame::cb_Graph_Clear (wxCommandEvent &event)
{
	undoableAction (_("Clear graph"));
	graph->clear ();
	canvas->redraw ();
}

void GTFrame::cb_Graph_Complement (wxCommandEvent &event)
{
	Graph *orig = graph;
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit, vit2;

	undoableAction (_("Complement graph"));
	graph = new Graph;
	for (vit = orig->v_begin (); vit != orig->v_end (); ++vit)
		graph->add (new Vertex (**vit));
	// Method: create a new complete graph, then remove the edges that were
	//	in the original. TODO: speed this up!
	for (vit = graph->v_begin (); vit != graph->v_end (); ++vit)
		for (vit2 = vit + 1; vit2 != graph->v_end (); ++vit2)
			graph->add (new Edge (*vit, *vit2));
	for (eit = orig->e_begin (); eit != orig->e_end (); ++eit) {
		Vertex *v1, *v2;
		v1 = graph->find ((*eit)->v->label);
		v2 = graph->find ((*eit)->w->label);
		graph->remove (graph->find (v1, v2));
	}

	delete orig;
	canvas->redraw ();
}

void GTFrame::cb_Graph_LineGraph (wxCommandEvent &event)
{
	undoableAction (_("Line Graph"));
	Graph *orig = graph;
	graph = orig->line_graph ();
	delete orig;
	canvas->redraw ();
}

void GTFrame::cb_Graph_Subgraph (wxCommandEvent &event)
{
	Graph *orig = graph;
	Graph::v_const_iterator vit;

	if (!graph->v_selected_head) {
		msg (_("Subgraph"), _("At least one vertex must be selected "
						"to induce a subgraph."));
		return;
	}

	undoableAction (_("Induced subgraph"));

	// Mark selection
	for (vit = graph->v_begin (); vit != graph->v_end (); ++vit) {
		Vertex *v = *vit;
		v->mark = v->selected ? 1 : 0;
	}

	graph = orig->subgraph_marked ();
	delete orig;
	canvas->redraw ();
}

void GTFrame::cb_Graph_Find_ShortestPath (wxCommandEvent &event)
{
	Vertex *v1, *v2, *walk;
	Graph::e_const_iterator eit;
	long tot;

	v2 = graph->v_selected_head;
	if (!v2 || !v2->next || v2->next->next) {
		msg (_("Shortest Path"), _("Exactly two vertices must be "
				"selected to find a shortest path."));
		return;
	}
	v1 = v2->next;

	pushStatus (_("Finding shortest path..."));

	graph->mark_shortest_path (v1, v2);

	// Select shortest path
	tot = v2->mark;
	graph->unselect_all ();
	graph->select (v2);
	walk = v2;
	while (walk != v1) {
		Edge *e = 0;
		Vertex *alt = 0;

		for (eit = walk->e_begin (); eit != walk->e_end (); ++eit) {
			e = *eit;
			if (e->selected)
				continue;
			alt = walk->opposite (e);
			if (walk->mark == (alt->mark + e->weight))
				break;
		}
		if (eit == walk->e_end ())
			break;		// Uh, oh!
		graph->select (e);
		graph->select (alt);
		walk = alt;
	}

	if (tot < 0) {
		// If there is no path at all,
		// leave original vertices selected
		graph->unselect_all ();
		graph->select (v1);
		graph->select (v2);
	}

	canvas->redraw ();

	if (tot >= 0) {
		wxString str = wxString::Format (_("Shortest path is %i."), tot);
		msg (_("Shortest Path"), str);
	} else
		msg (_("Shortest Path"), _("No path found."));

	popStatus ();
}

void GTFrame::cb_Graph_Find_BFS (wxCommandEvent &event)
{
	Vertex *v;

	v = graph->v_selected_head;
	if (!v || v->next) {
		msg (_("Breadth-First Search"), _("Exactly one vertex "
					"must be selected to perform a "
					"Breadth-First Search."));
		return;
	}

	pushStatus (_("Performing BFS..."));

	wxString m (wxT("BFS:\n\n"));
	graph->bfs (v, m);

	msg (_("Breadth-First Search"), m);

	popStatus ();
}

void GTFrame::cb_Graph_Find_DFS (wxCommandEvent &event)
{
	Vertex *v;

	v = graph->v_selected_head;
	if (!v || v->next) {
		msg (_("Depth-First Search"), _("Exactly one vertex must "
			"be selected to perform a Depth-First Search."));
		return;
	}

	pushStatus (_("Performing DFS..."));

	wxString m (wxT("DFS:\n\n"));
	graph->dfs (v, m);

	msg (_("Depth-First Search"), m);

	popStatus ();
}

void GTFrame::cb_Graph_Find_MST (wxCommandEvent &event)
{
	std::set<Edge *> spanning_tree;
	std::set<Edge *>::const_iterator it;

	pushStatus (_("Finding Minimum Spanning Tree..."));

	graph->minimum_spanning_tree (spanning_tree);
	graph->unselect_all ();
	for (it = spanning_tree.begin (); it != spanning_tree.end (); ++it) {
		graph->select (*it);
		graph->select ((*it)->v);
		graph->select ((*it)->w);
	}
	canvas->redraw ();

	popStatus ();
}

void GTFrame::cb_Graph_Find_MaxFlow (wxCommandEvent &event)
{
	Vertex *v1, *v2;

	v2 = graph->v_selected_head;
	if (!v2 || !v2->next || v2->next->next) {
		msg (_("Maximum Flow"), _("Exactly two vertices must be "
				"selected to find a maximum flow."));
		return;
	}
	v1 = v2->next;

	pushStatus (_("Finding Maximum Flow..."));

	int flow = graph->ford_fulkerson (v1, v2);
#if 0
	graph->unselect_all ();
	for (it = spanning_tree.begin (); it != spanning_tree.end (); ++it) {
		graph->select (*it);
		graph->select ((*it)->v);
		graph->select ((*it)->w);
	}
#endif
	wxString str = wxString::Format (_("Maximum flow is %i."), flow);
	canvas->redraw ();

	msg (_("Maximum Flow"), str);

	popStatus ();
}

void GTFrame::cb_Graph_Properties_Connectivity (wxCommandEvent &event)
{
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	bool undir, conn, strong_conn = false;

	pushStatus (_("Testing connectivity..."));

	undir = graph->is_undirected ();

	conn = graph->is_connected ();
	if (conn && !undir)
		strong_conn = graph->is_strongly_connected ();
	if (conn) {
		if (undir)
			msg (_("Connectivity"), _("Graph is connected."));
		else if (!strong_conn)
			msg (_("Connectivity"), _("Graph is weakly connected."));
		else
			msg (_("Connectivity"), _("Graph is strongly connected."));
	} else {
		// Select marked vertices and edges
		graph->unselect_all ();
		for (eit = graph->e_begin (); eit != graph->e_end (); ++eit)
			if ((*eit)->mark)
				graph->select (*eit);
		for (vit = graph->v_begin (); vit != graph->v_end (); ++vit)
			if ((*vit)->mark)
				graph->select (*vit);
		canvas->redraw ();

		msg (_("Connectivity"), _("Graph is not connected."));
	}

	popStatus ();
}

void GTFrame::cb_Graph_Properties_Eulericity (wxCommandEvent &event)
{
	bool euler, semi;

	pushStatus (_("Determining Eulericity..."));

	wxString tour (wxT("\n("));
	graph->eulericity (euler, semi, tour);
	tour += wxT(")");

	if (euler)
		msg (_("Eulericity"), _("Graph is Eulerian.") + tour);
	else if (semi)
		msg (_("Eulericity"), _("Graph is Semi-Eulerian.") + tour);
	else
		msg (_("Eulericity"), _("Graph is neither Eulerian nor "
						"Semi-Eulerian."));

	popStatus ();
}

void GTFrame::cb_Graph_Statistics_AdjacencyMatrix (wxCommandEvent &event)
{
	MatrixDialog md (this, _("Adjacency Matrix"),
				graph->adjacency_matrix (), *graph);
	md.ShowModal ();
}

void GTFrame::cb_Graph_Statistics_DegreeSequence (wxCommandEvent &event)
{
	std::vector<int> seq;
	std::vector<int>::iterator it1, it2;
	Graph::v_const_iterator vit;

	if (graph->order () < 1) {
		wxString str = _("Degree Sequence");
		str += wxT(":\n\n");
		str += _("(none)");
		msg (_("Degree Sequence"), str);
		return;
	}

	for (vit = graph->v_begin (); vit != graph->v_end (); ++vit)
		seq.push_back ((*vit)->degree ());

	// boring bubble sort
	for (it1 = seq.begin (); (it1 + 1) != seq.end (); ++it1)
	for (it2 = it1 + 1; it2 != seq.end (); ++it2) {
		if (*it1 < *it2) {
			int tmp = *it1;
			*it1 = *it2;
			*it2 = tmp;
		}
	}

	wxString str = _("Degree Sequence");
	str += wxT(":\n\n");

	for (it1 = seq.begin (); it1 != seq.end (); ++it1) {
		if (it1 == seq.begin ())
			str += wxString::Format (wxT("%i"), *it1);
		else
			str += wxString::Format (wxT(", %i"), *it1);
	}

	msg (_("Degree Sequence"), str);
}

void GTFrame::cb_Graph_Statistics_Diameter (wxCommandEvent &event)
{
	pushStatus (_("Determining diameter..."));

	int diam = graph->diameter (true);
	wxString str = wxString::Format (wxT("diam(G) = %i"), diam);
	canvas->redraw ();

	msg (_("Diameter"), str);

	popStatus ();
}

void GTFrame::cb_Graph_Statistics_Radius (wxCommandEvent &event)
{
	pushStatus (_("Determining radius..."));

	int rad = graph->radius (true);
	wxString str = wxString::Format (wxT("rad(G) = %i"), rad);
	canvas->redraw ();

	msg (_("Radius"), str);

	popStatus ();
}

void GTFrame::cb_Graph_Statistics_ChromaticNumber (wxCommandEvent &event)
{
	pushStatus (_("Computing chromatic number..."));

	int chi = graph->chromatic_number ();

	graph->unselect_all ();
	while (chi >= 2) {		// WARNING: Abuse of loop construct!
		if (!graph->try_colouring (chi))
			break;
		Graph::v_iterator vit;
		for (vit = graph->v_begin (); vit != graph->v_end (); ++vit) {
			graph->select (*vit);
			(*vit)->selection_colour = (*vit)->mark;
		}
		break;
	}
	// TODO: mention something if we failed to colour properly!

	canvas->redraw ();
	wxString str = wxString::Format(wxT("chi(G) = %i"), chi);
	msg (_("Chromatic Number"), str);

	setProgress (-1);
	popStatus ();
}

void GTFrame::cb_Graph_Statistics_ChromaticIndex (wxCommandEvent &event)
{
	pushStatus (_("Computing chromatic index..."));

	Graph *lg = graph->line_graph ();
	int chi = lg->chromatic_number ();

	graph->unselect_all ();
	while (chi >= 2) {		// WARNING: Abuse of loop construct!
		if (!lg->try_colouring (chi))
			break;
		Graph::e_iterator eit;
		for (eit = graph->e_begin (); eit != graph->e_end (); ++eit) {
			Vertex *v = (Vertex *) (*eit)->mark;
			graph->select (*eit);
			(*eit)->selection_colour = v->mark;
		}
		break;
	}
	// TODO: mention something if we failed to colour properly!
	delete lg;

	canvas->redraw ();
	wxString str = wxString::Format (wxT("chi'(G) = %i"), chi);
	msg (_("Chromatic Index"), str);

	setProgress (-1);
	popStatus ();
}

void GTFrame::cb_Graph_Statistics_ChromaticPolynomial (wxCommandEvent &event)
{
	wxString m = _("Chromatic Polynomial");

	pushStatus (_("Computing chromatic polynomial..."));

	Polynomial p = graph->chromatic_polynomial ();

	m << wxT(":\n\n") << p.str ();
	msg (_("Chromatic Polynomial"), m);

	setProgress (-1);
	popStatus ();
}

void GTFrame::cb_Prefab_Complete (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Complete:"),
			_("Prefab Parameter"), 5, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::K (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_CompleteBipartite (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	ParamDialogIntInt dlg (this, _("Prefab Parameter"),
			_("Complete Bipartite:"), 3, 1, 20, 3, 1, 20);
	if (dlg.ShowModal () == wxID_CANCEL) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::K (dlg.GetValue1 (), dlg.GetValue2 ());
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Cycle (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Cycle:"),
			_("Prefab Parameter"), 5, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::C (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Gear (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Gear:"),
			_("Prefab Parameter"), 5, 3, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::G (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Hanoi (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Hanoi:"),
			_("Prefab Parameter"), 3, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::H (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Ladder (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Ladder:"),
			_("Prefab Parameter"), 4, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::L (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Lattice (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	ParamDialogIntInt dlg (this, _("Prefab Parameter"),
			_("Lattice:"), 4, 1, 20, 4, 1, 20);
	if (dlg.ShowModal () == wxID_CANCEL) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Lattice (dlg.GetValue1 (), dlg.GetValue2 ());
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Null (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Null:"),
			_("Prefab Parameter"), 3, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::N (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Star (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Star:"),
			_("Prefab Parameter"), 5, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::S (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Wheel (wxCommandEvent &event)
{
	pushStatus (_("Prefab..."));
	int param = wxGetNumberFromUser (wxT(""), _("Wheel:"),
			_("Prefab Parameter"), 5, 1, 20, this);
	if (param < 0) {
		popStatus ();
		return;
	}

	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::W (param);
	canvas->redraw ();
	popStatus ();
}

void GTFrame::cb_Prefab_Petersen (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Petersen ();
	canvas->redraw ();
}

void GTFrame::cb_Prefab_Platonic_Tetrahedral (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Tetrahedral ();
	canvas->redraw ();
}

void GTFrame::cb_Prefab_Platonic_Cubical (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Cubical ();
	canvas->redraw ();
}

void GTFrame::cb_Prefab_Platonic_Octahedral (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Octahedral ();
	canvas->redraw ();
}

void GTFrame::cb_Prefab_Platonic_Dodecahedral (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Dodecahedral ();
	canvas->redraw ();
}

void GTFrame::cb_Prefab_Platonic_Icosahedral (wxCommandEvent &event)
{
	undoableAction (_("Make prefab"));
	delete graph;
	graph = Factory::Icosahedral ();
	canvas->redraw ();
}

void GTFrame::cb_Help_About (wxCommandEvent &event)
{
	pushStatus (_("About GraphThing"));
	AboutBox dlg (this);
	dlg.CentreOnParent ();
	dlg.ShowModal ();

	popStatus ();
}

void GTFrame::cb_Change_Mode (wxCommandEvent &event)
{
	popStatus ();

	in_vertex_mode = (event.GetId() == ID_TOOL_VERTEXMODE);
	canvas->setVertexMode (in_vertex_mode);

	if (in_vertex_mode)
		pushStatus (_("Ready (Vertex Mode)."));
	else
		pushStatus (_("Ready (Edge Mode)."));


	// TODO: depress the correct buttons (mainly for keyboard mode switches)
	//Gtk::Toolbar_Helpers::ToolList &tl = toolbar->tools ();
	//Gtk::Toolbar_Helpers::Tool *tool = tl[1];
	//Gtk::Widget *widget = tool->get_widget (),
	//		*content = tool->get_content ();
	//std::cerr << Gtk::RadioButton::isA (widget) << "\n";
	//std::cerr << Gtk::Pixmap::isA (content) << "\n";
}
