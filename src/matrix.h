//
//	matrix.h
//

#ifndef __MATRIX_H__
#define __MATRIX_H__

#include <iostream>


class Matrix
{
private:
	class MatrixRep
	{
	public:
		unsigned int rows, columns;
		int *data;
		unsigned int ref;

		MatrixRep (unsigned int _rows, unsigned int _columns);
		MatrixRep (const MatrixRep *other);
		~MatrixRep ();
	};

	MatrixRep *rep;

	void unref ();
	void mutator ();

public:
	Matrix (unsigned int _rows, unsigned int _columns);
	Matrix (const Matrix &other);
	~Matrix ();

	unsigned int rows () const;
	unsigned int columns () const;


	int &operator () (unsigned int row, unsigned int column);
	int operator () (unsigned int row, unsigned int column) const;
	Matrix &operator= (const Matrix &other);
	bool operator== (const Matrix &other) const;
	void operator+= (const Matrix &other);
	Matrix operator+ (const Matrix &other) const;
	void operator-= (const Matrix &other);
	Matrix operator- (const Matrix &other) const;

	Matrix operator* (const Matrix &other) const;

	friend std::ostream &operator<< (std::ostream &o, const Matrix &mat);
};


#endif	// __MATRIX_H__
