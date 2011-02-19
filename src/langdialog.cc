//
//	langdialog.cc
//

#include "wx/bitmap.h"
#include "wx/bmpbuttn.h"
#include "wx/button.h"
#include "wx/dialog.h"
#include "wx/gbsizer.h"
#include "wx/image.h"
#include "wx/sizer.h"
#include "wx/statline.h"
#include "wx/stattext.h"
#include "wx/string.h"
#include "wx/utils.h"

#include "lang.h"
#include "langdialog.h"


#include "flags/cat.xpm"
#include "flags/de.xpm"
#include "flags/es.xpm"
#include "flags/fr.xpm"
#include "flags/it.xpm"
#include "flags/ro.xpm"
#include "flags/se.xpm"
#include "flags/uk.xpm"

// List of all languages we (at least partially) support
static struct lang_data {
	Language lang;		// enum value from lang.h (e.g. German)
	char **xpm;		// XPM data (may be NULL)
} supported[] = {
	{ Catalan, cat_xpm },
	{ English, uk_xpm },
	{ French, fr_xpm },
	{ German, de_xpm },
	{ Italian, it_xpm },
	{ Romanian, ro_xpm },
	{ Spanish, es_xpm },
	{ SwedishChef, se_xpm }
};
#define NUM_SUPPORTED	(sizeof (supported) / sizeof (supported[0]))

enum {
	ID_LANG = 1000
};

BEGIN_EVENT_TABLE(LangDialog, wxDialog)
    EVT_BUTTON(ID_LANG,		LangDialog::OnButton)
END_EVENT_TABLE()

LangDialog::LangDialog ()
	: wxDialog (0, wxID_ANY, wxString (wxT("Select Language")))
{
	m_lang = English;

	wxBeginBusyCursor();

	wxGridBagSizer *mainsizer = new wxGridBagSizer (5, 5);
	int num_rows, num_cols;

	num_cols = 3;		// TODO: make this adaptive
	num_rows = (NUM_SUPPORTED + num_cols - 1) / num_cols;

	int row = 0, col = 0;
	for (unsigned int i = 0; i < NUM_SUPPORTED; ++i) {
		Language lang = supported[i].lang;
		//String lang_name = Translator::get_language_name (lang);
		wxString lang_name = translator->lookup (lang, "English");

		wxButton *btn;
#if 0
		if (supported[i].xpm) {
			wxBitmap *bmp = new wxBitmap (supported[i].xpm);
			wxImage img = bmp->ConvertToImage ();
			img.Rescale (img.GetWidth () * 2, img.GetHeight () * 2);
			wxPoint pos (img.GetWidth () / 2, img.GetHeight () / 2);
			wxSize sz (img.GetWidth () * 2, img.GetHeight () * 2);
			img.Resize (sz, pos);
			wxBitmap *f_bmp = new wxBitmap (img);
			btn = new wxBitmapButton (this, ID_LANG + i, *f_bmp);
			btn->SetToolTip (lang_name);
#else
		if (supported[i].xpm) {
			wxBitmap *bmp = new wxBitmap (supported[i].xpm);
			btn = new wxBitmapButton (this, ID_LANG + i, *bmp);
			btn->SetToolTip (lang_name);
#endif
		} else {
			btn = new wxButton (this, ID_LANG + i, lang_name);
		}
		if (supported[i].lang == m_lang)
			btn->SetFocus ();
		Connect (ID_LANG + i,
				wxEVT_COMMAND_BUTTON_CLICKED,
				wxCommandEventHandler (LangDialog::OnButton));
		wxGBPosition pos (row, col);
		mainsizer->Add (btn, pos);

		++col;
		if (col >= num_cols) {
			++row;
			col = 0;
		}
	}

	SetSizer (mainsizer);
	SetAutoLayout (true);

	mainsizer->Layout ();
	mainsizer->SetSizeHints (this);
	mainsizer->Fit (this);

	Centre (wxBOTH);

	wxEndBusyCursor();
}

void LangDialog::OnButton (wxCommandEvent &event)
{
	m_lang = supported[event.GetId () - ID_LANG].lang;
	//std::cerr << "LangDialog: event ID = " << event.GetId () << ".\n";
	//std::cerr << "LangDialog: m_lang = " << m_lang << ".\n";

	EndModal (wxID_OK);
}
