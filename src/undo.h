//
//	undo.h
//

#ifndef __UNDO_H__
#define __UNDO_H__

#include "wx/string.h"

#include <stack>


class Graph;


class UndoStep
{
private:
	Graph *original_graph;
	const wxString description;

public:
	UndoStep (Graph *graph, const wxString desc);
	UndoStep (const UndoStep &other);
	~UndoStep ();

	Graph *getGraph () const;
	const wxString getMessage () const;
};

class UndoStack
{
private:
	std::stack<UndoStep> undos;

public:
	UndoStack ();
	~UndoStack ();

	bool empty () const;
	unsigned int size () const;
	const UndoStep &top () const;

	void push (UndoStep &step);
	UndoStep &pop ();
};

#endif	// __UNDO_H__
