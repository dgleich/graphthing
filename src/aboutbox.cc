//
//	aboutbox.cc
//

#include "wx/dialog.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "wx/string.h"

#include <sstream>
#include "aboutbox.h"
#include "config.h"
#include "lang.h"


#ifdef ENABLE_NLS
#include "wx/bitmap.h"
#include "wx/bmpbuttn.h"
#include "logo_sc.xpm"
#endif


AboutBox::AboutBox (wxWindow *parent)
		: wxDialog (parent, -1, _("About..."), wxDefaultPosition)
{
	wxString msg;
	wxBoxSizer *vbox = new wxBoxSizer (wxVERTICAL);


	vbox->Add (new wxStaticText (this, -1, wxT("G r a p h T h i n g")),
			0, wxALL | wxALIGN_CENTER, 10);
	msg << _("Version") << wxT(" " GT_VERSION);
	vbox->Add (new wxStaticText (this, -1, msg),
			0, wxALL | wxALIGN_CENTER, 0);
	vbox->Add (new wxStaticText (this, -1, wxT("(c) 2001-2006 David Symonds")),
			0, wxALL | wxALIGN_CENTER, 0);

	msg.Clear ();
	msg << _(
		"This program is free software; you can redistribute\n"
		"it and/or modify it under the terms of the GNU\n"
		"General Public License, version 2, as published by\n"
		"the Free Software Foundation.\n\n"

		"This program is distributed in the hope that it will\n"
		"be useful, but WITHOUT ANY WARRANTY; without even\n"
		"the implied warranty of MERCHANTABILITY or FITNESS\n"
		"FOR A PARTICULAR PURPOSE. See the GNU General\n"
		"Public License for more details.");
	wxStaticText *txt = new wxStaticText (this, -1, msg);
	vbox->Add (txt, 0, wxALL, 10);

	wxSizer *btns;
#ifdef ENABLE_NLS
	if (translator->get_language () != SwedishChef)
#endif
		btns = CreateStdDialogButtonSizer (wxOK);
#ifdef ENABLE_NLS
	else {
		btns = new wxBoxSizer (wxHORIZONTAL);
		wxBitmapButton *btn = new wxBitmapButton (this, wxID_OK,
						wxBitmap (logo_sc_xpm));
		btn->SetToolTip (wxT("Bork Bork Bork!"));

		btns->Add (btn);
		//btns->Layout ();
	}
#endif
	vbox->Add (btns, 0, wxALL | wxEXPAND, 10);

	SetSizer (vbox);
	vbox->SetSizeHints (this);
	vbox->Fit (this);
	vbox->Layout ();
}
