//
//	undo.cc
//

#include "wx/string.h"

#include "graph.h"
#include "undo.h"


UndoStep::UndoStep (Graph *graph, const wxString desc)
	: original_graph (graph), description (desc)
{
}

UndoStep::UndoStep (const UndoStep &other)
	: original_graph (other.original_graph), description (other.description)
{
}

UndoStep::~UndoStep ()
{
}

Graph *UndoStep::getGraph () const
{
	return original_graph;
}

const wxString UndoStep::getMessage () const
{
	return description;
}

UndoStack::UndoStack ()
{
}

UndoStack::~UndoStack ()
{
	while (!empty ())
		delete pop ().getGraph ();
}

bool UndoStack::empty () const
{
	return undos.empty ();
}

unsigned int UndoStack::size () const
{
	return undos.size ();
}

const UndoStep &UndoStack::top () const
{
	return undos.top ();
}

void UndoStack::push (UndoStep &step)
{
	undos.push (step);
}

UndoStep &UndoStack::pop ()
{
	UndoStep &step = undos.top ();

	undos.pop ();
	return step;
}
