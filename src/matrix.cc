//
//	matrix.cc
//

#include <iostream>
#include <string>
#include <string.h>
#include "matrix.h"


Matrix::MatrixRep::MatrixRep (unsigned int _rows, unsigned int _columns)
{
	rows = _rows;
	columns = _columns;
	data = new int[rows * columns];
	ref = 0;
}

Matrix::MatrixRep::MatrixRep (const MatrixRep *other)
{
	rows = other->rows;
	columns = other->columns;
	data = new int[rows * columns];
	ref = 0;

	memcpy (data, other->data, rows * columns * sizeof (int));
}

Matrix::MatrixRep::~MatrixRep ()
{
	delete[] data;
}

void Matrix::unref ()
{
	if (--rep->ref < 1)
		delete rep;
}

void Matrix::mutator ()
{
	if (rep->ref < 2)
		return;

	const MatrixRep *old = rep;
	unref ();
	rep = new MatrixRep (old);
	++rep->ref;
}

Matrix::Matrix (unsigned int _rows, unsigned int _columns)
{
	rep = new MatrixRep (_rows, _columns);
	++rep->ref;
}

Matrix::Matrix (const Matrix &other)
{
	rep = other.rep;
	++rep->ref;
}

Matrix::~Matrix ()
{
	unref ();
}

unsigned int Matrix::rows () const
{
	return rep->rows;
}

unsigned int Matrix::columns () const
{
	return rep->columns;
}

int &Matrix::operator () (unsigned int row, unsigned int column)
{
	// TODO: bounds checking

	mutator ();

	return rep->data[row * rep->columns + column];
}

int Matrix::operator () (unsigned int row, unsigned int column) const
{
	// TODO: bounds checking
	return rep->data[row * rep->columns + column];
}

Matrix &Matrix::operator= (const Matrix &other)
{
	if (this == &other)
		return *this;

	unref ();

	rep = other.rep;
	++rep->ref;

	return *this;
}

bool Matrix::operator== (const Matrix &other) const
{
	unsigned int i, j;

	if (rep == other.rep)
		return true;
	if ((rep->rows != other.rep->rows) ||
	    (rep->columns != other.rep->columns))
		return false;

	for (j = 0; j < rep->rows; ++j)
		for (i = 0; i < rep->columns; ++i)
			if ((*this) (i, j) != other (i, j))
				return false;
	return true;
}

void Matrix::operator+= (const Matrix &other)
{
	// TODO: check for equal sizes
	unsigned int i, j;

	mutator ();

	for (j = 0; j < rep->rows; ++j)
		for (i = 0; i < rep->columns; ++i)
			(*this) (i, j) += other (i, j);
}

Matrix Matrix::operator+ (const Matrix &other) const
{
	// TODO: check for equal sizes
	Matrix ret (*this);

	ret += other;
	return ret;
}

void Matrix::operator-= (const Matrix &other)
{
	// TODO: check for equal sizes
	unsigned int i, j;

	mutator ();

	for (j = 0; j < rep->rows; ++j)
		for (i = 0; i < rep->columns; ++i)
			(*this) (i, j) -= other (i, j);
}

Matrix Matrix::operator- (const Matrix &other) const
{
	// TODO: check for equal sizes
	Matrix ret (*this);

	ret -= other;
	return ret;
}

Matrix Matrix::operator* (const Matrix &other) const
{
	// TODO: check for proper sizes: this.columns == other.rows
	Matrix ret (rows (), other.columns ());
	unsigned int common = columns ();

	for (unsigned int i = 0; i < ret.columns (); ++i) {
		for (unsigned int j = 0; j < ret.rows (); ++j) {
			int tot = 0;
			for (unsigned int k = 0; k < common; ++k)
				tot += (*this) (j, k) * other (k, i);
			ret (j, i) = tot;
		}
	}

	return ret;
}

std::ostream &operator<< (std::ostream &o, const Matrix &mat)
{
	unsigned int i, j;

	o << '/' << std::string (mat.rep->columns * 2, '-') << "-\\\n";
	for (j = 0; j < mat.rep->rows; ++j) {
		o << "| ";
		for (i = 0; i < mat.rep->columns; ++i)
			o << mat (i, j) << ' ';
		o << "|\n";
	}
	o << '\\' << std::string (mat.rep->columns * 2, '-') << "-/\n";

	return o;
}
