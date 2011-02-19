//
//	canvas.h
//

#ifndef __CANVAS_H__
#define __CANVAS_H__

#include "wx/brush.h"
#include "wx/colour.h"
#include "wx/dc.h"
#include "wx/scrolwin.h"

#include <vector>


class Edge;
class GTFrame;		// see gui.h
class Graph;
class Vertex;

class wxFont;

class Canvas : public wxScrolledWindow
{
private:
	static const int vertex_radius = 8;
	static const int edge_width = 2;
	static const int font_height = 8;

	GTFrame *gui;
	Graph **gg;
	bool vertex_mode;
    bool edge_mode_dir;
	bool do_labels, do_weights, do_flows;

	std::vector<wxColour> selection_colours;
	wxColour white, black, red;
	wxFont *bold_font, *normal_font;

	int motion_last_x, motion_last_y;

	DECLARE_EVENT_TABLE()

	void draw (wxDC &dc, Vertex *v);
	void draw (wxDC &dc, Edge *e, bool curved = false);

	Vertex *findVertex (int x, int y) const;
	Edge *findEdge (int x, int y) const;
	void cb_Properties ();

public:

	Canvas (GTFrame *gui, Graph **g);
	void OnPaint (wxPaintEvent &event);
	void OnEraseBackground (wxEraseEvent &event);
	void OnClick (wxMouseEvent &event);
	void OnMouseMove (wxMouseEvent &event);
	void OnKeyPress (wxKeyEvent &event);

	void redraw ();
	void setVertexMode (bool v_mode = true);
	void setEdgeMode (bool dir);
	void setParam (bool labels, bool weights, bool flows);
};

#endif	// __CANVAS_H__
