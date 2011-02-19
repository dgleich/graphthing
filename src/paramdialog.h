//
//	paramdialog.h
//

#ifndef __PARAMDIALOG_H__
#define __PARAMDIALOG_H__

#include "wx/dialog.h"
#include "wx/spinctrl.h"
#include "wx/string.h"

class Edge;

class wxWindow;


class ParamDialogIntInt : public wxDialog
{
public:
	ParamDialogIntInt (wxWindow *parent, const wxString &title,
		const wxString &prompt, long value1, long min1, long max1,
					long value2, long min2, long max2);

	long GetValue1 () const { return m_value1; }
	long GetValue2 () const { return m_value2; }

protected:

	void OnOK (wxCommandEvent &event);
	void OnCancel (wxCommandEvent &event);

	wxSpinCtrl *m_spinctrl1, *m_spinctrl2;

	long m_value1, m_min1, m_max1;
	long m_value2, m_min2, m_max2;

private:
	DECLARE_EVENT_TABLE()
};

class ParamDialogEdge : public wxDialog
{
public:
	ParamDialogEdge (wxWindow *parent, const wxString &title,
							const Edge *e);
	long GetWeight () const { return m_weight; }
	long GetFlow () const { return m_flow; }
	long GetDirection () const { return m_dir; }

protected:

	void OnOK (wxCommandEvent &event);
	void OnCancel (wxCommandEvent &event);
	void OnSpin (wxSpinEvent &event);
	void OnEdgeDir (wxCommandEvent &event);

	wxSpinCtrl *m_spinctrl_weight, *m_spinctrl_flow;

	long m_weight, m_flow, m_dir;
	bool dir_inverted;

private:
	DECLARE_EVENT_TABLE()
};

#endif	//__PARAMDIALOG_H__
