//
//	polynomial.cc
//

#include <iostream>
#include <vector>
#include "math.h"
#include "polynomial.h"



Polynomial::PolynomialRep::PolynomialRep ()
{
	ref = 0;
}

Polynomial::PolynomialRep::PolynomialRep (const PolynomialRep *other)
{
	ref = 0;

	data = other->data;
}

void Polynomial::minimise ()
{
	std::vector<int>::iterator it;

	for (it = rep->data.end () - 1; it != rep->data.begin (); --it)
		if (*it)
			break;
	rep->data.erase (++it, rep->data.end ());
}

void Polynomial::unref ()
{
	if (--rep->ref < 1)
		delete rep;
}

void Polynomial::mutator ()
{
	if (rep->ref < 2)
		return;

	const PolynomialRep *old = rep;
	unref ();
	rep = new PolynomialRep (old);
	++rep->ref;
}

Polynomial::Polynomial ()
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (0);
}

Polynomial::Polynomial (int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
}

Polynomial::Polynomial (int x1, int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
	rep->data.push_back (x1);
	minimise ();
}

Polynomial::Polynomial (int x2, int x1, int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
	rep->data.push_back (x1);
	rep->data.push_back (x2);
	minimise ();
}

Polynomial::Polynomial (int x3, int x2, int x1, int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
	rep->data.push_back (x1);
	rep->data.push_back (x2);
	rep->data.push_back (x3);
	minimise ();
}

Polynomial::Polynomial (int x4, int x3, int x2, int x1, int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
	rep->data.push_back (x1);
	rep->data.push_back (x2);
	rep->data.push_back (x3);
	rep->data.push_back (x4);
	minimise ();
}

Polynomial::Polynomial (int x5, int x4, int x3, int x2, int x1, int x0)
{
	rep = new PolynomialRep ();
	++rep->ref;

	rep->data.push_back (x0);
	rep->data.push_back (x1);
	rep->data.push_back (x2);
	rep->data.push_back (x3);
	rep->data.push_back (x4);
	rep->data.push_back (x5);
	minimise ();
}

Polynomial::Polynomial (const Polynomial &other)
{
	rep = other.rep;
	++rep->ref;
}

Polynomial::~Polynomial ()
{
	unref ();
}

unsigned int Polynomial::degree () const
{
	return (rep->data.size () - 1);
}

int Polynomial::eval (int x) const
{
	std::vector<int>::reverse_iterator it;
	int val;

	val = 0;
	for (it = rep->data.rbegin (); it != rep->data.rend (); ++it)
		val = val * x + *it;
	return val;
}

Polynomial Polynomial::binomial (int n, unsigned int pow)
{
	Polynomial p;
	int i, npow;

	npow = 1;
	for (i = (signed) pow; i >= 0; --i) {
		p[i] = Math::choose (pow, i) * npow;
		npow *= n;
	}

	return p;
}

const int &Polynomial::operator[] (unsigned int pow) const
{
	// TODO: throw exception on out-of-bounds
	// if (degree () < pow)
	//	return 0;
	return rep->data[pow];
}

int &Polynomial::operator[] (unsigned int pow)
{
	mutator ();

	if (degree () < pow)
		rep->data.resize (pow + 1);
	return rep->data[pow];
}

int Polynomial::operator() (int x) const
{
	return eval (x);
}

Polynomial &Polynomial::operator= (const Polynomial &other)
{
	if (this == &other)
		return *this;

	rep = other.rep;
	++rep->ref;

	return *this;
}

wxString Polynomial::str () const
{
	wxString s;
	std::vector<int>::reverse_iterator it;
	int pow, val;
	bool sp;

	pow = rep->data.size ();
	sp = false;
	for (it = rep->data.rbegin (); it != rep->data.rend (); ++it) {
		--pow;
		val = *it;

		if (!val)
			continue;
		if (sp && (val > 0))
			s << wxT(" + ");
		else if (sp && (val < 0)) {
			s << wxT(" - ");
			val = -val;
		}

		if ((val != 1) || (pow == 0))
			s << val;

		if (pow > 1) {
			s << wxT("x^") << pow;
		} else if (pow == 1)
			s << wxT("x");

		sp = true;
	}

	if (!sp)
		s << wxT("0");

	return s;
}

std::ostream &operator<< (std::ostream &o, const Polynomial &p)
{
	std::vector<int>::reverse_iterator it;
	int pow, val;
	bool sp;

	pow = p.rep->data.size ();
	sp = false;
	for (it = p.rep->data.rbegin (); it != p.rep->data.rend (); ++it) {
		--pow;
		val = *it;

		if (!val)
			continue;
		if (sp && (val > 0))
			o << " + ";
		else if (sp && (val < 0)) {
			o << " - ";
			val = -val;
		}

		if ((val != 1) || (pow == 0))
			o << val;

		if (pow > 1) {
			o << "x^" << pow;
		} else if (pow == 1)
			o << 'x';

		sp = true;
	}

	if (!sp)
		o << '0';

	return o;
}

bool operator== (const Polynomial &p1, const Polynomial &p2)
{
	if (p1.rep == p2.rep)
		return true;
	if (p1.degree () != p2.degree ())
		return false;
	if (!(p1.rep->data == p2.rep->data))
		return false;
	return true;
}

void Polynomial::operator+= (const Polynomial &other)
{
	int d1, d2, i;

	mutator ();

	d1 = degree ();
	d2 = other.degree ();

	if (d1 > d2) {
		for (i = 0; i <= d2; ++i)
			(*this) [i] += other[i];
	} else {
		rep->data.reserve (d2 + 1);
		for (i = 0; i <= d1; ++i)
			(*this) [i] += other[i];
		for (; i <= d2; ++i)
			(*this) [i] = other[i];
	}

	minimise ();
}

Polynomial Polynomial::operator+ (const Polynomial &other) const
{
	Polynomial ret (*this);

	ret += other;
	return ret;
}

void Polynomial::operator-= (const Polynomial &other)
{
	int d1, d2, i;

	mutator ();

	d1 = degree ();
	d2 = other.degree ();

	if (d1 > d2) {
		for (i = 0; i <= d2; ++i)
			rep->data[i] -= other[i];
	} else {
		rep->data.reserve (d2 + 1);
		for (i = 0; i <= d1; ++i)
			rep->data[i] -= other[i];
		for (; i <= d2; ++i)
			rep->data[i] = -other[i];
	}

	minimise ();
}

Polynomial Polynomial::operator- (const Polynomial &other) const
{
	Polynomial ret (*this);

	ret -= other;
	return ret;
}

void Polynomial::operator*= (const Polynomial &other)
{
	Polynomial wk;
	int i, j, d1, d2, dmax;

	d1 = degree ();
	d2 = other.degree ();
	dmax = (d1 > d2) ? d1 : d2;

	// Operand scanning method
	for (i = dmax; i >= 0; --i)
		wk[i] = 0;
	for (i = 0; i <= d1; ++i)
		for (j = 0; j <= d2; ++j)
			wk[i+j] += rep->data[i] * other[j];

	unref ();
	rep = wk.rep;
	++rep->ref;

	minimise ();
}

Polynomial Polynomial::operator* (const Polynomial &other) const
{
	Polynomial ret (*this);

	ret *= other;
	return ret;
}
