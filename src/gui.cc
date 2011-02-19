//
//	gui.cc
//

#include "wx/bitmap.h"
#include "wx/frame.h"
#include "wx/gauge.h"
#include "wx/icon.h"
#include "wx/menu.h"
#include "wx/msgdlg.h"
#include "wx/statusbr.h"
#include "wx/string.h"
#include "wx/toolbar.h"

#include <math.h>
#include "canvas.h"
#include "config.h"
#include "edge.h"
#include "factory.h"
#include "graph.h"
#include "gui.h"
#include "lang.h"
#include "undo.h"
#include "vertex.h"


// Toolbar pixmaps
#include "edge_mode.xpm"
#include "vertex_mode.xpm"

#include  "logo.xpm"


BEGIN_EVENT_TABLE(GTFrame, wxFrame)
	EVT_SIZE	(GTFrame::OnSize)

	EVT_MENU	(ID_FILE_LOAD,		GTFrame::cb_File_Load)
	EVT_MENU	(ID_FILE_SAVE,		GTFrame::cb_File_Save)
	EVT_MENU	(ID_FILE_EXIT,		GTFrame::cb_File_Exit)

	EVT_MENU	(ID_EDIT_UNDO,		GTFrame::cb_Edit_Undo)
	EVT_MENU	(ID_EDIT_SELECTALL,	GTFrame::cb_Edit_SelectAll)
	EVT_MENU	(ID_EDIT_SELECTNONE,	GTFrame::cb_Edit_SelectNone)
	EVT_MENU	(ID_EDIT_INVERTSELV,	GTFrame::cb_Edit_InvertSelectionVertices)
	EVT_MENU	(ID_EDIT_INVERTSELE,	GTFrame::cb_Edit_InvertSelectionEdges)
	EVT_MENU	(ID_EDIT_INVERTSELA,	GTFrame::cb_Edit_InvertSelectionAll)

	EVT_MENU	(ID_VIEW_LABELS,	GTFrame::cb_View)
	EVT_MENU	(ID_VIEW_WEIGHTS,	GTFrame::cb_View)
	EVT_MENU	(ID_VIEW_FLOWS,		GTFrame::cb_View)

	EVT_MENU	(ID_GRAPH_CLEAR,	GTFrame::cb_Graph_Clear)
	EVT_MENU	(ID_GRAPH_COMPLEMENT,	GTFrame::cb_Graph_Complement)
	EVT_MENU	(ID_GRAPH_LINEGRAPH,	GTFrame::cb_Graph_LineGraph)
	EVT_MENU	(ID_GRAPH_SUBGRAPH,	GTFrame::cb_Graph_Subgraph)

	EVT_MENU	(ID_GRAPH_FIND_SHORTESTPATH,	GTFrame::cb_Graph_Find_ShortestPath)
	EVT_MENU	(ID_GRAPH_FIND_BFS,	GTFrame::cb_Graph_Find_BFS)
	EVT_MENU	(ID_GRAPH_FIND_DFS,	GTFrame::cb_Graph_Find_DFS)
	EVT_MENU	(ID_GRAPH_FIND_MST,	GTFrame::cb_Graph_Find_MST)
	EVT_MENU	(ID_GRAPH_FIND_MAXFLOW,	GTFrame::cb_Graph_Find_MaxFlow)

	EVT_MENU	(ID_GRAPH_PROPERTIES_CONNECTIVITY,	GTFrame::cb_Graph_Properties_Connectivity)
	EVT_MENU	(ID_GRAPH_PROPERTIES_EULERICITY,	GTFrame::cb_Graph_Properties_Eulericity)
	EVT_MENU	(ID_GRAPH_PROPERTIES_HAMILTONICITY,	GTFrame::OnMenu)
	EVT_MENU	(ID_GRAPH_PROPERTIES_PLANARITY,		GTFrame::OnMenu)

	EVT_MENU	(ID_GRAPH_STATISTICS_ADJMATRIX,	GTFrame::cb_Graph_Statistics_AdjacencyMatrix)
	EVT_MENU	(ID_GRAPH_STATISTICS_DEGSEQ,	GTFrame::cb_Graph_Statistics_DegreeSequence)
	EVT_MENU	(ID_GRAPH_STATISTICS_DIAMETER,	GTFrame::cb_Graph_Statistics_Diameter)
	EVT_MENU	(ID_GRAPH_STATISTICS_GIRTH,	GTFrame::OnMenu)
	EVT_MENU	(ID_GRAPH_STATISTICS_RADIUS,	GTFrame::cb_Graph_Statistics_Radius)
	EVT_MENU	(ID_GRAPH_STATISTICS_CHROMNUM,	GTFrame::cb_Graph_Statistics_ChromaticNumber)
	EVT_MENU	(ID_GRAPH_STATISTICS_CHROMINDEX,	GTFrame::cb_Graph_Statistics_ChromaticIndex)
	EVT_MENU	(ID_GRAPH_STATISTICS_CHROMPOLY,	GTFrame::cb_Graph_Statistics_ChromaticPolynomial)

	EVT_MENU	(ID_PREFAB_COMPLETE,	GTFrame::cb_Prefab_Complete)
	EVT_MENU	(ID_PREFAB_COMPLETEBIPARTITE,	GTFrame::cb_Prefab_CompleteBipartite)
	EVT_MENU	(ID_PREFAB_CYCLE,	GTFrame::cb_Prefab_Cycle)
	EVT_MENU	(ID_PREFAB_GEAR,	GTFrame::cb_Prefab_Gear)
	EVT_MENU	(ID_PREFAB_HANOI,	GTFrame::cb_Prefab_Hanoi)
	EVT_MENU	(ID_PREFAB_LADDER,	GTFrame::cb_Prefab_Ladder)
	EVT_MENU	(ID_PREFAB_LATTICE,	GTFrame::cb_Prefab_Lattice)
	EVT_MENU	(ID_PREFAB_NULL,	GTFrame::cb_Prefab_Null)
	EVT_MENU	(ID_PREFAB_STAR,	GTFrame::cb_Prefab_Star)
	EVT_MENU	(ID_PREFAB_TREE,	GTFrame::OnMenu)
	EVT_MENU	(ID_PREFAB_WHEEL,	GTFrame::cb_Prefab_Wheel)
	EVT_MENU	(ID_PREFAB_PETERSEN,	GTFrame::cb_Prefab_Petersen)

	EVT_MENU	(ID_PREFAB_PLATONIC_TETRAHEDRAL,	GTFrame::cb_Prefab_Platonic_Tetrahedral)
	EVT_MENU	(ID_PREFAB_PLATONIC_CUBICAL,		GTFrame::cb_Prefab_Platonic_Cubical)
	EVT_MENU	(ID_PREFAB_PLATONIC_OCTAHEDRAL,		GTFrame::cb_Prefab_Platonic_Octahedral)
	EVT_MENU	(ID_PREFAB_PLATONIC_DODECAHEDRAL,	GTFrame::cb_Prefab_Platonic_Dodecahedral)
	EVT_MENU	(ID_PREFAB_PLATONIC_ICOSAHEDRAL,	GTFrame::cb_Prefab_Platonic_Icosahedral)

	EVT_MENU	(ID_HELP_ABOUT,		GTFrame::cb_Help_About)

	EVT_TOOL	(ID_TOOL_VERTEXMODE,	GTFrame::cb_Change_Mode)
	EVT_TOOL	(ID_TOOL_EDGEMODE,	GTFrame::cb_Change_Mode)
    EVT_TOOL	(ID_TOOL_DIREDGEMODE,	GTFrame::cb_Change_Mode)
END_EVENT_TABLE()

wxMenuBar *GTFrame::genMenuBar ()
{
	// File Menu
	wxMenu *menu_file = new wxMenu ();
	menu_file->Append (ID_FILE_LOAD, _("&Load\tCtrl-O"));
	menu_file->Append (ID_FILE_SAVE, _("&Save\tCtrl-S"));
	menu_file->AppendSeparator ();
	menu_file->Append (ID_FILE_EXIT, _("E&xit\tCtrl-Q"));

	// Edit Menu
	wxMenu *menu_edit = new wxMenu ();
	menu_edit->Append (ID_EDIT_UNDO, _("&Undo\tCtrl-Z"));
	menu_edit->FindItem (ID_EDIT_UNDO)->Enable (false);
	menu_edit->AppendSeparator ();
	menu_edit->Append (ID_EDIT_SELECTALL, _("Select &All\tCtrl-A"));
	menu_edit->Append (ID_EDIT_SELECTNONE, _("Select &None"));
	menu_edit->Append (ID_EDIT_INVERTSELV, _("Invert Selection (&Vertices)"));
	menu_edit->Append (ID_EDIT_INVERTSELE, _("Invert Selection (&Edges)"));
	menu_edit->Append (ID_EDIT_INVERTSELA, _("&Invert Selection (All)"));

	// View Menu
	wxMenu *menu_view = new wxMenu ();
	menu_view->AppendCheckItem (ID_VIEW_LABELS, _("&Labels"));
	menu_view->AppendSeparator ();
	menu_view->AppendCheckItem (ID_VIEW_WEIGHTS, _("&Weights"));
	menu_view->AppendCheckItem (ID_VIEW_FLOWS, _("&Flows"));

	// Graph Menu
	wxMenu *menu_graph = new wxMenu ();
	menu_graph->Append (ID_GRAPH_CLEAR, _("&Clear\tCtrl-W"));
	menu_graph->Append (ID_GRAPH_COMPLEMENT, _("C&omplement"));
	menu_graph->Append (ID_GRAPH_LINEGRAPH, _("&Line Graph"));
	menu_graph->Append (ID_GRAPH_SUBGRAPH, _("S&ubgraph"));
	menu_graph->AppendSeparator ();

	// Graph/Find submenu
	wxMenu *menu_graph_find = new wxMenu ();
	menu_graph_find->Append (ID_GRAPH_FIND_SHORTESTPATH,
					_("&Shortest Path"));
	menu_graph_find->Append (ID_GRAPH_FIND_BFS,
					_("&Breadth-First Search"));
	menu_graph_find->Append (ID_GRAPH_FIND_DFS,
					_("&Depth-First Search"));
	menu_graph_find->Append (ID_GRAPH_FIND_MST,
					_("&Minimum Spanning Tree"));
	menu_graph_find->Append (ID_GRAPH_FIND_MAXFLOW,
					_("Maximum &Flow"));
	menu_graph->Append (ID_GRAPH_FIND, _("&Find"), menu_graph_find);

	// Graph/Properties submenu
	wxMenu *menu_graph_prop = new wxMenu ();
	menu_graph_prop->Append (ID_GRAPH_PROPERTIES_CONNECTIVITY,
					_("&Connectivity"));
	menu_graph_prop->Append (ID_GRAPH_PROPERTIES_EULERICITY,
					_("&Eulericity"));
	menu_graph_prop->Append (ID_GRAPH_PROPERTIES_HAMILTONICITY,
					_("&Hamiltonicity"));
	menu_graph_prop->FindItem (ID_GRAPH_PROPERTIES_HAMILTONICITY)->Enable (false);
	menu_graph_prop->Append (ID_GRAPH_PROPERTIES_PLANARITY,
					_("&Planarity"));
	menu_graph_prop->FindItem (ID_GRAPH_PROPERTIES_PLANARITY)->Enable (false);
	menu_graph->Append (ID_GRAPH_PROPERTIES, _("&Properties"), menu_graph_prop);

	// Graph/Statistics submenu
	wxMenu *menu_graph_stat = new wxMenu ();
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_ADJMATRIX,
					_("&Adjacency Matrix"));
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_DEGSEQ,
					_("&Degree Sequence"));
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_DIAMETER,
					_("D&iameter"));
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_GIRTH,
					_("&Girth"));
	menu_graph_stat->FindItem (ID_GRAPH_STATISTICS_GIRTH)->Enable (false);
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_RADIUS,
					_("&Radius"));
	menu_graph_stat->AppendSeparator ();
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_CHROMNUM,
					_("&Chromatic Number"));
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_CHROMINDEX,
					_("C&hromatic Index"));
	menu_graph_stat->Append (ID_GRAPH_STATISTICS_CHROMPOLY,
					_("Chromatic &Polynomial"));
	menu_graph->Append (ID_GRAPH_STATISTICS, _("&Statistics"), menu_graph_stat);

	// Prefab Menu
	wxMenu *menu_prefab = new wxMenu ();
	menu_prefab->Append (ID_PREFAB_COMPLETE, _("Complete (&Kn)"));
	menu_prefab->Append (ID_PREFAB_COMPLETEBIPARTITE,
					_("Complete &Bipartite (Kn,m)"));
	menu_prefab->Append (ID_PREFAB_CYCLE, _("Cycle (&Cn)"));
	menu_prefab->Append (ID_PREFAB_GEAR, _("Gear (&Gn)"));
	menu_prefab->Append (ID_PREFAB_HANOI, _("Hanoi (&Hn)"));
	menu_prefab->Append (ID_PREFAB_LADDER, _("Ladder (&Ln)"));
	menu_prefab->Append (ID_PREFAB_LATTICE, _("Lattice (Ln,m)"));
	menu_prefab->Append (ID_PREFAB_NULL, _("Null (&Nn)"));
	menu_prefab->Append (ID_PREFAB_STAR, _("Star (&Sn)"));
	menu_prefab->Append (ID_PREFAB_TREE, _("Tree (&Tn)"));
	menu_prefab->FindItem (ID_PREFAB_TREE)->Enable (false);
	menu_prefab->Append (ID_PREFAB_WHEEL, _("Wheel (&Wn)"));
	menu_prefab->AppendSeparator ();
	menu_prefab->Append (ID_PREFAB_PETERSEN, _("Petersen"));

	// Prefab/Platonic submenu
	wxMenu *menu_prefab_platonic = new wxMenu ();
	menu_prefab_platonic->Append (ID_PREFAB_PLATONIC_TETRAHEDRAL,
							_("&Tetrahedral"));
	menu_prefab_platonic->Append (ID_PREFAB_PLATONIC_CUBICAL,
							_("&Cubical"));
	menu_prefab_platonic->Append (ID_PREFAB_PLATONIC_OCTAHEDRAL,
							_("&Octahedral"));
	menu_prefab_platonic->Append (ID_PREFAB_PLATONIC_DODECAHEDRAL,
						       	_("&Dodecahedral"));
	menu_prefab_platonic->Append (ID_PREFAB_PLATONIC_ICOSAHEDRAL,
							_("&Icosahedral"));
	menu_prefab->Append (ID_PREFAB_PLATONIC, _("&Platonic"), menu_prefab_platonic);

	// Help Menu
	wxMenu *menu_help = new wxMenu ();
	menu_help->Append (ID_HELP_ABOUT, _("&About\tF1"));

	// Menubar
	wxMenuBar *mb = new wxMenuBar ();
	mb->Append (menu_file, _("&File"));
	mb->Append (menu_edit, _("&Edit"));
	mb->Append (menu_view, _("&View"));
	mb->Append (menu_graph, _("&Graph"));
	mb->Append (menu_prefab, _("&Prefab"));
	mb->Append (menu_help, _("&Help"));
	// TODO: right-align help menu

	return mb;
}

void GTFrame::OnMenu (wxCommandEvent &event)
{
	// TODO
	wxMessageDialog dlg (this, wxT("This isn't implemented yet!"), wxT("NYI."),
						wxOK | wxICON_ERROR);
	dlg.ShowModal ();
}

#include "wx/settings.h"
void GTFrame::OnSize (wxSizeEvent &event)
{
	wxRect rect;

	// Fix up progress bar
	if (!statusBar)
		return;
	statusBar->GetFieldRect (2, rect);

	progressBar->SetSize (rect.x + 2, rect.y + 2, rect.width - 4, rect.height - 4);
	wxSize sz = progressBar->GetSize ();
	progressBar->Move (rect.x + (rect.width - sz.x) / 2,
				rect.y + (rect.height - sz.y) / 2);

	// Fix up canvas
	sz = GetClientSize ();
	int nudge_horiz = wxSystemSettings::GetMetric (wxSYS_VSCROLL_X);
	int nudge_vert = wxSystemSettings::GetMetric (wxSYS_HSCROLL_Y);
	sz.SetWidth (sz.GetWidth () - nudge_horiz);
	sz.SetHeight (sz.GetHeight () - nudge_vert);
	canvas->SetClientSize (sz.GetWidth (), sz.GetHeight ());
	Factory::width = sz.GetWidth ();
	Factory::height = sz.GetHeight ();
}

void GTFrame::setUndoText (wxString description)
{
	wxString s_txt (_("&Undo"));
	if (description != wxT(""))
		s_txt += wxT(" (") + description + wxT(")");
	s_txt += wxT("\tCtrl-Z");
	edit_Undo->SetText (s_txt);
}

GTFrame::GTFrame ()
	: wxFrame ((wxFrame *) NULL, -1, wxT("GraphThing " GT_VERSION),
		wxDefaultPosition, wxSize (600, 500)), statusBar (0)
{
	SetIcon (wxIcon (logo_xpm));

	graph = new Graph ();


	SetMenuBar (genMenuBar ());

	edit_Undo = GetMenuBar ()->FindItem (ID_EDIT_UNDO);
	view_Labels = GetMenuBar ()->FindItem (ID_VIEW_LABELS);
	view_Weights = GetMenuBar ()->FindItem (ID_VIEW_WEIGHTS);
	view_Flows = GetMenuBar ()->FindItem (ID_VIEW_FLOWS);

	view_Labels->Check ();


	wxToolBar *tb = CreateToolBar (wxTB_HORIZONTAL | wxTB_TEXT);

	statusBar = CreateStatusBar (3);
	int widths[] = { 0, -1, -1 };
	statusBar->SetStatusWidths (3, widths);

	progressBar = new wxGauge (statusBar, -1, 100);
	progressBar->SetValue (0);

	wxBitmap *vPix = new wxBitmap (vertex_mode_xpm);
	wxBitmap *ePix = new wxBitmap (edge_mode_xpm);
	tb->AddRadioTool (ID_TOOL_VERTEXMODE, _("Vertex Mode"), *vPix, *vPix,
					_("Change to Vertex Mode"));
	tb->AddRadioTool (ID_TOOL_EDGEMODE, _("Undirected Edge Mode"), *ePix, *ePix,
					_("Change to Undirected Edge Mode"));
    tb->AddRadioTool (ID_TOOL_DIREDGEMODE, _("Directed Edge Mode"), *ePix, *ePix,
					_("Change to Directed Edge Mode"));
	tb->Realize ();

	canvas = new Canvas (this, &graph);

	pushStatus (_("Ready."));
	wxCommandEvent ev (0, ID_TOOL_VERTEXMODE);
	cb_Change_Mode (ev);
	cb_View (ev);
}

GTFrame::~GTFrame ()
{
	delete graph;
}

void GTFrame::undoableAction (wxString description)
{
	UndoStep step (new Graph (*graph), description);
	undoStack.push (step);
	edit_Undo->Enable ();

	setUndoText (description);
}

void GTFrame::loadGraph (const wxString fname)
{
	bool res;

	Graph *g = graph->load (fname, res);
	if (g) {
		// TODO: Make this undoable?
		delete graph;
		graph = g;
	}

	canvas->redraw ();
}

void GTFrame::msg (const wxString &title, const wxString &s)
{
	// TODO: pass through more flags
	wxMessageDialog dlg (this, s, title, wxOK);
	dlg.CentreOnParent ();
	dlg.ShowModal ();
}

void GTFrame::msg (const wxString &title, const char *s)
{
	msg (title, wxString (s, wxConvUTF8));
}

void GTFrame::toggleMode ()
{
	// TODO: toggle the toolbar buttons!
}

void GTFrame::pushStatus (const wxString &str)
{
	statusBar->PushStatusText (str, 1);
}

void GTFrame::popStatus ()
{
	statusBar->PopStatusText (1);
}

void GTFrame::setProgress (double frac)
{
	static double last_frac = -1.0;

	// TODO: implement progress percentage text somehow
	if (frac < 0.0) {
		// hide progress bar
		//progressBar->set_show_text (false);
		progressBar->Hide ();
		last_frac = frac - 1;
	} else if (frac <= 1.0) {
		progressBar->SetValue (int (frac * 100));
		//progressBar->set_format_string ("%p%% complete");
		//progressBar->set_show_text (true);
		progressBar->Show ();
	}

	//progressBar->Refresh ();
	//progressBar->Update ();
}
