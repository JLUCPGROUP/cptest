#include <string>
#include "XBuilder.h"
//#include "XBuilder.h"
using namespace std;
//using namespace cp;
const string X_PATH = "BMPath.xml";

const int TimeLimit = 1800000;
int test() {
	cp::XBuilder builder(X_PATH, cp::XRT_BM_PATH);
	//HModel* hm = new HModel();
	//builder.GenerateHModel(hm);
	//MAC3 s(*hm);
	////MAC3rm s(*hm);
	////MAC3bit s(*hm);
	//////lMaxRPC_bit_rm s(*hm);
	//////s.propagate(s.vars, 0);
	//////s.binary_search(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	//s.binary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
	//cout << builder.file_name() << endl;
	//cout << "time = " << s.statistics().solve_time << endl;
	//cout << "positives = " << s.statistics().num_positives << endl;
	//cout << "revisions = " << s.statistics().num_revisions << endl;
	//delete hm;
	return 0;
}
int main() {
	test();
	return 0;
}

