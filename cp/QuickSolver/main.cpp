﻿#include <string>
#include "XBuilder.h"
#include "BacktrackingSearch.h"
#include "QModel.h"
#include "Timer.h"
#include "BitSearch.h"
using namespace std;
using namespace cp;
const string X_PATH = "BMPath.xml";

const int TimeLimit = 1800000;
int test() {
	XBuilder builder(X_PATH, XRT_BM_PATH);
	HModel* m = new HModel();
	builder.GenerateHModel(m);
	//MAC3 s(*hm);
	//MAC3rm s(*hm);
	//MAC3bit s(*m);
	////lMaxRPC_bit_rm s(*hm);
	//s.propagate(s.vars, 0);
	////s.binary_search(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	//s.binary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	//cout << builder.file_name() << endl;
	//cout << "time = " << s.statistics().solve_time << endl;
	//cout << "positives = " << s.statistics().num_positives << endl;
	//cout << "revisions = " << s.statistics().num_revisions << endl;

	//SAC1bit s(*m);
	//Timer t;
	//s.propagate(s.vars, 0);
	//const u64 pt = t.elapsed();
	//cout << "del = " << s.num_del() << endl;
	//cout << "time = " << pt << endl;
	BitSearch s(*m);
	s.nonbinary_search(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, false, TimeLimit);
	//cout << builder.file_name() << endl;
	//cout << "time = " << s.statistics().solve_time << endl;
	//cout << "positives = " << s.statistics().num_positives << endl;
	//cout << "revisions = " << s.statistics().num_revisions << endl;
	delete m;
	return 0;
}
int main() {
	test();
	return 0;
}

