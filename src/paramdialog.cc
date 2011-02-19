//
//	paramdialog.cc
//

#include "wx/button.h"
#include "wx/dialog.h"
#include "wx/radiobut.h"
#include "wx/sizer.h"
#include "wx/spinctrl.h"
#include "wx/statline.h"
#include "wx/stattext.h"
#include "wx/string.h"
#include "wx/utils.h"

#include "edge.h"
#include "lang.h"
#include "paramdialog.h"
#include "vertex.h"


BEGIN_EVENT_TABLE(ParamDialogIntInt, wxDialog)
    EVT_BUTTON(wxID_OK,		ParamDialogIntInt::OnOK)
    EVT_BUTTON(wxID_CANCEL,	ParamDialogIntInt::OnCancel)
END_EVENT_TABLE()

ParamDialogIntInt::ParamDialogIntInt (wxWindow *parent, const wxString &title,
	const wxString &prompt, long value1, long min1, long max1,
				long value2, long min2, long max2)
                   			: wxDialog (parent, wxID_ANY, title)
{
	m_value1 = value1;
	m_max1 = max1;
	m_min1 = min1;
	m_value2 = value2;
	m_max2 = max2;
	m_min2 = min2;

	wxBeginBusyCursor();

	wxBoxSizer *topsizer = new wxBoxSizer (wxVERTICAL);

	// 2) prompt and spin ctrl
	wxBoxSizer *inputsizer = new wxBoxSizer (wxHORIZONTAL);

	// prompt, if any
	if (!prompt.IsEmpty ())
        	inputsizer->Add (new wxStaticText (this, wxID_ANY, prompt),
					0, wxCENTER | wxLEFT | wxEXPAND, 10);
 
	// spin ctrl
	wxString valStr1, valStr2;
	valStr1.Printf (wxT("%ld"), m_value1);
	valStr2.Printf (wxT("%ld"), m_value2);
	m_spinctrl1 = new wxSpinCtrl (this, wxID_ANY, valStr1);
	m_spinctrl2 = new wxSpinCtrl (this, wxID_ANY, valStr2);
	m_spinctrl1->SetRange (int (m_min1), int (m_max1));
	m_spinctrl2->SetRange (int (m_min2), int (m_max2));
	inputsizer->Add (m_spinctrl1, 1, wxCENTER | wxLEFT | wxRIGHT, 10);
	inputsizer->Add (m_spinctrl2, 1, wxCENTER | wxLEFT | wxRIGHT, 10);

	// add both
	topsizer->Add (inputsizer, 1, wxEXPAND | wxALL, 5);

	// 3) static line
	topsizer->Add (new wxStaticLine (this, wxID_ANY), 0,
						wxEXPAND | wxALL, 10);

	// 4) buttons
	topsizer->Add (CreateButtonSizer (wxOK | wxCANCEL), 0,
							wxEXPAND | wxALL, 10);

	SetSizer (topsizer);
	SetAutoLayout (true);

	topsizer->SetSizeHints (this);
	topsizer->Fit (this);

	Centre (wxBOTH);

	m_spinctrl1->SetSelection (-1, -1);
	m_spinctrl1->SetFocus ();

	wxEndBusyCursor ();
}

void ParamDialogIntInt::OnOK (wxCommandEvent &event)
{
	m_value1 = m_spinctrl1->GetValue ();
	if ((m_value1 < m_min1) || (m_value1 > m_max1)) {
		// not a number or out of range
		m_value1 = -1;
		EndModal (wxID_CANCEL);
	}
	m_value2 = m_spinctrl2->GetValue ();
	if ((m_value2 < m_min2) || (m_value2 > m_max2)) {
		// not a number or out of range
		m_value2 = -1;
		EndModal (wxID_CANCEL);
	}

	EndModal (wxID_OK);
}

void ParamDialogIntInt::OnCancel (wxCommandEvent &event)
{
	EndModal (wxID_CANCEL);
}

////////////////////////////////////////////////////////////

enum {
	ID_SPIN = 1000,

	ID_EDGE_RIGHT,
	ID_EDGE_NONE,
	ID_EDGE_LEFT
};

BEGIN_EVENT_TABLE(ParamDialogEdge, wxDialog)
    EVT_BUTTON(wxID_OK,		ParamDialogEdge::OnOK)
    EVT_BUTTON(wxID_CANCEL,	ParamDialogEdge::OnCancel)
    EVT_SPINCTRL(ID_SPIN,	ParamDialogEdge::OnSpin)
    EVT_RADIOBUTTON(ID_EDGE_RIGHT,	ParamDialogEdge::OnEdgeDir)
    EVT_RADIOBUTTON(ID_EDGE_NONE,	ParamDialogEdge::OnEdgeDir)
    EVT_RADIOBUTTON(ID_EDGE_LEFT,	ParamDialogEdge::OnEdgeDir)
END_EVENT_TABLE()

ParamDialogEdge::ParamDialogEdge (wxWindow *parent, const wxString &title,
							const Edge *e)
                   		: wxDialog (parent, wxID_ANY, title)
{
	m_weight = e->weight;
	m_flow = e->flow;
	m_dir = e->directed ? 1 : 0;

	dir_inverted = (e->w->label < e->v->label);
	if (dir_inverted)
		m_dir = -m_dir;

	wxBeginBusyCursor();

	wxBoxSizer *topsizer = new wxBoxSizer (wxVERTICAL);

	// 2) prompt and spin ctrl
	wxBoxSizer *inputsizer0 = new wxBoxSizer (wxHORIZONTAL);
	wxBoxSizer *inputsizer1 = new wxBoxSizer (wxHORIZONTAL);
	wxBoxSizer *inputsizer2 = new wxBoxSizer (wxHORIZONTAL);

	// prompts
	inputsizer0->Add (new wxStaticText (this, wxID_ANY,
				_("Edge weight:")),
				0, wxCENTER | wxLEFT, 10);
	inputsizer1->Add (new wxStaticText (this, wxID_ANY,
				_("Edge flow:")),
				0, wxCENTER | wxLEFT, 10);
 
	// spin ctrls
	wxString valStr1;
	valStr1.Printf (wxT("%ld"), m_weight);
	m_spinctrl_weight = new wxSpinCtrl (this, ID_SPIN, valStr1);
	m_spinctrl_weight->SetRange (0, 10000);	// FIXME: arbitrary bounds!
	inputsizer0->Add (m_spinctrl_weight, 1, wxCENTER | wxLEFT | wxRIGHT, 5);

	wxString valStr2;
	valStr2.Printf (wxT("%ld"), m_flow);
	m_spinctrl_flow = new wxSpinCtrl (this, ID_SPIN, valStr2);
	m_spinctrl_flow->SetRange (0, m_weight);
	inputsizer1->Add (m_spinctrl_flow, 1, wxCENTER | wxLEFT | wxRIGHT, 5);

	// edge direction
	inputsizer2->Add (new wxStaticText (this, wxID_ANY,
				_("Edge direction:")),
				0, wxCENTER | wxLEFT | wxEXPAND, 10);
	wxBoxSizer *dirsizer = new wxBoxSizer (wxVERTICAL);
	wxString rightS = wxT("--->"), noneS = wxT("----"), leftS = wxT("<---");
	wxString leftV = e->v->label, rightV = e->w->label;
	if (dir_inverted) {
		wxString tmp = leftV;
		leftV = rightV;
		rightV = tmp;
	}
	rightS = leftV + rightS + rightV;
	noneS = leftV + noneS + rightV;
	leftS = leftV + leftS + rightV;
	wxRadioButton *rightB = new wxRadioButton (this, ID_EDGE_RIGHT, rightS,
			wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	wxRadioButton *noneB = new wxRadioButton (this, ID_EDGE_NONE, noneS);
	wxRadioButton *leftB = new wxRadioButton (this, ID_EDGE_LEFT, leftS);
	dirsizer->Add (rightB);
	dirsizer->Add (noneB);
	dirsizer->Add (leftB);
	inputsizer2->Add (dirsizer, 0, wxCENTER, 10);
	if (m_dir == 0)
		noneB->SetValue (true);
	else {
		if (!dir_inverted)
			rightB->SetValue (true);
		else
			leftB->SetValue (true);
	}

	// add both
	topsizer->Add (inputsizer0, 1, wxEXPAND | wxALL, 5);
	topsizer->Add (inputsizer1, 1, wxEXPAND | wxALL, 5);
	topsizer->Add (inputsizer2, 1, wxEXPAND | wxALL, 5);

	// 3) static line
	topsizer->Add (new wxStaticLine (this, wxID_ANY), 0,
						wxEXPAND | wxALL, 10);

	// 4) buttons
	topsizer->Add (CreateButtonSizer (wxOK | wxCANCEL), 0,
							wxEXPAND | wxALL, 10);

	SetSizer (topsizer);
	SetAutoLayout (true);

	topsizer->SetSizeHints (this);
	topsizer->Fit (this);

	CentreOnParent ();

	m_spinctrl_weight->SetSelection (-1, -1);
	m_spinctrl_weight->SetFocus ();

	wxEndBusyCursor();
}

void ParamDialogEdge::OnOK (wxCommandEvent &event)
{
	m_weight = m_spinctrl_weight->GetValue ();
	if ((m_weight < 0) || (m_weight > m_spinctrl_weight->GetMax ())) {
		// not a number or out of range
		m_weight = -1;
		EndModal (wxID_CANCEL);
	}
	m_flow = m_spinctrl_flow->GetValue ();
	if ((m_flow < 0) || (m_flow > m_spinctrl_flow->GetMax ())) {
		// not a number or out of range
		m_flow = -1;
		EndModal (wxID_CANCEL);
	}
	if (m_flow > m_weight) {
		// out of range
		m_flow = -1;
		EndModal (wxID_CANCEL);
	}
	if (dir_inverted)
		m_dir = -m_dir;

	EndModal (wxID_OK);
}

void ParamDialogEdge::OnCancel (wxCommandEvent &event)
{
	EndModal (wxID_CANCEL);
}

void ParamDialogEdge::OnSpin (wxSpinEvent &event)
{
	m_weight = m_spinctrl_weight->GetValue ();
	if ((m_weight < 0) || (m_weight > m_spinctrl_weight->GetMax ())) {
		// not a number or out of range
		return;
	}

	m_flow = m_spinctrl_flow->GetValue ();
	if (m_flow > m_weight)
		m_spinctrl_flow->SetValue (m_weight);	// clamp

	m_spinctrl_flow->SetRange (0, m_weight);
	m_spinctrl_flow->Refresh ();
}

void ParamDialogEdge::OnEdgeDir (wxCommandEvent &event)
{
	switch (event.GetId ()) {
		case ID_EDGE_RIGHT:
			m_dir = 1;
			break;
		case ID_EDGE_NONE:
			m_dir = 0;
			break;
		case ID_EDGE_LEFT:
			m_dir = -1;
			break;
	}
}
