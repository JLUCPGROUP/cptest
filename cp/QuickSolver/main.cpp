//#include <string>
//#include "XBuilder.h"
//#include "BacktrackingSearch.h"
//#include "QModel.h"
//#include "Timer.h"
//#include "BitSearch.h"
//using namespace std;
//using namespace cp;
//const string X_PATH = "BMPath.xml";
//
//const int TimeLimit = 1800000;
//int test() {
//	XBuilder builder(X_PATH, XRT_BM_PATH);
//	HModel* m = new HModel();
//	builder.GenerateHModel(m);
//	//MAC3 s(*hm);
//	//MAC3rm s(*m);
//	//MAC3bit s(*m);
//	//RNSQ s(*m);
//	NSAC s(*m);
//	//MAC3_matrix s(*m);
//	//AC3withSAC1bitprocessing s(*m);
//
//	//lMaxRPC_bit_rm s(*m);
//	//s.propagate(s.vars, 0);
//	//s.binary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
//	s.binary_search(Heuristic::Var::VRH_DOM_DDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
//
//	//s.binary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
//	cout << builder.file_name() << endl;
//	cout << "time = " << s.statistics().solve_time << endl;
//	cout << "positives = " << s.statistics().num_positives << endl;
//	cout << "solution = " << s.sol_str << endl;
//	//cout << "revisions = " << s.statistics().num_revisions << endl;
//	cout << "check = " << s.solution_check() << endl;
//
//	//BitSearch ss(*m);
//	//ss.nonbinary_search(Heuristic::Var::VRH_DOM_DDEG_MIN, Heuristic::Val::VLH_MIN, false, TimeLimit);
//	//cout << builder.file_name() << endl;
//	//cout << "initial_propagate_time = " << ss.statistics().initial_propagate_time << endl;
//	//cout << "build_time = " << ss.statistics().build_time << endl;
//	//cout << "search_time = " << ss.statistics().search_time << endl;
//	//cout << "total_time = " << ss.statistics().total_time << endl;
//	//cout << "positives = " << ss.statistics().num_positives << endl;
//	////cout << "nodes = " << ss.statistics().nodes << endl;
//	//cout << "solution = " << ss.sol_str << endl;
//	//cout << "check = " << ss.solution_check() << endl;
//
//	delete m;
//	return 0;
//}
//int main() {
//	test();
//	return 0;
//}