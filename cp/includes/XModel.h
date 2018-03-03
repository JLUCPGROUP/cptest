#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <ostream>
#include <iostream>
using namespace std;

typedef unsigned u32;
//typedef
//typedef tuple<int, int> IntTuple;
/**
* \brief 表约束语义
*/
enum Semantices {
	SEM_CONFLICT = 0,		///<冲突
	SEM_SUPPORT = 1		///<支持
};

namespace cp {

class XDom {
public:
	const int id;
	vector<int> vals;
	XDom(const int id, const int num_vals, char* val_str);
	~XDom();
	void show();
};

class XVar {
public:
	const int id;
	int xdom_id = -1;
	XVar::XVar(const int id, char* xdom_str);
	void show() const;
	~XVar();
};

class XRel {
public:
	const int id;
	const int arity;
	const int size;
	Semantices semantics;
	vector<vector<int>> tuples;
	XRel(const int id, const int arity, const int size, const char* sem, char* ts_str);
	~XRel();
	void show();
protected:
	char* ts_str_;
	void generate_tuples();
};

class XCon {
public:
	const int id;
	int rel_id = -1;
	const int arity;
	vector<int> scope;
	XCon(const int id, char* rel_str, const int arity, char* scope_str);
	void show();
	~XCon();
};

class XModel {
public:
	XModel() {};
	~XModel();
	vector<XDom*> doms;
	vector<XVar*> vars;
	vector<XRel*> rels;
	vector<XCon*> cons;
	void add(const int id, const int num_vals, char* val_str);
	void add(const int id, char* xdom_id);
	void add(const int id, const int arity, const int size, const char* sem, char* ts_str);
	void add(const int id, char* rel_id, const int arity, char* scope_str);
	void show();
	//int max_domain_size;
	//int max_arity;
};
}

