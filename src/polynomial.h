//
//	polynomial.h
//

#ifndef __POLYNOMIAL_H__
#define __POLYNOMIAL_H__

#include "wx/string.h"

#include <iostream>
#include <vector>


class Polynomial
{
private:
	class PolynomialRep
	{
	public:
		std::vector<int> data;
		unsigned int ref;

		PolynomialRep ();
		PolynomialRep (const PolynomialRep *other);
	};

	PolynomialRep *rep;

	void minimise ();
	void unref ();
	void mutator ();

public:
	Polynomial ();
	Polynomial (int x0);
	Polynomial (int x1, int x0);
	Polynomial (int x2, int x1, int x0);
	Polynomial (int x3, int x2, int x1, int x0);
	Polynomial (int x4, int x3, int x2, int x1, int x0);
	Polynomial (int x5, int x4, int x3, int x2, int x1, int x0);
	Polynomial (const Polynomial &other);
	~Polynomial ();


	unsigned int degree () const;
	int eval (int x) const;

	static Polynomial binomial (int n, unsigned int pow);


	const int &operator[] (unsigned int pow) const;
	int &operator[] (unsigned int pow);
	int operator() (int x) const;
	Polynomial &operator= (const Polynomial &other);

	wxString str () const;
	friend std::ostream &operator<< (std::ostream &o, const Polynomial &p);
	friend bool operator== (const Polynomial &p1,
						const Polynomial &p2);
	void operator+= (const Polynomial &other);
	Polynomial operator+ (const Polynomial &other) const;
	void operator-= (const Polynomial &other);
	Polynomial operator- (const Polynomial &other) const;
	void operator*= (const Polynomial &other);
	Polynomial operator* (const Polynomial &other) const;
};


#endif	// __POLYNOMIAL_H__
