//
//	main.cc
//

#include "wx/app.h"

#include <iostream>
#include <stdexcept>
#include "config.h"
#include "gui.h"


#ifdef ENABLE_NLS
#include "lang.h"
#ifndef LANG_DIALOG_IS_DISABLED
#include "langdialog.h"
#endif

Translator *translator;
#endif

class GTApp : public wxApp
{
private:
	GTFrame *win;

public:
	virtual bool OnInit ();
	virtual int OnExit ();

	void setProgress (double frac);
};

IMPLEMENT_APP(GTApp)

bool GTApp::OnInit ()
{
#ifdef ENABLE_NLS
	translator = new Translator (1);	// be noisy
	try {
		translator->init ();
	} catch (std::runtime_error e) {
		std::cerr << "Translation init failed: " << e.what () << "\n";
		return false;
	}
#ifndef LANG_DIALOG_IS_DISABLED
	if (!translator->guess_language ()) {
		LangDialog *dlg = new LangDialog ();
		if (dlg->ShowModal () == wxID_OK)
			translator->set_language (dlg->GetLanguage ());
		dlg->Destroy ();
	}
#else
	translator->guess_language ();
#endif

#ifdef GT_WITH_DEV
	translator->verify_phrases ();
#endif
#endif

	win = new GTFrame ();
	win->Show (true);
	SetTopWindow (win);

	if (argc > 1)
		win->loadGraph (wxString (argv[1], wxConvUTF8));

	return true;
}

int GTApp::OnExit ()
{
#ifdef ENABLE_NLS
	delete translator;
#endif

	return 0;
}

void GTApp::setProgress (double frac)
{
	win->setProgress (frac);
}

void setProgress (double frac)
{
	wxGetApp ().setProgress (frac);
}
