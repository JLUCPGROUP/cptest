//#pragma once
//#include <string>
//#include <windows.h>
//#include <io.h>  
//#include "XBuilder.h"
//#include <fstream>
//#include "BacktrackingSearch.h"
//#include <chrono>
//#include "BitSearch.h"
//using namespace cp;
//using namespace std;
//
//#define LOGFILE
//const string XPath = "BMPath.xml";
//const int64_t TimeLimit = 1800100;
//const string X_PATH = R"(E:\Projects\benchmarks\)";
//const string bmp_ext = ".xml";
//void getFilesAll(const string& path, vector<string>& files);
//bool getScheme(char ** argv, SearchScheme& ss);
//
//
//int main(const int argc, char ** argv) {
//
//	if (argc <= 3) {
//		std::cout << "no argument" << endl;
//		return 0;
//	}
//	SearchScheme ss;
//	const bool no_error = getScheme(argv, ss);
//	if (!no_error) {
//		cout << "error" << endl;
//		return 0;
//	}
//	vector<string> files;
//	getFilesAll(X_PATH + argv[1], files);
//	auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
//#ifdef LOGFILE
//	ofstream lofi;
//	const string bm_res = X_PATH + "res2\\FC\\" + ss.vrh_str + "\\" + argv[1] + "-" + std::to_string(tt) + ".csv";
//	lofi.open(bm_res, ios::out | ios::trunc);
//	cout << bm_res << endl;
//	if (!lofi.is_open())
//		return 0;
//	lofi << "files" << "," << "cpu" << "," << "#nodes" << "," << "test" << "," << "solution" << endl;
//#endif
//	double ts = 0;
//	double tn = 0;
//	u64 to = 0;
//	for (const auto f : files) {
//		cout << f << endl;
//		XBuilder builder(f, XRT_BM);
//		HModel* m = new HModel();
//		builder.GenerateHModel(m);
//
//		BitSearch s(*m);
//		s.nonbinary_search(ss.vrh, Heuristic::Val::VLH_MIN, true, TimeLimit);
//		const bool test = s.solution_check();
//#ifdef LOGFILE
//		lofi << builder.file_name() << "," << s.statistics().total_time << "," << s.statistics().num_positives << "," << test << "," << s.sol_str << endl;
//#endif
//		ts += s.statistics().total_time;
//		tn += s.statistics().num_positives;
//		if (s.statistics().total_time > TimeLimit)
//			++to;
//
//		delete m;
//	}
//	const double avg_ts = ts / files.size() / 1000;
//	const double avg_tn = tn / files.size() / 1000000;
//#ifdef LOGFILE
//	lofi << avg_ts << endl;
//	lofi << avg_tn << endl;
//	lofi << to << endl;
//	lofi.close();
//#endif
//	return 0;
//}
//
//
//void getFilesAll(const string& path, vector<string>& files) {
//	//文件句柄 
//	intptr_t  h_file = 0;
//	//文件信息 
//	struct _finddata_t fileinfo;
//	string p;
//	if ((h_file = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
//		do {
//			if ((fileinfo.attrib & _A_SUBDIR)) {
//				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
//					//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
//					getFilesAll(p.assign(path).append("\\").append(fileinfo.name), files);
//				}
//			}
//			else {
//				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
//			}
//		} while (_findnext(h_file, &fileinfo) == 0);
//		_findclose(h_file);
//	}
//}
//
//bool getScheme(char ** argv, SearchScheme& ss) {
//	ss.ds_str = argv[2];
//	ss.vrh_str = argv[3];
//	//ss.vlh_str = argv[4];
//
//	if (ss.ds_str == "2") {
//		ss.ds = Heuristic::DS_BI;
//	}
//	else if (ss.ds_str == "n") {
//		ss.ds = Heuristic::DS_NB;
//	}
//	else {
//		return false;
//	}
//
//	if (ss.vrh_str == "d") {
//		ss.vrh = Heuristic::VRH_DOM_MIN;
//	}
//	else if (ss.vrh_str == "dg") {
//		ss.vrh = Heuristic::VRH_DOM_DEG_MIN;
//	}
//	else if (ss.vrh_str == "wd") {
//		ss.vrh = Heuristic::VRH_DOM_WDEG_MIN;
//	}
//	else {
//		return false;
//	}
//
//	return true;
//}