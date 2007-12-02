/****   , [ MatrixGame.cpp ], 
Copyright (c) 2007 Universite d'Orleans - Arnaud Lallouet 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
 *************************************************************************/

#include <cstdlib> /* pour srand, rand et RAND_MAX */
#include <ctime> /* pour time */
#include <math.h> /* pour pow */

#include <iostream>

#include "gecode/minimodel.hh"
#include "gecode/int/element.hh"

#include "qsolver.hh"
#include "implicative.hh"
#include "scoreSDF.hh"
#include "NaiveValueHeuristics.hh"

#define UNIVERSAL true
#define EXISTENTIAL false

using namespace MiniModel;

int main (int argc, char * const argv[]) {
    
    int depth = 6;  // � 8, on sature la m�moire
    int nbDecisionVar = 2*depth;
    int nbScope = nbDecisionVar+1;
    int boardSize = (int)pow((double)2,(double)depth);
    
    std::srand(std::time(NULL));
    
    IntArgs board(boardSize*boardSize);
    for (int i=0; i<boardSize; i++)
        for (int j=0; j<boardSize; j++)
            board[j*boardSize+i] = (int)( (double)rand()  /  ((double)RAND_MAX + 1) * 50 ) < 25 ? 0:1;
    
    IntArgs access(nbDecisionVar);
    access[nbDecisionVar-1]=1;
    access[nbDecisionVar-2]=boardSize;
    for (int i=nbDecisionVar-3; i>=0; i--)
        access[i]=access[i+2]*2;
	
    // debug
    for (int i=0; i<boardSize; i++)
    {
        for (int j=0; j<boardSize; j++)
            cout << board[j*boardSize+i] << " ";
        cout << endl;
	}
    cout  << endl;
    for (int i=0; i<nbDecisionVar; i++)
        cout << access[i] << " ";
    cout << endl;
    // end debug
    
    int *scopesSize = new int[nbScope];
    for (int i=0; i<nbScope-1; i++)
        scopesSize[i]=1;
    scopesSize[nbScope-1]=2;
    
    Implicative p(nbScope, EXISTENTIAL, scopesSize, 2);
    
    // Defining the variable of the n first scopes ...
    for (int i=0; i<nbDecisionVar; i++)
    {
        p.QIntVar(i, 0, 1);
        p.nextScope();
    }
	
    // Declaring last scope variables ...
    
    p.QIntVar(nbDecisionVar, 0, 1);
    p.QIntVar(nbDecisionVar+1, 0, boardSize*boardSize);
    p.nextScope();
    // Body
    
    post(p.space(), p.var(nbDecisionVar) == 1);
    
    IntVarArgs X(nbDecisionVar);
    for (int i=0; i<nbDecisionVar; i++)
        X[i]=p.var(i);
    
    linear(p.space(), access, X, IRT_EQ, p.var(nbDecisionVar+1));
    //  MiniModel::LinRel R(E, IRT_EQ, MiniModel::LinExpr(p.var(nbDecisionVar+1)));
    element(p.space(), board, p.var(nbDecisionVar+1), p.var(nbDecisionVar));
    
    // When every variables and constraints have been declared, the makeStructure method
    // must be called in order to lead the problem ready for solving.
    p.makeStructure();
    
    /*
     cout << (p.quantification(0)==UNIVERSAL? "universal" : "existential") << "-0\n";
     cout << "  " << (p.quantification(1)==UNIVERSAL? "universal" : "existential") << "-1\n";
     cout << "    " << (p.quantification(2)==UNIVERSAL? "universal" : "existential") << "-2\n";
     cout << "      " << (p.quantification(3)==UNIVERSAL? "universal" : "existential") << "-3\n";
     cout << "        " << (p.quantification(4)==UNIVERSAL? "universal" : "existential") << "-4\n";
     
     */
    
    // We will use a Smallest Domain First branching heuristic for solving this problem.
    sdf heur;
    SmallestValueFirst v_heur;
    
    // So, we build a quantified solver for our problem p, using the heuristic we just created.
    QSolver solver(&p,&heur, &v_heur);
    
    unsigned long int nodes=0;
    unsigned long int steps=0;
    
    // then we solve the problem. Nodes and Steps will contain the number of nodes encountered and
    // of propagation steps achieved during the solving.
    bool outcome = solver.solve(nodes,steps);
    
    cout << "  outcome: " << ( outcome? "TRUE" : "FALSE") << endl;
    cout << "  nodes visited: " << nodes << " " << steps << endl;
    
    return outcome? 10:20;
    
}
