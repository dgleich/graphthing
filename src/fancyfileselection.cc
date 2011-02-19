//
//	fancyfileselection.cc
//

#include "wx/filedlg.h"
#include "wx/string.h"
#include "wx/window.h"

#include <sstream>
#include "fancyfileselection.h"
#include "lang.h"


static struct {
	const char *name, *extension;
} file_formats[] = {
	{ "GraphThing", ".gt" },
	{ "GraphML", ".graphml" },
	{ "Graphviz", ".dot" },
	{ "Maple", ".mws" },
	{ "Postscript", ".ps" }
};

FancyFileSelection::FancyFileSelection (wxWindow *parent, const wxString &title,
					long style, bool file_types,
					const wxString &mask)
		: wxFileDialog (parent, title, wxT(""), wxT(""), wxT("*.*"), style)
{
	if (style & wxOPEN)
		style |= wxHIDE_READONLY;
	else if (style & wxSAVE)
		style |= wxOVERWRITE_PROMPT;
	SetStyle (style);

	if (file_types) {
		wxString wc = wxT("");
		int num = sizeof (file_formats) / sizeof (file_formats[0]);
		for (int i = 0; i < num; ++i) {
			if (i > 0)
				wc += wxT("|");
			wc += wxString (file_formats[i].name, wxConvUTF8);
			wc += wxT(" (*");
			wc += wxString (file_formats[i].extension, wxConvUTF8);
			wc += wxT(")|*");
			wc += wxString (file_formats[i].extension, wxConvUTF8);
			wc += wxT("");
		}
		SetWildcard (wc);
	} else {
		SetWildcard (mask);
	}
}

wxString FancyFileSelection::GetFileType () const
{
	return wxString (file_formats[GetFilterIndex ()].name, wxConvUTF8);
}

wxString FancyFileSelection::GetFileName () const
{
	return wxFileDialog::GetPath ();
}
