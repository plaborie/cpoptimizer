// --------------------------------------------------------------------
//
//  File:        dtc_variables.h
//  Date:        02/98
//  Last update: 03/99
//  Description: Global variables used by dtc_evaluate.c
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_VARIABLES_H
#define _DTC_VARIABLES_H

/******************** Variables ********************/

#ifndef _OP_TYPE_
#define _OP_TYPE_
enum op_type { _insert_, _remove_, _mixed_, _bool_qry_, _path_qry_ };
#endif  _OP_TYPE_

enum { _print_type_, _print_axes_, _print_alg_, _print_frmt_, _run_alg_, _gen_random_, _gen_load_ };
enum { _time_, _error_ };
enum { _nodes_, _edges_, _ops_ };
enum { _view_, _latex_, _gnuplot_, _ps_ };
enum { _off_, _on_ };

int	AlgOn[AlgAnz];
string	AlgLabel[AlgAnz];

bool	GenOn		= false;
bool	RunOn		= false;

int	NodeAnz, EdgeAnz;
int	NodeAnzLow	= 0;		/* Number of nodes in the initial graph (low) */
int	NodeAnzHigh	= 0;		/* Number of nodes in the initial graph (high) */
int	NodeStep	= 0;		/* Number of node steps */
int	EdgeAnzLow	= 0;		/* Number of edges in the initial graph (low) */ 
int	EdgeAnzHigh	= 0;		/* Number of edges in the initial graph (high) */ 
int	EdgeStep	= 0;		/* Number of edge steps */ 
int	Acyclic		= _off_;	/* If the graph is acyclic */
int	Simple		= _on_;		/* If the graph is simple */
int	DgrOn		= _off_;	/* If the graph is of bounded degree */
int	Dgr		= 0;		/* Maximum node degree */
int	SubNodeAnz	= 0;		/* Number of nodes in a complete subgraph. */

int	OpAnz		= 0;		/* Length of the sequence of operations */
int	OpPerc		= 50;		/* Percentage of queries */
int	UpdType		= _mixed_;	/* _insert_, _remove_, _mixed_ */
int	UpdPerc		= 50;		/* Percentage of insert operations of all updates */
int	QryType		= _mixed_;	/* _bool_qry_, _path_qry_, _mixed_ */
int	QryPerc		= 50;		/* Percentage of bool queries of all queries */

int	LoadGraph	= _off_;	/* If the graph is loaded from a file */
int	Random		= _on_;		/* If random graphs and opseqs are generated */
int	GraphAnz	= 1;		/* Number of different graphs in the test sequence */
int	OpSeqAnz	= 1;		/* Number of different opseqs per graph in the test sequence */

string	Tmp, GraphFile, OutputFile;
string	SaveDir		= "Data";
string	SessionDir	= "Session-1";
string	TestDir		= "Test";
string	ResultDir	= "Result";

int	LargeStep	= 0;		/* Step in operations (large) */
int	SmallStep	= 0;		/* Step in operations (small) */
int	IterAnz		= 1;		/* Number of iterations to execute each test */
int	CheckOn		= _off_;	/* If the correctness check is on */

int	OutputFormat	= _view_;	/* _view_, _latex_, _gnuplot_, _ps_ */
int	OutputType	= _time_;	/* _time_, _error_ */
int	OutputAxes	= _ops_;	/* _ops_, _nodes_, _edges_ */
int	OutputTime	= 0;		/* 0: ops+init, 1: ops */
int	OutputColor	= _on_;		/* _on_, _off_ */
int	OutputLandscp	= _on_;		/* _on_, _off_ */
int	OutputSolid	= _on_;		/* _on_, _off_ */
int	OutputFSize	= 12;		/* Font size */

#endif _DTC_VARIABLES_H
