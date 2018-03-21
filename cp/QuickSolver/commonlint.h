#pragma once
#include "BacktrackingSearch.h"
#include <corecrt_io.h>
using namespace std;
using namespace cp;
void getFilesAll(const string path, vector<string>& files) {
	//文件句柄 
intptr_t  h_file = 0;
//文件信息 
struct _finddata_t fileinfo;
string p;
if ((h_file = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
	do {
		if ((fileinfo.attrib & _A_SUBDIR)) {
			if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
				//files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
				getFilesAll(p.assign(path).append("\\").append(fileinfo.name), files);
			}
		}
		else {
			files.push_back(p.assign(path).append("\\").append(fileinfo.name));
		}
	} while (_findnext(h_file, &fileinfo) == 0);
	_findclose(h_file);
}
}

bool getScheme(char ** argv, cp::SearchScheme& ss) {
	ss.ds_str = argv[2];
	ss.vrh_str = argv[3];
	//ss.vlh_str = argv[4];

	if (ss.ds_str == "2") {
		ss.ds = Heuristic::DS_BI;
	}
	else if (ss.ds_str == "n") {
		ss.ds = Heuristic::DS_NB;
	}
	else {
		return false;
	}

	if (ss.vrh_str == "d") {
		ss.vrh = Heuristic::VRH_DOM_MIN;
	}
	else if (ss.vrh_str == "dg") {
		ss.vrh = Heuristic::VRH_DOM_DEG_MIN;
	}
	else if (ss.vrh_str == "wd") {
		ss.vrh = Heuristic::VRH_DOM_WDEG_MIN;
	}
	else if (ss.vrh_str == "lx") {
		ss.vrh = Heuristic::VRH_LEX;
	}
	else if (ss.vrh_str == "deg") {
		ss.vrh = Heuristic::VRH_DEG_MIN;
	}
	else {
		return false;
	}

	return true;
}