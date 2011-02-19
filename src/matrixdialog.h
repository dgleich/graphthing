//
//	matrixdialog.h
//

#ifndef __MATRIXDIALOG_H__
#define __MATRIXDIALOG_H__

#include "wx/dialog.h"
#include "wx/grid.h"
#include "wx/spinctrl.h"
#include "wx/string.h"

#include <vector>
#include "graph.h"
#include "matrix.h"

class wxWindow;


class MatrixGrid : public wxGrid
{
public:
	MatrixGrid (wxWindow *parent);
	void SetScrollbar (int orient, int pos, int thumb, int range,
						bool refresh = true);
};

class MatrixDialog : public wxDialog
{
public:
	MatrixDialog (wxWindow *parent, const wxString &title,
					const Matrix &mat, const Graph &g);

private:

	static const int max_exponent = 20;

	void OnChangeExponent (wxSpinEvent &event);

	wxSpinCtrl *m_spinctrl;
	MatrixGrid *m_grid;

	std::vector<Matrix> matrices;

	DECLARE_EVENT_TABLE()
};

#endif	//__MATRIXDIALOG_H__
