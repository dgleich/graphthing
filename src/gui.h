//
//	gui.h
//

#ifndef __GUI_H__
#define __GUI_H__

#include "wx/frame.h"
#include "wx/string.h"

#include <vector>
#include "matrix.h"
#include "undo.h"


class Canvas;	// see canvas.h
class Graph;
class Edge;
class Vertex;

class wxGauge;
class wxMenuItem;
class wxStatusBar;

class GTFrame : public wxFrame
{
private:
	Canvas *canvas;
	Graph *graph;

	UndoStack undoStack;
	wxMenuItem *edit_Undo, *view_Labels, *view_Weights, *view_Flows;

	wxGauge *progressBar;
	wxStatusBar *statusBar;

	DECLARE_EVENT_TABLE()

	void OnMenu (wxCommandEvent &event);
	void OnSize (wxSizeEvent &event);

	// All callbacks are implemented in gui_cb.cc
#define CB(func)	void func (wxCommandEvent &event);
	CB(cb_File_Load)
	CB(cb_File_Save)
	CB(cb_File_Exit)

	CB(cb_Edit_Undo)
	CB(cb_Edit_SelectAll)
	CB(cb_Edit_SelectNone)
	CB(cb_Edit_InvertSelectionVertices)
	CB(cb_Edit_InvertSelectionEdges)
	CB(cb_Edit_InvertSelectionAll)

	CB(cb_View)

	CB(cb_Graph_Clear)
	CB(cb_Graph_Complement)
	CB(cb_Graph_LineGraph)
	CB(cb_Graph_Subgraph)
	CB(cb_Graph_Find_ShortestPath)
	CB(cb_Graph_Find_BFS)
	CB(cb_Graph_Find_DFS)
	CB(cb_Graph_Find_MST)
	CB(cb_Graph_Find_MaxFlow)
	CB(cb_Graph_Properties_Connectivity)
	CB(cb_Graph_Properties_Eulericity)

	CB(cb_Graph_Statistics_AdjacencyMatrix)
	CB(cb_Graph_Statistics_DegreeSequence)
	CB(cb_Graph_Statistics_Diameter)
	CB(cb_Graph_Statistics_Radius)
	CB(cb_Graph_Statistics_ChromaticNumber)
	CB(cb_Graph_Statistics_ChromaticIndex)
	CB(cb_Graph_Statistics_ChromaticPolynomial)

	CB(cb_Prefab_Complete)
	CB(cb_Prefab_CompleteBipartite)
	CB(cb_Prefab_Cycle)
	CB(cb_Prefab_Gear)
	CB(cb_Prefab_Hanoi)
	CB(cb_Prefab_Ladder)
	CB(cb_Prefab_Lattice)
	CB(cb_Prefab_Null)
	CB(cb_Prefab_Star)
	CB(cb_Prefab_Wheel)
	CB(cb_Prefab_Petersen)
	CB(cb_Prefab_Platonic_Tetrahedral)
	CB(cb_Prefab_Platonic_Cubical)
	CB(cb_Prefab_Platonic_Octahedral)
	CB(cb_Prefab_Platonic_Dodecahedral)
	CB(cb_Prefab_Platonic_Icosahedral)

	CB(cb_Help_About)

	// Split mode
	enum {EDIT_MODE_VERTEX=0, 
        EDIT_MODE_UNDIR_EDGE, EDIT_MODE_DIR_EDGE, EDIT_MODE_END} edit_mode;
	CB(cb_Change_Mode)


	wxMenuBar *genMenuBar ();
	void setUndoText (wxString description);

public:
	GTFrame ();
	~GTFrame ();

	void undoableAction (wxString description);
	void loadGraph (const wxString fname);
	void msg (const wxString &title, const wxString &s);
	void msg (const wxString &title, const char *s);

	void toggleMode ();

	void pushStatus (const wxString &str);
	void popStatus ();
	void setProgress (double frac);		// argument in [0, 1]
};

// Event IDs for main frame
enum {
	ID_FILE_LOAD = 1001,
	ID_FILE_SAVE,
	ID_FILE_PRINT,
	ID_FILE_EXIT,
	
	ID_EDIT_UNDO,
	ID_EDIT_SELECTALL,
	ID_EDIT_SELECTNONE,
	ID_EDIT_INVERTSELV,
	ID_EDIT_INVERTSELE,
	ID_EDIT_INVERTSELA,

	ID_VIEW_LABELS,
	ID_VIEW_WEIGHTS,
	ID_VIEW_FLOWS,

	ID_GRAPH_CLEAR,
	ID_GRAPH_COMPLEMENT,
	ID_GRAPH_LINEGRAPH,
	ID_GRAPH_SUBGRAPH,

	ID_GRAPH_FIND,		// submenu
	ID_GRAPH_FIND_SHORTESTPATH,
	ID_GRAPH_FIND_BFS,
	ID_GRAPH_FIND_DFS,
	ID_GRAPH_FIND_MST,
	ID_GRAPH_FIND_MAXFLOW,

	ID_GRAPH_PROPERTIES,	// submenu
	ID_GRAPH_PROPERTIES_CONNECTIVITY,
	ID_GRAPH_PROPERTIES_EULERICITY,
	ID_GRAPH_PROPERTIES_HAMILTONICITY,
	ID_GRAPH_PROPERTIES_PLANARITY,

	ID_GRAPH_STATISTICS,	// submenu
	ID_GRAPH_STATISTICS_ADJMATRIX,
	ID_GRAPH_STATISTICS_DEGSEQ,
	ID_GRAPH_STATISTICS_DIAMETER,
	ID_GRAPH_STATISTICS_GIRTH,
	ID_GRAPH_STATISTICS_RADIUS,
	ID_GRAPH_STATISTICS_CHROMNUM,
	ID_GRAPH_STATISTICS_CHROMINDEX,
	ID_GRAPH_STATISTICS_CHROMPOLY,

	ID_PREFAB_COMPLETE,
	ID_PREFAB_COMPLETEBIPARTITE,
	ID_PREFAB_CYCLE,
	ID_PREFAB_GEAR,
	ID_PREFAB_HANOI,
	ID_PREFAB_LADDER,
	ID_PREFAB_LATTICE,
	ID_PREFAB_NULL,
	ID_PREFAB_STAR,
	ID_PREFAB_TREE,
	ID_PREFAB_WHEEL,
	ID_PREFAB_PETERSEN,

	ID_PREFAB_PLATONIC,	// submenu
	ID_PREFAB_PLATONIC_TETRAHEDRAL,
	ID_PREFAB_PLATONIC_CUBICAL,
	ID_PREFAB_PLATONIC_OCTAHEDRAL,
	ID_PREFAB_PLATONIC_DODECAHEDRAL,
	ID_PREFAB_PLATONIC_ICOSAHEDRAL,

	ID_HELP_ABOUT,

	// Toolbar
	ID_TOOL_VERTEXMODE,
	ID_TOOL_EDGEMODE,
    ID_TOOL_DIREDGEMODE
};

#endif	// __GUI_H__
