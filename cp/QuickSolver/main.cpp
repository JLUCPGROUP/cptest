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
//	//MAC3rm s(*hm);
//	//MAC3bit s(*m);
//	////lMaxRPC_bit_rm s(*hm);
//	//s.propagate(s.vars, 0);
//	//s.binary_search(Heuristic::Var::VRH_DOM_WDEG_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
//	////s.binary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, TimeLimit);
//	//cout << builder.file_name() << endl;
//	//cout << "time = " << s.statistics().solve_time << endl;
//	//cout << "positives = " << s.statistics().num_positives << endl;
//	//cout << "solution = " << s.get_solution_str() << endl;
//	//cout << "revisions = " << s.statistics().num_revisions << endl;
//
//	BitSearch ss(*m);
//	ss.nonbinary_search(Heuristic::Var::VRH_DOM_MIN, Heuristic::Val::VLH_MIN, false, TimeLimit);
//	cout << builder.file_name() << endl;
//	cout << "initial_propagate_time = " << ss.statistics().initial_propagate_time << endl;
//	cout << "build_time = " << ss.statistics().build_time << endl;
//	cout << "search_time = " << ss.statistics().search_time << endl;
//	cout << "total_time = " << ss.statistics().total_time << endl;
//	cout << "positives = " << ss.statistics().num_positives << endl;
//	cout << "solution = " << ss.get_solution_str() << endl;
//	delete m;
//	return 0;
//}
//int main() {
//	test();
//	return 0;
//}
//
////void getFilesAll(const string path, vector<string>& files) {
////	//文件句柄 
////	intptr_t  h_file = 0;
////	//文件信息 
////	struct _finddata_t fileinfo;
////	string p;
////	if ((h_file = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
////		do {
////			if ((fileinfo.attrib & _A_SUBDIR)) {
////				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
////					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
////					getFilesAll(p.assign(path).append("\\").append(fileinfo.name), files);
////				}
////			}
////			else {
////				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
////			}
////		} while (_findnext(h_file, &fileinfo) == 0);
////		_findclose(h_file);
////	}
////}
////
////
////bool getScheme(char ** argv, SearchScheme& ss) {
////	ss.ds_str = argv[2];
////	ss.vrh_str = argv[3];
////	//ss.vlh_str = argv[4];
////
////	if (ss.ds_str == "2") {
////		ss.ds = Heuristic::DS_BI;
////	}
////	else if (ss.ds_str == "n") {
////		ss.ds = Heuristic::DS_NB;
////	}
////	else {
////		return false;
////	}
////
////	if (ss.vrh_str == "d") {
////		ss.vrh = Heuristic::VRH_DOM_MIN;
////	}
////	else if (ss.vrh_str == "dg") {
////		ss.vrh = Heuristic::VRH_DOM_DEG_MIN;
////	}
////	else if (ss.vrh_str == "wd") {
////		ss.vrh = Heuristic::VRH_DOM_WDEG_MIN;
////	}
////	else {
////		return false;
////	}
////
////	return true;
////}