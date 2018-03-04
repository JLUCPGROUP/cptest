#include <string>
#include "XBuilder.h"
#include "BacktrackingSearch.h"
#include "QModel.h"
using namespace std;
using namespace cp;
const string X_PATH = "BMPath.xml";

const int TimeLimit = 1800000;
int main() {
	XBuilder builder(X_PATH, XRT_BM_PATH);
	HModel* hm = new HModel();
	builder.GenerateHModel(hm);
	MAC3bit s(hm);
	//s.solve(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	s.solve(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	cout << "time = " << s.statistics().solve_time << endl;
	cout << "positives = " << s.statistics().num_positives << endl;
	cout << "revisions = " << s.statistics().num_revisions << endl;
	delete hm;
	return 0;
}

