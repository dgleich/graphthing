//
//	exporter.cc
//

#include "wx/string.h"

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "config.h"
#include "edge.h"
#include "exporter.h"
#include "graph.h"
#include "vertex.h"


Exporter *Exporter::make (const wxString format, const wxString fname)
{
	if (format == wxT("GraphThing"))
		return new GraphThing_Exporter (fname);
	else if (format == wxT("GraphML"))
		return new GraphML_Exporter (fname);
	else if (format == wxT("Graphviz"))
		return new Graphviz_Exporter (fname);
	else if (format == wxT("Maple"))
		return new Maple_Exporter (fname);
	else if (format == wxT("Postscript"))
		return new Postscript_Exporter (fname);

	throw std::runtime_error ("Unknown output format!");
}

Exporter::Exporter (const wxString fname) : filename (fname)
{
	fs.open ((const char *) filename.mb_str (wxConvUTF8),
							std::fstream::out);
	if (!fs.is_open ())
		throw std::runtime_error ("Couldn't open file.");
}

Exporter::~Exporter ()
{
	fs.close ();
}

void GraphThing_Exporter::output (const Graph *g, bool labels, bool weights)
{
	fs << *g;
}

void GraphML_Exporter::output (const Graph *g, bool labels, bool weights)
{
	fs << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	fs << "<!-- This file created by GraphThing " GT_VERSION
				"   (c) 2001-2006 David Symonds -->\n";
	fs << "<!DOCTYPE graphml SYSTEM \"graphml.dtd\">\n";
	fs << "<graphml>\n";

	fs << "    <key id=\"weight\" for=\"edge\" />\n";
	fs << "    <graph edgedefault=\"undirected\">\n";
	fs << "\t<desc></desc>\n";
	fs << "\n";

	// Vertices
	Graph::v_const_iterator vit;
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		Vertex *v = *vit;

		// TODO: filter out quotes and other nasties!
		fs << "\t<node id=\"" << v->label.mb_str (wxConvUTF8)
			<< "\" />\n";
	}
	fs << "\n";

	// Edges
	Graph::e_const_iterator eit;
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;

		// TODO: filter out quotes and other nasties!
		fs << "\t<edge source=\"" << e->v->label.mb_str (wxConvUTF8)
			<< "\" target=\"" << e->w->label.mb_str (wxConvUTF8)
			<< "\"";
		if (e->directed)
			fs << " directed=\"true\"";
		fs << ">\n";
		fs << "\t    <data key=\"weight\">" << e->weight << "</data>\n";
		fs << "\t</edge>\n";
	}

	fs << "    </graph>\n";
	fs << "</graphml>\n";
}

void Graphviz_Exporter::output (const Graph *g, bool labels, bool weights)
{
	Graph::e_const_iterator eit;
	bool digraph = false;

	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		if ((*eit)->directed) {
			digraph = true;
			break;
		}
	}

	fs << (digraph ? "digraph" : "graph") << " \"G\" {\n";

	// defaults
	fs << "\tnode [\n";
	fs << "\t\tfontname = \"Arial\"\n";
	fs << "\t\tlabel = \"\\N\"\n";
	fs << "\t\tshape = \"circle\"\n";
	fs << "\t\twidth = \"0.5\"\n";
	fs << "\t\theight = \"0.5\"\n";
	fs << "\t\tcolor = \"black\"\n";
	fs << "\t]\n";
	fs << "\tedge [\n";
	fs << "\t\tcolor = \"black\"\n";
	fs << "\t\tweight = \"1\"\n";
	fs << "\t]\n";
	fs << "\n";

	// Vertices are combined with edges in this format
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;

		fs << "\t\"" << e->v->label.mb_str (wxConvUTF8) << "\" "
			<< (e->directed ? "->" : "--") << " \""
			<< e->w->label.mb_str (wxConvUTF8) << "\"";
		if (e->weight != 1)
			fs << " [weight=\"" << e->weight << "\"]";
		fs << ";\n";
	}

	fs << "}\n";
}

void Maple_Exporter::output (const Graph *g, bool labels, bool weights)
{
	// TODO: handle digraphs

	fs << "> with(networks):\n";

	// Vertex set
	fs << "> vertexset:=[";
	Graph::v_const_iterator vit;
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		if (vit != g->v_begin ())
			fs << ",";
		fs << (*vit)->label.mb_str (wxConvUTF8);
	}
	fs << "];\n";

	// Edge set
	fs << "> edgeset:=[";
	Graph::e_const_iterator eit;
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		Edge *e = *eit;
		if (eit != g->e_begin ())
			fs << ",";
		fs << "{" << e->v->label.mb_str (wxConvUTF8) << ","
			<< e->w->label.mb_str (wxConvUTF8) << "}";
	}
	fs << "];\n";

	// Edge weights
	fs << "> weightset:=[";
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		if (eit != g->e_begin ())
			fs << ",";
		fs << (*eit)->weight;
	}
	fs << "];\n";

	fs <<	"> new(G):\n"
		"> addvertex(vertexset,G);\n"
		"> addedge(edgeset,weights=weightset,G);\n";
}

void Postscript_Exporter::output (const Graph *g, bool labels, bool weights)
{
	int minx, miny, maxx, maxy, len;
	time_t timep;
	Graph::e_const_iterator eit;
	Graph::v_const_iterator vit;
	char *str;

	minx = miny = 999999;
	maxx = maxy = -1;
	for (vit = g->v_begin (); vit != g->v_end (); ++vit) {
		if (minx > (*vit)->x)
			minx = (*vit)->x;
		if (maxx < (*vit)->x)
			maxx = (*vit)->x;
		if (miny > (*vit)->y)
			miny = (*vit)->y;
		if (maxy < (*vit)->y)
			maxy = (*vit)->y;
	}
	if (g->order () < 1) {
		minx = miny = 10;
		maxx = maxy = 200;
	}
	minx -= 10;
	miny -= (labels ? 10 : 20);
	maxx += (labels ? 20 : 10);
	maxy += 20;

	timep = time (0);
	str = ctime (&timep);
	len = strlen (str);
	if (str[len - 1] == '\n')
		str[len - 1] = '\0';	// remove trailing newline

	fs <<	"%%!PS-Adobe-2.0\n"
		"%%%%Title: " << filename << "\n"
		"%%%%Creator: GraphThing " GT_VERSION
			"   (c) 2001-2006 David Symonds\n"
		"%%%%CreationDate: " << str << "\n"
		"%%%%Pages: 1\n"
		"%%%%Origin: 0 0\n"
		"%%%%BoundingBox: " << minx << ' ' << miny << ' ' <<
					maxx << ' ' << maxy << "\n\n";

	fs <<	"/Helvetica findfont 12 scalefont setfont\n\n";

	// Vertex macro
	fs <<	"%% Args: label, xcenter, ycenter\n"
		"/vertex {\n"
		"\t" << maxy << " exch sub\n"
		"\t1 index 1 index\n"
		"\tnewpath 5 0 360 arc fill\n";
	if (labels)
		fs << "\texch 10 add exch 10 add moveto show\n";
	else
		fs << "\tpop\n";
	fs << "} bind def\n\n";

	// Arrow head macros
	fs <<	"%% Arrow head stuff\n"
		"/ArrowHeadSize 15 def\n"
		"%% Args: x1 y1 x2 y2  (arrowhead at x2,y2; from x1,y1)\n"
		"%% Out: x1 y1 x2' y2' (draw line to x2', y2')\n"
		"/arrowhead {\n"
		"\t1 index 4 index sub\n"
		"\t1 index 4 index sub\n"
		"\texch atan\n"
		"\n"
		"\tArrowHeadSize -.8 mul\n"
		"\tdup\n"
		"\t2 index cos mul 4 index add\n"
		"\texch\n"
		"\t2 index sin mul 3 index add\n"
		"\n"
		"\t5 2 roll\n"
		"\n"
		"\tgsave\n"
		"\t\t3 1 roll\n"
		"\t\ttranslate\n"
		"\t\trotate\n"
		"\t\tnewpath\n"
		"\t\t0 0 moveto\n"
		"\t\tArrowHeadSize dup neg exch .25 mul\n"
		"\t\t2 copy lineto\n"
		"\t\tArrowHeadSize -.8 mul 0\n"
		"\t\t2 copy\n"
		"\t\t6 4 roll\n"
		"\t\tneg curveto\n"
		"\t\tclosepath fill\n"
		"\tgrestore\n"
		"} bind def\n\n";

	// Edge macros
	fs <<	"%% Return point on line, given by t = [0, 1]\n"
		"%% Args: x1 y1 x2 y2 t\n"
		"%% Out: xt yt\n"
		"/linepositiondict 5 dict def\n"
		"/lineposition {\n"
		"\tlinepositiondict begin\n"
		"\t\t/t exch def\n"
		"\t\t/y2 exch def /x2 exch def\n"
		"\t\t/y1 exch def /x1 exch def\n"
		"\t\tx2 x1 sub t mul x1 add\n"
		"\t\ty2 y1 sub t mul y1 add\n"
		"\tend\n"
		"} bind def\n\n";
	fs <<	"%% Args: label, startx, starty, endx, endy\n"
		"/edge {\n"
		"\t" << maxy << " exch sub 3 index 3 index " << maxy <<
							" exch sub\n"
		"\t1 index 1 index moveto\n"
		"\t3 index 3 index lineto stroke\n";
	if (labels)
		fs <<	"\t0.5 lineposition\n"
			"\texch 5 add exch 5 add moveto pop pop show\n";
	else
		fs << "\tpop pop pop pop pop pop pop\n";
	fs <<	"} bind def\n\n";
	fs <<	"%% Args: label, startx, starty, endx, endy\n"
		"/dir_edge {\n"
		"\t" << maxy << " exch sub 3 index 3 index " << maxy <<
							" exch sub\n"
		"\t3 index 3 index arrowhead\n"
		"\t3 index 3 index moveto\n"
		"\tlineto stroke\n";
	if (labels)
		fs <<	"\t0.5 lineposition\n"
			"\texch 5 add exch 5 add moveto pop pop show\n";
	else
		fs << "\tpop pop pop pop pop pop pop\n";
	fs <<	"} bind def\n\n";

	// We dump the edges first so that the vertices will be printed
	// "on top" of the edge ends

	// Dump edges first
	fs << "%% Edges\n";
	for (eit = g->e_begin (); eit != g->e_end (); ++eit) {
		if (weights)
			fs << "(" << (*eit)->weight << ") ";
		else
			fs << "() ";
		fs << (*eit)->v->x << ' ' << (*eit)->v->y << ' ' <<
			(*eit)->w->x << ' ' << (*eit)->w->y <<
			((*eit)->directed ? " dir_edge\n" : " edge\n");
	}
	fs << '\n';

	// Now dump the vertices
	fs << "%% Vertices\n";
	// This bit needs a freshly created iterator. Why? I have no idea.
	// It crashes if we try to use the previous ('vit').
	Graph::v_const_iterator vit2;
	for (vit2 = g->v_begin (); vit2 != g->v_end (); ++vit2) {
		Vertex *v = *vit2;
		//std::cerr << "* dumping '" << v->label << "'...\n";
		fs << '(' << v->label.mb_str (wxConvUTF8) << ") " << v->x
			<< ' ' << v->y << " vertex\n";
	}

	fs << "\n\nshowpage\n";
}
