//
//	unit_test.cc
//

#include <iostream>
#include <vector>
#include "edge.h"
#include "factory.h"
#include "graph.h"
#include "polynomial.h"
#include "matrix.h"
#include "vertex.h"


// Kludge -- fixes linking error, since I don't want to include parsing
#include <fstream>
std::fstream *yy_gt_fs;
int yy_gt_parse (void) { return 0; }
int yy_gt_debug;
Graph *new_graph;

void setProgress (double frac) { }


int tests = 0, errors = 0;
int subtests = 0;
char *test_name;


#define TEST_FAIL		\
	std::cerr << "\n\t* Failure at line " << __LINE__ << '\n'
#define TEST(test)		\
		++subtests;	\
		if (!(test)) { 	\
			++errors; TEST_FAIL;	\
			std::cerr << "\t\t(" #test ")\n";	\
		} else {	\
			std::cerr << ".";	\
		}
#define TEST1(test,param)	\
		++subtests;	\
		if (!(test)) {	\
			++errors; TEST_FAIL;	\
			std::cerr << "\t\t(" #test "), " << param << "\n"; \
		} else {	\
			std::cerr << ".";	\
		}
#define START_TEST(name)		\
		++tests;		\
		test_name = (name);	\
		std::cerr << "Running test " << tests << ": '" << test_name \
							<< "': "
#define END_TEST(name)			\
		std::cerr << "\n";


void test_Polynomials ()
{
	Polynomial p1 (7, 2, -3, 4), p2 (4, 0, 6, 0);
	TEST ((p1[0] == 4) && (p2[3] == 4));
	TEST (p1.degree () == 3);
	TEST ((p1.eval (-1) == 2) && (p2 (3) == 126));
	TEST ((p1 + p2) == Polynomial (11, 2, 3, 4));
	TEST ((p1 - p2) == Polynomial (3, 2, -9, 4));
	TEST ((p1 * p2).degree () == 6);
	TEST ((p1 * p2) (5) == 484420);
}

void test_Matrices ()
{
	Matrix m (2, 2), n (2, 2), o (2, 2);
	m (0, 0) = 1; m (0, 1) = 2; m (1, 0) = 3; m (1, 1) = 4;
	n (0, 0) = 5; n (0, 1) = 7; n (1, 0) = 8; n (1, 1) = 6;
	o (0, 0) = 6; o (0, 1) = 9; o (1, 0) = 11; o (1, 1) = 10;
	TEST (m (0, 1) == 2);
	TEST (n (1, 0) == 8);
	TEST ((m + n) == o);
	o (0, 0) = -4; o (0, 1) = -5; o (1, 0) = -5; o (1, 1) = -2;
	TEST ((m - n) == o);
	o (0, 0) = 6; o (0, 1) = 9; o (1, 0) = 11; o (1, 1) = 10;
	Matrix mn = m * n;
	TEST (mn (0, 0) == 21);
	TEST (mn (0, 1) == 19);
	TEST (mn (1, 0) == 47);
	TEST (mn (1, 1) == 45);
	Matrix no = n * o;
	TEST (no (0, 0) == 107);
	TEST (no (0, 1) == 115);
	TEST (no (1, 0) == 114);
	TEST (no (1, 1) == 132);
	Matrix onm = o * n * m;
	TEST (onm (0, 0) == 390);
	TEST (onm (0, 1) == 588);
	TEST (onm (1, 0) == 546);
	TEST (onm (1, 1) == 818);
	Matrix r (4, 4);
	r (0, 0) = 2; r (0, 1) = 6; r (0, 2) = -3; r (0, 3) = 7;
	r (1, 0) = -4; r (1, 1) = 3; r (1, 2) = 2; r (1, 3) = 4;
	r (2, 0) = 0; r (2, 1) = -6; r (2, 2) = 1; r (2, 3) = -2;
	r (3, 0) = 1; r (3, 1) = 8; r (3, 2) = 0; r (3, 3) = -9;
	std::vector<Matrix> m_powers;
	m_powers.push_back (r);
	for (int i = 1; i < 8; ++i)
		m_powers.push_back (m_powers[i - 1] * r);
	Matrix t (4, 4);
	t (0, 0) = 26133615; t (0, 1) = 82601560;
		t (0, 2) = -5961481; t (0, 3) = -128891807;
	t (1, 0) = 19529336; t (1, 1) = 64384673;
		t (1, 2) = 2412896; t (1, 3) = -141162224;
	t (2, 0) = 4231758; t (2, 1) = -11702652;
		t (2, 2) = -868391; t (2, 3) = -7138728;
	t (3, 0) = -80539827; t (3, 1) = -168196902;
		t (3, 2) = 10811601; t (3, 3) = 395518792;
	TEST (m_powers[7] == t);
}

void test_Graph ()
{
	Graph *g = new Graph;
	g->add (new Vertex ("A", 20, 30));
	g->add (new Vertex ("B", 50, 20));
	g->add (new Vertex ("Charlie", 200, 80));
	g->add (new Vertex ("D", 40, 50));
	TEST (g->find ("A"))
	TEST (g->find ("B"))
	TEST (g->find ("Charlie"))
	//TEST (g->find (48, 22) == g->find ("B"))
	TEST (!g->find ("A-dup"))
	TEST (!g->find ("C"))
	TEST (g->find ("D"))
	g->remove (g->find ("D"));
	TEST (!g->find ("D"))
	TEST (!g->are_adjacent (g->find ("A"), g->find ("B")));
	g->add (new Edge (g->find ("A"), g->find ("B")));
	TEST (g->are_adjacent (g->find ("A"), g->find ("B")));
	g->remove (*(g->find ("A")->e_begin ()));
	TEST (!g->are_adjacent (g->find ("A"), g->find ("B")));
	delete g;
}

void test_Chromatic ()
{
	const static int MAX_TREE = 7, MAX_CYCLE = MAX_TREE;
	Graph *g;

	// Set up basics
	Polynomial T[MAX_TREE + 1];
	T[1] = Polynomial (1, 0);
	for (int i = 2; i <= MAX_TREE; ++i)
		T[i] = T[i - 1] * Polynomial (1, -1);
	Polynomial C[MAX_CYCLE + 1];
	C[3] = Polynomial (1, -3, 2, 0);
	for (int i = 4; i <= MAX_CYCLE; ++i)
		C[i] = T[i] - C[i - 1];

	// Base cases
	for (int i = 1; i < 5; ++i) {
		g = Factory::N (1);
		TEST1 (g->chromatic_number () == 1, "i = " << i)
		delete g;
	}

	// Cyclic graphs
	for (int i = 3; i <= MAX_CYCLE; ++i) {
		g = Factory::C (i);
		TEST1 (g->chromatic_number () == ((i % 2) + 2), "i = " << 1);
		TEST1 (g->chromatic_polynomial () == C[i], "i = " << i);
		delete g;
	}
	// Wheel graphs
	g = Factory::W (5);
	TEST (g->chromatic_polynomial () == C[3] * Polynomial (1, -5, 7));
	delete g;
	g = Factory::W (6);
	TEST (g->chromatic_polynomial () ==
			C[3] * Polynomial (1, -3) * Polynomial (1, -4, 5));
	delete g;
	// Other graphs
	g = Factory::Cubical ();
	{
		Polynomial p (-11, 55, -159, 282, -290, 133);
		p[6] = 1;
		TEST (g->chromatic_polynomial () ==
				p * Polynomial (1, -1, 0));
	}
	delete g;
	g = Factory::Lattice (4, 4);
	{
		int coeff[16] = {
			-17493, 112275, -346274, 682349,
			-960627, 1022204, -848056, 557782,
			-292883, 122662, -40614, 10437,
			-2015, 276, -24, 1 };
		Polynomial p (0);
		for (int i = 0; i < 16; ++i)
			p[i + 1] = coeff[i];
		TEST (g->chromatic_polynomial () == p);
	}
	delete g;
	g = Factory::Petersen ();
	{
		int coeff[10] = {
			-704, 2606, -4305, 4275, -2861,
			1353, -455, 105, -15, 1 };
		Polynomial p (0);
		for (int i = 0; i < 10; ++i)
			p[i + 1] = coeff[i];
		TEST (g->chromatic_polynomial () == p);
	}
}

int main ()
{

	std::cerr << "-----------------------------\n";
	std::cerr << "  GraphThing unit testing\n";
	std::cerr << "-----------------------------\n";


	//*************** START OF TESTS ****************

	//*********************************************
	START_TEST ("Polynomials");
	test_Polynomials ();
	END_TEST ("Polynomials");

	//*********************************************
	START_TEST ("Matrices");
	test_Matrices ();
	END_TEST ("Matrices");

	//*********************************************
	START_TEST ("Graph manipulation");
	test_Graph ();
	END_TEST ("Graph manipulation");

	//*********************************************
	START_TEST ("Chromatic polynomials");
	test_Chromatic ();
	END_TEST ("Chromatic polynomials");


	//*************** END OF TESTS ****************

	std::cerr << "\n";
	std::cerr << "-----------------------------\n";
	std::cerr << "Total tests: " << tests << '\n';
	std::cerr << "     Errors: " << errors << '\n';
	std::cerr << '\n';
	std::cerr << "  (Subtests: " << subtests << ")\n";
	std::cerr << "-----------------------------\n";

	return 0;
}
