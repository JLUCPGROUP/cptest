#include "XModel.h"


namespace cp {
XDom::XDom(const int id, const int num_vals, char* val_str) :
	id(id) {
	vals.resize(num_vals);
	string s = val_str;
	s += " ";
	string tmp = val_str;
	int start = 0;
	int end = 0;
	const int pre_blankpos = 0;
	string::size_type blankpos = s.find(" ", 0);
	int g_count = 0;
	int value_int;

	if (blankpos == string::npos) {
		sscanf_s(tmp.c_str(), "%d..%d", &start, &end);
		for (int i = start; i <= end; ++i) {
			vals[g_count] = i;
			++g_count;
		}
	}
	else {
		while (blankpos != string::npos) {
			tmp = s.substr(0, blankpos - pre_blankpos);
			s = s.substr(blankpos + 1);
			const string::size_type pointpos = tmp.find(".", 0);
			blankpos = s.find(" ", 0);

			if (pointpos != string::npos) {
				sscanf_s(tmp.c_str(), "%d..%d", &start, &end);

				for (int i = start; i <= end; ++i) {
					vals[g_count] = i;
					++g_count;
				}
			}
			else {
				sscanf_s(tmp.c_str(), "%d", &value_int);
				vals[g_count] = value_int;
				++g_count;
			}
		}
	}
}

XDom::~XDom() {
	vals.clear();
	vector<int>().swap(vals);
}

void XDom::show() {
	cout << id << ": ";
	for (size_t i = 0; i < vals.size(); ++i) {
		cout << vals[i] << " ";
	}
	cout << endl;
}
////////////////////////////////////////////////////////////////
XVar::XVar(const int id, char* xdom_str) :id(id) {
	sscanf_s(xdom_str, "D%d", &xdom_id);
}

void XVar::show() const {
	cout << "id: " << id << " dom_id :" << xdom_id << endl;
}

XVar::~XVar() {}

////////////////////////////////////////////////////////////////
XRel::XRel(const int id, const int arity, const int size, const char* sem, char* ts_str) :
	id(id), arity(arity), size(size), ts_str_(ts_str) {

	switch (strlen(sem)) {
	case 9:
		semantics = SEM_CONFLICT;
		break;
	case 8:
		semantics = SEM_SUPPORT;
		break;
	default:
		break;
	}
	tuples.resize(size, vector<int>(arity));
	generate_tuples();
}

XRel::~XRel() {
	for (auto& t : tuples) {
		t.clear();
		vector<int>().swap(t);
	}
	tuples.clear();
	vector<vector<int>>().swap(tuples);
}

void XRel::show() {
	const string sem = (semantics == SEM_SUPPORT) ? "supports" : "conflicts";
	cout << "id: " << id << " semantics: " << sem << " size: " << tuples.size()
		<< " arity: " << arity << endl;
	for (auto t : tuples) {
		cout << "( ";
		for (auto i : t)
			cout << i << " ";
		cout << ")";
	}
	cout << endl;
}

void XRel::generate_tuples() {
	const char seps[] = " |";
	int i = 0, j = 0;

	for (i = 0; i < size; ++i) {
		for (j = 0; j < arity; ++j) {
			char * ptr = strtok_s(ts_str_, seps, &ts_str_);
			tuples[i][j] = atoi(ptr);
		}
	}
}

////////////////////////////////////////////////////////////////
XCon::XCon(const int id, char* rel_str, const int arity, char* scope_str) :
	id(id), arity(arity) {
	sscanf_s(rel_str, "R%d", &rel_id);
	scope.resize(arity);
	char* context = scope_str;
	const char seps[] = " V";

	for (int i = 0; i < arity; ++i) {
		char * ptr = strtok_s(context, seps, &context);
		scope[i] = atoi(ptr);
	}
}

void XCon::show() {
	cout << "id: " << id << " rel_id: " << rel_id << " arity: " << arity << endl;
	cout << "scope: ";
	for (auto t : scope)
		cout << t << " ";
	cout << endl;
}

XCon::~XCon() {
	scope.clear();
	vector<int>().swap(scope);
}

XModel::~XModel() {
	for (auto& i : doms)
		delete i;
	for (auto& i : vars)
		delete i;
	for (auto& i : rels)
		delete i;
	for (auto& i : cons)
		delete i;
	doms.clear();
	vars.clear();
	rels.clear();
	cons.clear();
}

////////////////////////////////////////////////////////////////

void XModel::add(const int id, const int num_vals, char* val_str) {
	XDom* dom = new XDom(id, num_vals, val_str);
	doms.push_back(dom);
}

void XModel::add(const int id, char* xdom_id) {
	XVar* var = new XVar(id, xdom_id);
	vars.push_back(var);
}

void XModel::add(const int id, const int arity, const int size, const char* sem, char* ts_str) {
	XRel* rel = new XRel(id, arity, size, sem, ts_str);
	rels.push_back(rel);
}

void XModel::add(const int id, char* rel_str, const int arity, char* scope_str) {
	XCon* con = new XCon(id, rel_str, arity, scope_str);
	cons.push_back(con);
}

void XModel::show() {
	cout << "-------------------domains-------------------" << endl;
	for (auto& i : doms)
		i->show();
	cout << "-------------------variables-------------------" << endl;
	for (auto& i : vars)
		i->show();
	cout << "-------------------relations-------------------" << endl;
	for (auto& i : rels)
		i->show();
	cout << "-------------------constraints-------------------" << endl;
	for (auto& i : cons)
		i->show();
}
}
