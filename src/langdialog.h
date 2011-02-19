//
//	langdialog.h
//

#ifndef __LANGDIALOG_H__
#define __LANGDIALOG_H__

#include "wx/dialog.h"

#include "lang.h"


class wxWindow;


class LangDialog : public wxDialog
{
public:
	LangDialog ();

	Language GetLanguage () const { return m_lang; }

protected:

	void OnButton (wxCommandEvent &event);

	Language m_lang;

private:
	DECLARE_EVENT_TABLE()
};

#endif	//__LANGDIALOG_H__
