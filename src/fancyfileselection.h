//
//	fancyfileselection.h
//

#ifndef __FANCYFILESELECTION_H__
#define __FANCYFILESELECTION_H__

#include "wx/filedlg.h"
#include "wx/string.h"

class wxWindow;


class FancyFileSelection : public wxFileDialog
{
public:
	FancyFileSelection (wxWindow *parent, const wxString &title,
			long style = 0,
			bool file_types = false,
			const wxString &mask = wxT("GraphThing (*.gt)|*.gt"));

	wxString GetFileType () const;
	virtual wxString GetFileName () const;
};

#endif	// __FANCYFILESELECTION_H__
