//
//	exporter.h
//

#ifndef __EXPORTER_H__
#define __EXPORTER_H__

#include <fstream>
#include "wx/string.h"


class Graph;


class Exporter
{
protected:
	const wxString &filename;
	std::fstream fs;

	Exporter (const wxString fname);

public:
	bool failed;

	static Exporter *make (const wxString format, const wxString fname);

	virtual void output (const Graph *g, bool labels = true, bool weights = true) = 0;
	virtual ~Exporter ();
};

class GraphThing_Exporter : public Exporter
{
public:
	GraphThing_Exporter (const wxString fname) : Exporter (fname) { }
	void output (const Graph *g, bool labels = true, bool weights = true);
};

class GraphML_Exporter : public Exporter
{
public:
	GraphML_Exporter (const wxString fname) : Exporter (fname) { }
	void output (const Graph *g, bool labels = true, bool weights = true);
};

class Graphviz_Exporter : public Exporter
{
public:
	Graphviz_Exporter (const wxString fname) : Exporter (fname) { }
	void output (const Graph *g, bool labels = true, bool weights = true);
};

class Maple_Exporter : public Exporter
{
public:
	Maple_Exporter (const wxString fname) : Exporter (fname) { }
	void output (const Graph *g, bool labels = true, bool weights = true);
};

class Postscript_Exporter : public Exporter
{
public:
	Postscript_Exporter (const wxString fname) : Exporter (fname) { }
	void output (const Graph *g, bool labels = true, bool weights = true);
};

#endif	// __EXPORTER_H__
