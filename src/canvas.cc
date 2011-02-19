//
//	canvas.cc
//

#include "wx/brush.h"
#include "wx/colour.h"
#include "wx/dc.h"
#include "wx/dcclient.h"
#include "wx/font.h"
#include "wx/gdicmn.h"
#include "wx/pen.h"
#include "wx/scrolwin.h"
#include "wx/textdlg.h"

#include "canvas.h"
#include "edge.h"
#include "graph.h"
#include "gui.h"
#include "lang.h"
#include "paramdialog.h"
#include "vertex.h"


BEGIN_EVENT_TABLE(Canvas, wxScrolledWindow)
	EVT_PAINT	(Canvas::OnPaint)
	EVT_ERASE_BACKGROUND(Canvas::OnEraseBackground)
	EVT_LEFT_DOWN	(Canvas::OnClick)
	EVT_LEFT_DCLICK	(Canvas::OnClick)
	EVT_RIGHT_DOWN	(Canvas::OnClick)
	EVT_MOTION	(Canvas::OnMouseMove)
	EVT_CHAR	(Canvas::OnKeyPress)
END_EVENT_TABLE()


void Canvas::draw (wxDC &dc, Vertex *v)
{
	int r = Canvas::vertex_radius;
	wxColour col = black;

	if (v->selected) {
		unsigned int idx = v->selection_colour %
						selection_colours.size ();
		col = selection_colours[idx];
	}

	dc.SetBrush (wxBrush (col));
	dc.SetPen (wxPen (col));

	dc.DrawCircle (v->x, v->y, r);

	if (do_labels) {
		dc.SetFont (*bold_font);
		dc.SetTextForeground (col);
		dc.DrawText (v->label, v->x + r, v->y - 2 * r);
	}
}

void Canvas::draw (wxDC &dc, Edge *e, bool curved)
{
	double dx, dy, r = Canvas::vertex_radius;
	double theta;
	wxColour col = black;

	if (e->selected) {
		unsigned int idx = e->selection_colour %
						selection_colours.size ();
		col = selection_colours[idx];
	}
	wxPen pen (col, Canvas::edge_width);
	pen.SetCap (wxCAP_BUTT);
	pen.SetJoin (wxJOIN_MITER);
	dc.SetPen (pen);
	dc.SetBrush (wxBrush (col));

	theta = atan2 (e->w->y - e->v->y, e->w->x - e->v->x);
	dx = r * cos (theta);
	dy = r * sin (theta);

	if (e->v->y == e->w->y)		// horizontal line (dy = 0)
		dy = 0;
	else if (e->v->x == e->w->x)	// vertical line (dx = 0)
		dx = 0;

	if (!curved)
		dc.DrawLine (int (e->v->x + dx), int (e->v->y + dy),
				int (e->w->x - dx), int (e->w->y - dy));
	else {
		dc.SetBrush (wxBrush (white));
		// TODO: get this right!
		double cx, cy;	// mid-spline control point
		cx = (e->v->x + e->w->x) / 2 - 2 * dy;
		cy = (e->v->y + e->w->y) / 2 - 2 * dx;
		dc.DrawSpline (int (e->v->x + dx), int (e->v->y + dy),
				int (cx), int (cy),
				int (e->w->x - dx), int (e->w->y - dy));

		dc.SetBrush (wxBrush (col));
	}

	if (e->directed) {
		wxPoint tri[3];
		double scale = 0.7;
		tri[0] = wxPoint (0, 0);
		tri[1] = wxPoint (int (scale * (dy - dx)),
				int (-scale * (dx + dy)));
		tri[2] = wxPoint (int (-scale * (dx + dy)),
				int (scale * (dx - dy)));
		dc.DrawPolygon (3, tri, int (e->w->x - dx), int (e->w->y - dy));
	}

	if (do_weights) {
		wxString str;
		if (!do_flows)
			str = wxString::Format (wxT("%i"), e->weight);
		else
			str = wxString::Format (wxT("%i/%i"), e->flow, e->weight);
		double rx, ry;
		wxCoord w, h;

		dc.SetFont (*normal_font);
		dc.GetTextExtent (str, &w, &h);

		rx = (e->v->x + e->w->x) / 2 + dy;
		ry = (e->v->y + e->w->y) / 2 - dx;
		if (curved) {
			rx += dy;
			ry -= dy;
		}
		if (theta < 0)
			rx += w * sin (theta);
		ry -= h / 2;

		if (fabs (theta) > M_PI_2)
			ry -= dx / 2;

		dc.SetTextForeground (col);
		dc.DrawText (str, int (rx), int (ry));
	}
}

Vertex *Canvas::findVertex (int x, int y) const
{
	Graph::v_const_iterator vit;
	int r2 = Canvas::vertex_radius * Canvas::vertex_radius;
	Graph *g = *gg;

	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		int dx = x - (*vit)->x, dy = y - (*vit)->y;
		if ((dx * dx + dy * dy) < r2)
			return *vit;
	}

	return 0;
}

Edge *Canvas::findEdge (int x, int y) const
{
	Graph::e_const_iterator eit;
	double rad = Canvas::vertex_radius;
	double r2 = rad * rad;
	double best_dist = 0;		// actually, best distance squared
	Edge *best_edge = 0;
	Graph *g = *gg;

	// find straight-line edge closest to (x,y)
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		double x1, y1, x2, y2, s2, num, d2;
		x1 = (*eit)->v->x;
		y1 = (*eit)->v->y;
		x2 = (*eit)->w->x;
		y2 = (*eit)->w->y;
		if (x1 < x2) {
			if ((x < (x1 - rad)) || ((x2 + rad) < x))
				continue;
		} else if (x2 < x1) {
			if ((x < (x2 - rad)) || ((x1 + rad) < x))
				continue;
		}
		if (y1 < y2) {
			if ((y < (y1 - rad)) || ((y2 + rad) < y))
				continue;
		} else if (y2 < y1) {
			if ((y < (y2 - rad)) || ((y1 + rad) < y))
				continue;
		}
		s2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
		if (s2 < 0.01)
			continue;	// degenerate edge
		num = (y2 - y1) * (x - x1) - (x2 - x1) * (y - y1);
		d2 = num * num / s2;
		if ((d2 < best_dist) || !best_edge) {
			best_edge = *eit;
			best_dist = d2;
		}
	}

	// still only accept it if within a vertex of (x,y)
	if (best_dist <= r2)
		return best_edge;
	return 0;
}

void Canvas::cb_Properties ()
{
	Edge *e;
	Graph *g = *gg;
	Vertex *v;

	v = g->v_selected_head;
	e = g->e_selected_head;

	if (v && !e) {
		// Vertex Properties

		wxString str = wxGetTextFromUser (_("Label:"),
					_("Vertex Properties"),
					v->label, gui);
		if (str.IsEmpty ())
			return;

		// Handle duplicate labels
		while (1) {
			Vertex *f = g->find (str);
			if (!f || (f == v))
				break;
			str += wxT("-dup");
		}

		if (str != v->label) {
			gui->undoableAction (_("Rename vertex"));
			g->rename (v, str);
			redraw ();
		}
	} else if (e) {
		// Edge Properties
		ParamDialogEdge dlg (gui, _("Edge Properties"), e);
		if (dlg.ShowModal () == wxID_CANCEL)
			return;

		int curr_dir = e->directed ? 1 : 0;
		int new_wt = dlg.GetWeight (), new_dir = dlg.GetDirection ();
		int new_flow = dlg.GetFlow ();
		if ((new_wt == e->weight) && (new_dir == curr_dir)) {
			if (e->flow != new_flow) {
				e->flow = new_flow;
				redraw ();
			}
			return;
		}
		if (new_wt == e->weight)
			gui->undoableAction (_("Change edge direction"));
		else if (new_dir == curr_dir)
			gui->undoableAction (_("Change edge weight"));
		else
			gui->undoableAction (_("Change edge weight and direction"));
		e->weight = new_wt;
		if (new_dir == 0)
			e->directed = false;
		else if (new_dir == 1)
			e->directed = true;
		else {
			e->directed = true;
			Vertex *tmp = e->v;
			e->v = e->w;
			e->w = tmp;
		}
		redraw ();
	}
}

Canvas::Canvas (GTFrame *gui, Graph **g)
	: wxScrolledWindow (gui, -1, wxPoint (0, 0), wxDefaultSize,
//			0),
			wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN),
		gui (gui), gg (g)
{
	vertex_mode = true;
	do_labels = true;
	do_weights = do_flows = false;

	white = wxColour (255, 255, 255);
	black = wxColour (0, 0, 0);

	// Selection colours
	const char *cols[] = {
		"red", "blue", "magenta", "green",
		"brown", "cyan", "black"
	};
	for (unsigned int i = 0; i < (sizeof (cols) / sizeof (cols[0])); ++i) {
		wxColour col = wxTheColourDatabase->Find
					(wxString (cols[i], wxConvUTF8));
		selection_colours.push_back (col);
	}
	red = selection_colours[0];


	int pointSize = Canvas::font_height;
	normal_font = new wxFont (pointSize, wxFONTFAMILY_DEFAULT,
				wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	bold_font = new wxFont (pointSize, wxFONTFAMILY_DEFAULT,
				wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);

	SetScrollRate (10, 10);
	
}

#define USE_WXMEMORYDC

#ifdef USE_WXMEMORYDC
#include "wx/dcmemory.h"
#endif

void Canvas::OnPaint (wxPaintEvent &event)
{
#ifndef USE_WXMEMORYDC
	wxPaintDC dc (this);
#else
	wxPaintDC dest_dc (this);
	int blit_width, blit_height;
	GetClientSize (&blit_width, &blit_height);
	wxBitmap bmp_blit (blit_width, blit_height);
	wxMemoryDC dc;
	dc.SelectObject (bmp_blit);
#endif
	Graph *g = *gg;

#ifndef USE_WXMEMORYDC
	DoPrepareDC (dc);
#else
	DoPrepareDC (dc);
	DoPrepareDC (dest_dc);
#endif

	dc.SetBackground (wxBrush (white));
	dc.Clear ();

	Graph::e_const_iterator eit;
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;
		bool curved = false;
		if (e->directed) {
			if (g->find (e->w, e->v, true))
				curved = true;
		}
		draw (dc, e, curved);
	}

	Graph::v_const_iterator vit;
	int maxx = 0, maxy = 0;
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		Vertex *v = *vit;
		draw (dc, v);
		if (v->x > maxx)
			maxx = v->x;
		if (v->y > maxy)
			maxy = v->y;
	}
#ifdef USE_WXMEMORYDC
	// Blit!
	dest_dc.Blit (0, 0, blit_width, blit_height, &dc, 0, 0);
#endif

	SetVirtualSize (maxx + 30, maxy + 30);
}

void Canvas::OnEraseBackground (wxEraseEvent &event)
{
	wxDC *dc = event.GetDC ();

	dc->SetBackground (wxBrush (white));
	dc->Clear ();
}

void Canvas::OnClick (wxMouseEvent &event)
{
	Graph *g = *gg;
	Vertex *v;
	int m_x, m_y;

	bool left_click = event.LeftDown () || event.LeftDClick (),
		shifted = event.ShiftDown (),
		double_click = event.ButtonDClick ();

	CalcUnscrolledPosition (int (event.m_x), int (event.m_y), &m_x, &m_y);
	v = findVertex (m_x, m_y);

	// Assumptions: [lr]-click (or double click)

	if (vertex_mode && left_click) {
		if (!shifted) {
			// no shift key
			if (!double_click) {
				// Single click
				g->unselect_all ();
				if (v)
					g->select (v);
				redraw ();
				return;
			} else {
				// Double click
				if (v)
					cb_Properties ();
				else {
					gui->undoableAction (_("Add vertex"));
					g->add (new Vertex ("", m_x, m_y));
					redraw ();
				}
				return;
			}
		} else {
			// shift key
			if (!double_click) {
				if (v) {
					v->selected ?
						g->unselect (v) :
						g->select (v);
					redraw ();
				}
			}
			return;
		}
	} else if (vertex_mode && !left_click)
		return;		// do nothing

	// edge mode
	if (!v) {
		Edge *e = findEdge (m_x, m_y);
		if (!e) {
			if (left_click && !shifted) {
				g->unselect_all ();
				redraw ();
			}
			return;
		}
		// clicking on an edge
		if (!double_click) {
			// Single click
			if (left_click) {
				if (!shifted) {
					g->unselect_all ();
					g->select (e);
				} else {
					e->selected ?
						g->unselect (e) :
						g->select (e);
				}
			} else
				e->cycle_orientations ();
		} else {
			// Double click
			if (left_click)
				cb_Properties ();
		}
		redraw ();
		return;
	}
	if (!left_click)
		return;
	if (!v->selected) {
		Vertex *valt = g->v_selected_head;
		if (!valt) {
			g->select (v);
			redraw ();
			return;
		}
		if (!g->are_adjacent (valt, v)) {
			gui->undoableAction (_("Add edge"));
			g->add (new Edge (valt, v));
		}
		g->unselect_all ();
		g->select (v);
		redraw ();
		return;
	}
	g->unselect (v);
	redraw ();
}

void Canvas::OnMouseMove (wxMouseEvent &event)
{
	Graph *g = *gg;
	Vertex *v;
	int m_x, m_y;

	CalcUnscrolledPosition (int (event.m_x), int (event.m_y), &m_x, &m_y);

	if (event.Moving ()) {
		// Button not down => nothing to do
		motion_last_x = -1;
		motion_last_y = -1;
		return;
	}

	if (motion_last_x == -1) {
		// First motion event; record start only
		motion_last_x = m_x;
		motion_last_y = m_y;
		return;
	}

	// Move stuff
	for (v = g->v_selected_head; v; v = v->next) {
		v->x += (m_x - motion_last_x);
		v->y += (m_y - motion_last_y);
	}

	motion_last_x = m_x;
	motion_last_y = m_y;

	redraw ();
}

void Canvas::OnKeyPress (wxKeyEvent &event)
{
	Graph *g = *gg;

	if (!event.HasModifiers () && (event.GetKeyCode () == WXK_DELETE)) {
		// delete selected objects
		// TODO: make this message more accurate
		gui->undoableAction (_("Delete objects"));

		Edge *prevE, *e = g->e_selected_head;
		while (e) {
			prevE = e;
			e = e->next;
			g->remove (prevE);
		}

		Vertex *prevV, *v = g->v_selected_head;
		while (v) {
			prevV = v;
			v = v->next;
			g->remove (prevV);
		}

		redraw ();
		return;
	}

	if (event.GetKeyCode () == ' ') {
		gui->toggleMode ();
		return;
	}

	// We don't want it - pass it back up
	event.Skip ();
}

void Canvas::redraw ()
{
	Refresh ();
	Update ();
}

void Canvas::setVertexMode (bool v_mode)
{
	vertex_mode = v_mode;
}

void Canvas::setEdgeMode ()
{
	vertex_mode = false;
}

void Canvas::setParam (bool labels, bool weights, bool flows)
{
	do_labels = labels;
	do_weights = weights;
	do_flows = flows;
}
