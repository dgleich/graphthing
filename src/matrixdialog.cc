//
//	matrixdialog.cc
//

#include "wx/button.h"
#include "wx/dialog.h"
#include "wx/grid.h"
#include "wx/sizer.h"
#include "wx/spinctrl.h"
#include "wx/statline.h"
#include "wx/stattext.h"
#include "wx/string.h"
#include "wx/window.h"

#include <sstream>
#include <vector>
#include "graph.h"
#include "lang.h"
#include "matrix.h"
#include "matrixdialog.h"
#include "vertex.h"


MatrixGrid::MatrixGrid (wxWindow *parent)
	: wxGrid (parent, wxID_ANY)
{
}

void MatrixGrid::SetScrollbar (int orient, int pos, int thumb, int range, bool refresh)
{
	int m_scrollstyle = 0;
	if (m_scrollstyle & orient)
		wxGrid::SetScrollbar (orient, pos, thumb, range, refresh);
	else
		wxGrid::SetScrollbar (orient, 0, 0, 0);
}

enum {
	ID_SPINCTRL = 1000
};

BEGIN_EVENT_TABLE(MatrixDialog, wxDialog)
	EVT_SPINCTRL(ID_SPINCTRL,	MatrixDialog::OnChangeExponent)
END_EVENT_TABLE()

MatrixDialog::MatrixDialog (wxWindow *parent, const wxString &title,
					const Matrix &mat, const Graph &g)
			: wxDialog (parent, wxID_ANY, title)
{
	unsigned int i;

	wxBeginBusyCursor ();

	// Create identity matrix (A^0)
	Matrix id = mat - mat;		// zero matrix
	for (i = 0; i < id.rows (); ++i)
		id(i, i) = 1;

	// TODO: Check that mat.rows () == mat.columns ()
	matrices.push_back (id);
	matrices.push_back (mat);

	wxBoxSizer *topsizer = new wxBoxSizer (wxVERTICAL);

	// Create grid
	m_grid = new MatrixGrid (this);
	m_grid->CreateGrid (mat.rows (), mat.columns ());
	m_grid->DisableCellEditControl ();
	for (i = 0; i < mat.rows (); ++i) {
		m_grid->SetColLabelValue (i, g[i]->label);
		m_grid->SetRowLabelValue (i, g[i]->label);
	}

	topsizer->Add (m_grid, 1, wxCENTER | wxALL | wxEXPAND, 10);

	// label + spin ctrl below grid
	wxBoxSizer *expsizer = new wxBoxSizer (wxHORIZONTAL);
	expsizer->Add (new wxStaticText (this, wxID_ANY, _("Exponent:")),
				0, wxCENTER | wxLEFT | wxEXPAND, 10);
	m_spinctrl = new wxSpinCtrl (this, ID_SPINCTRL, wxT("1"));
	m_spinctrl->SetRange (0, MatrixDialog::max_exponent);
	expsizer->Add (m_spinctrl, 0, wxCENTER | wxRIGHT, 10);
	topsizer->Add (expsizer);

	// static line
	topsizer->Add (new wxStaticLine (this, wxID_ANY), 0,
						wxEXPAND | wxALL, 10);

	// OK button
	topsizer->Add (CreateStdDialogButtonSizer (wxOK), 0,
						wxEXPAND | wxALL, 10);


	SetSizer (topsizer);
	SetAutoLayout (true);

	Centre (wxBOTH);

	wxSpinEvent event;
	OnChangeExponent (event);

	wxEndBusyCursor ();
}

void MatrixDialog::OnChangeExponent (wxSpinEvent &event)
{
	int exp = m_spinctrl->GetValue (), sz = matrices.size ();
	unsigned int i, j;

	// Compute any powers we don't already have cached
	while (sz <= exp) {
		matrices.push_back (matrices[sz - 1] * matrices[1]);
		sz = matrices.size ();
	}

	Matrix &mat = matrices[exp];

	// Add contents
	m_grid->BeginBatch ();
	for (j = 0; j < mat.rows (); ++j)
		for (i = 0; i < mat.columns (); ++i) {
			wxString str = wxString::Format (wxT("%d"), mat (i, j));
			m_grid->SetCellValue (j, i, str);
		}
	m_grid->EndBatch ();
	m_grid->AutoSize ();

	m_grid->SetScrollbar (wxHORIZONTAL | wxVERTICAL, 0, 0, 0);
	Fit ();
}
