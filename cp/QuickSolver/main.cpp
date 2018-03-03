#include <string>
#include "XBuilder.h"
#include "BacktrackingSearch.h"
#include "QModel.h"
using namespace std;
using namespace cp;
const string X_PATH = "BMPath.xml";

const int TimeLimit = 1800000000000;
int main() {
	XBuilder builder(X_PATH, XRT_BM_PATH);
	HModel* hm = new HModel();
	builder.GenerateHModel(hm);
	//QVar v(hm->vars[0]);
	//cout << v.size(0) << endl;
	lMaxRPC_bit_rm s(hm);
	//auto ps = s.propagate(s.vars, 0);
	s.solve(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	cout << "time = " << s.statistics().solve_time << endl;
	cout << "positive = " << s.statistics().num_positive << endl;
	cout << "negative = " << s.statistics().num_negative << endl;
	delete hm;
	return 0;
}

