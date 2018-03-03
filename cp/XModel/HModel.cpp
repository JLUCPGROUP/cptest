/*
* HModel.cpp
*
*  Created on: 2017年9月26日
*      Author: leezear
*/

#include "HModel.h"

namespace cp {

////////////////////////////////////////////////////////////////////
HVar::HVar(const int id, const int uid, const string name, const int min_val, const int max_val) :
	id(id), uid(uid), name(name), std_max(max_val - min_val) {
	int j = 0;
	const int size = max_val - min_val + 1;
	vals.resize(size);
	anti_map.resize(size);
	for (int i = min_val; i <= max_val; ++i) {
		val_map[i] = j;
		vals[j] = j;
		anti_map[j] = i;
		++j;
	}
}

HVar::HVar(const int id, const int uid, const string name, vector<int>& v) :
	id(id), uid(uid), name(name), std_max(v.size() - 1) {
	vals.resize(v.size());
	anti_map = v;
	for (size_t i = 0; i < vals.size(); ++i) {
		val_map[v[i]] = i;
		vals[i] = i;
	}
}

void HVar::Show() {
	cout << id << "| " << name << ": ";
	for (size_t i = 0; i < vals.size(); ++i)
		cout << vals[i] << "[" << val_map[i] << "] ";
	cout << endl;
}

HVar::~HVar() {}
////////////////////////////////////////////////////////////////////
HTab::HTab(const int id, const bool sem, vector<vector<int>>& ts, vector<HVar*>& scp, const bool STD) :
	id(id), scope(scp), semantics(sem), isSTD(STD) {
	unsigned long all_size = 1;
	for (auto i : scp)
		all_size *= i->vals.size();
	unsigned long sup_size;

	if (!sem)
		sup_size = all_size - ts.size();
	else
		sup_size = ts.size();

	//非标准表
	if (!STD) {
		vector<int> ori_t_(scope.size());
		vector<int> std_t_(scope.size());
		tmp_t_.resize(scope.size());
		tuples.resize(sup_size, vector<int>(scope.size()));

		if (sem) {
			for (size_t i = 0; i < sup_size; i++) {
				GetSTDTuple(ts[i], std_t_);
				tuples[i] = std_t_;
			}
		}
		else {
			int j = 0;
			for (int i = 0; (i < all_size) && (j <= sup_size); ++i) {
				GetTuple(i, ori_t_, std_t_);
				if (find(ts.begin(), ts.end(), ori_t_) == ts.end())
					tuples[j++] = std_t_;
			}
		}
	}
	else {
		tuples = ts;
	}

	semantics = true;
	isSTD = true;
}

HTab::HTab(HTab * t, vector<HVar *>& scp) :
	scope(scp), semantics(t->semantics) {
	id = t->id + 1;
	isSTD = true;
	tuples = t->tuples;
}

//void HTab::GetSTDTuple(vector<int>& tuple) {
//	for (size_t i = 0; i < tuple.size(); ++i)
//		tuple[i] = scope[i]->val_map[tuple[i]];
//}
void HTab::GetSTDTuple(vector<int>& src_tuple, vector<int>& std_tuple) {
	for (size_t i = 0; i < src_tuple.size(); ++i)
		std_tuple[i] = scope[i]->val_map[src_tuple[i]];
}

void HTab::GetORITuple(vector<int>& std_tuple, vector<int>& ori_tuple) {
	for (size_t i = 0; i < std_tuple.size(); ++i)
		ori_tuple[i] = scope[i]->anti_map[std_tuple[i]];
}

int HTab::GetAllSize() const {
	int size = 1;
	for (auto v : scope)
		size *= v->vals.size();
	return size;
}

bool HTab::SAT(vector<int>& t) {
	//return find(tuples.begin(), tuples.end(), t) != tuples.end();
	return binary_search(tuples.begin(), tuples.end(), t);
}

bool HTab::SAT_STD(vector<int>& t) {
	GetORITuple(t, tmp_t_);
	return binary_search(tuples.begin(), tuples.end(), tmp_t_);
}

void HTab::GetTuple(int idx, vector<int>& src_t, vector<int>& std_t) {
	for (int i = (scope.size() - 1); i >= 0; --i) {
		HVar* v = scope[i];
		std_t[i] = idx % v->vals.size();
		src_t[i] = v->anti_map[std_t[i]];
		idx /= v->vals.size();
	}
}

bool HTab::sat(vector<int>& t) {
	return binary_search(tuples.begin(), tuples.end(), t);
}

ostream & operator<<(ostream &os, const vector<HVar*>& a) {
	for (auto v : a)
		os << v->id << "[" << v->name << "] ";
	return os;
}

void HTab::Show() {
	const string sem = semantics ? "supports" : "conflicts";
	cout << "id: " << id << " semantics: " << sem << " size: " << tuples.size()
		<< " arity:" << scope.size() << " scope = {" << scope << endl;
	for (auto t : tuples) {
		cout << "( ";
		for (auto i : t)
			cout << i << " ";
		cout << ")";
	}
	cout << endl;
}
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
HModel::HModel() {
	// TODO Auto-generated constructor stub

}

int HModel::AddVar(const string name, const int min_val, const int max_val) {
	const int id = vars.size();
	const int uid = generate_var_uid();
	HVar* var = new HVar(id, uid, name, min_val, max_val);
	str_var_map_[name] = var;
	int_var_map_[uid] = var;
	vars.push_back(var);
	mds_ = max(mds_, var->vals.size());
	return id;
}

int HModel::AddVar(const string name, vector<int>& v) {
	const int id = vars.size();
	const int uid = generate_var_uid();
	HVar* var = new HVar(id, uid, name, v);
	str_var_map_[name] = var;
	int_var_map_[uid] = var;
	vars.push_back(var);
	mds_ = max(mds_, var->vals.size());
	return id;
}

int HModel::AddTab(const bool sem, vector<vector<int>>& ts, vector<HVar*>& scp, const bool STD) {
	const int id = tabs.size();
	HTab* t = new HTab(id, sem, ts, scp, STD);
	tabs.push_back(t);
	mas_ = max(mas_, t->scope.size());
	subscript(t);
	return id;
}

int HModel::AddTab(const bool sem, vector<vector<int>>& ts, vector<int>& scp) {
	vector<HVar*> scope(scp.size());
	for (int i = 0; i < scp.size(); ++i)
		scope[i] = vars[scp[i]];
	return AddTab(sem, ts, scope);
}

int HModel::AddTab(const bool sem, vector<vector<int>>& ts, vector<string>& scp) {
	vector<HVar*> scope;
	get_scope(scp, scope);
	return AddTab(sem, ts, scope);
}

int HModel::AddTabAsPrevious(HTab* t, vector<string>& scp) {
	vector<HVar*> scope;
	get_scope(scp, scope);
	HTab* nt = new HTab(t, scope);
	tabs.push_back(nt);
	mas_ = max(mas_, nt->scope.size());
	subscript(t);
	return tabs.size() - 1;
}

int HModel::AddTab(const string expr) {
	//cout << expr << endl;
	//表达式栈
	vector<int> expr_stack;
	vector<int> params;
	vector<HVar*> scp;
	vector<int> num_op_params;
	get_postfix(expr, expr_stack, params, num_op_params, scp);
	vector<int> expr_tmp(expr_stack);
	vector<vector<int>> ts;
	vector<int> ori_t(scp.size());
	vector<int> std_t(scp.size());
	unordered_map<HVar*, int> t;
	int num_total_tuples = 1;

	for (auto i : scp)
		num_total_tuples *= i->vals.size();

	for (int i = 0; i < num_total_tuples; ++i) {
		get_ori_tuple_by_index(i, std_t, scp);
		GetORITuple(std_t, ori_t, scp);

		for (int j = 0; j < scp.size(); ++j)
			t[scp[j]] = ori_t[j];

		expr_tmp = expr_stack;

		for (size_t j = 0; j < expr_tmp.size(); j++) {
			//将变量替换为值
			if (get_type(expr_tmp[j]) == ET_VAR)
				//通过uid拿到HVar* 再通过HVar 拿到赋值
				expr_tmp[j] = t[int_var_map_[expr_tmp[j]]];
		}

		t.clear();
		const int result = calculate(expr_tmp, num_op_params);

		if (result)
			ts.push_back(std_t);
	}

	return AddTab(true, ts, scp, true);
}

void HModel::show() {
	cout << "--------------Variables--------------" << endl;
	cout << "size: " << vars.size() << "\tmax domain size :" << mds_ << endl;
	for (auto v : vars)
		v->Show();
	cout << "-------------Constraints-------------" << endl;
	cout << "size: " << tabs.size() << "\tmax arity size :" << mas_ << endl;
	for (auto t : tabs)
		t->Show();
}

int HModel::regist(const string exp_name, function<int(std::vector<int>&)> exp) {
	const int id = generate_exp_uid();
	if (Funcs::str_expr_map.find(exp_name) != Funcs::str_expr_map.end()) {
		Funcs::str_expr_map[exp_name] = id;
		Funcs::int_expr_map[id] = exp;
		return id;
	}
	else {
		cout << "existing" << endl;
		return INT_MIN;
	}
}

int HModel::calculate(vector<int>& expr, vector<int>& params_len) {
	vector<int> res_stack;
	//res_stack.reserve(10);
	int j = -1;

	for (int i = 0; i < expr.size(); ++i) {
		const int op = expr[i];

		//op为参数
		if (op > MAX_OPT)
			res_stack.push_back(op);
		else
			result(op, res_stack, params_len[++j]);
	}

	return res_stack[0];
}

vector<HTab*> HModel::solution_check(vector<int>& sol) {
	if (sol.empty()) {
		cout << "no solution" << endl;
		return vector<HTab*>();
	}
	vector<int> a(sol.size());
	for (int i = 0; i < sol.size(); ++i) {
		a[i] = vars[i]->anti_map[sol[i]];
	}

	for (auto c : a)
		cout << c << " ";

	cout << endl;
	vector<int> tuple(max_arity());
	vector<HTab*> conflict_constraints(tabs.size());
	conflict_constraints.clear();
	tuple.clear();
	for (auto c : tabs) {
		for (auto v : c->scope) {
			tuple.push_back(sol[v->id]);
		}
		if (!c->sat(tuple)) {
			cout << "conflict constraint id = " << c->id << endl;
			conflict_constraints.push_back(c);
		}
		tuple.clear();
	}

	if (conflict_constraints.empty())
		cout << "pass!" << endl;

	return conflict_constraints;
}


void HModel::get_postfix(const string expr, vector<int>& data, vector<int>& params, vector<int>& num_op_params, vector<HVar*>& scp) {
	//转换表达式
	string s = expr;
	string tmp;
	int startpos = 0;
	tuple<ExpType, int> t;
	for (int i = 0; i < s.length(); ++i) {
		switch (s[i]) {
		case '(':
			tmp = s.substr(startpos, i - startpos);
			t = get_type(tmp);
			if (get<0>(t) != ET_NONE) {
				data.push_back(get<1>(t));

				if (get<0>(t) == ET_VAR) {
					params.push_back(get<1>(t));
					if (find(scp.begin(), scp.end(), str_var_map_[tmp]) == scp.end())
						scp.push_back(str_var_map_[tmp]);
				}

				if (get<0>(t) == ET_CONST)
					params.push_back(get<1>(t));
			}
			data.push_back(Funcs::str_expr_map["("]);
			startpos = i + 1;
			break;
		case ')':
			tmp = s.substr(startpos, i - startpos);
			t = get_type(tmp);
			if (get<0>(t) != ET_NONE) {
				data.push_back(get<1>(t));

				if (get<0>(t) == ET_VAR) {
					params.push_back(get<1>(t));
					if (find(scp.begin(), scp.end(), str_var_map_[tmp]) == scp.end())
						scp.push_back(str_var_map_[tmp]);
				}

				if (get<0>(t) == ET_CONST)
					params.push_back(get<1>(t));
			}
			data.push_back(Funcs::str_expr_map[")"]);
			startpos = i + 1;
			break;
		case ',':
			//","不被推入栈
			tmp = s.substr(startpos, i - startpos);

			t = get_type(tmp);
			if (get<0>(t) != ET_NONE) {
				data.push_back(get<1>(t));

				if (get<0>(t) == ET_VAR) {
					params.push_back(get<1>(t));
					if (find(scp.begin(), scp.end(), str_var_map_[tmp]) == scp.end())
						scp.push_back(str_var_map_[tmp]);
				}

				if (get<0>(t) == ET_CONST)
					params.push_back(get<1>(t));
			}
			//data.push_back(Funcs::str_expr_map[","]);
			startpos = i + 1;
			break;
		case ' ':
			startpos = i + 1;
			break;
		default:
			break;
		}
	}

	//统计每个参数个数
	vector<int> postfix_stack(data);
	int last_lpar_idx = 0;
	for (int i = 0; i < postfix_stack.size(); ++i) {
		int op = postfix_stack[i];

		//找到左括号
		if (op == ET_LPAR)
			last_lpar_idx = i;
		//找右括号，后寻找左括号
		else if (op == ET_RPAR) {
			int num = 0;
			for (int j = last_lpar_idx; j < i; ++j) {
				if (postfix_stack[j] > MAX_OPT) {
					postfix_stack[j] = ET_NONE;
					++num;
				}
				else if (postfix_stack[j] < ET_PARAMS&&postfix_stack[j] >ET_COMMA) {
					postfix_stack[j] = ET_NONE;
					++num;
				}
			}
			num_op_params.push_back(num);

			postfix_stack[last_lpar_idx] = ET_NONE;
			const int idx = last_lpar_idx - 1;
			op = postfix_stack[idx];

			//再找下一个左括号
			while (op != ET_LPAR && last_lpar_idx > 0) {
				--last_lpar_idx;
				op = postfix_stack[last_lpar_idx];
			}
		}
	}

	postfix_stack.clear();
	//生成后缀表达式
	last_lpar_idx = 0;
	for (int i = 0; i < data.size(); ++i) {
		int op = data[i];

		//参数与常量压入栈
		if (op > MAX_OPT)
			postfix_stack.push_back(op);

		//找到左括号
		if (op == ET_LPAR)
			last_lpar_idx = i;
		//找右括号，后寻找左括号
		else if (op == ET_RPAR) {
			postfix_stack.push_back(data[last_lpar_idx - 1]);

			data[last_lpar_idx] = ET_NONE;
			const int idx = last_lpar_idx - 1;
			op = data[idx];

			//再找下一个左括号
			while (op != ET_LPAR && last_lpar_idx > 0) {
				--last_lpar_idx;
				op = data[last_lpar_idx];
			}
		}
	}

	data = postfix_stack;
	vector<int>().swap(postfix_stack);
}

tuple<ExpType, int> HModel::get_type(std::string expr) {
	if (expr == "")
		return make_tuple(ET_NONE, Funcs::str_expr_map[expr]);
	if (expr == "(")
		return make_tuple(ET_LPAR, Funcs::str_expr_map[expr]);
	if (expr == ")")
		return make_tuple(ET_RPAR, Funcs::str_expr_map[expr]);
	if (expr == ",")
		return make_tuple(ET_COMMA, Funcs::str_expr_map[expr]);
	if (Funcs::str_expr_map.find(expr) != Funcs::str_expr_map.end())
		return make_tuple(ET_OP, Funcs::str_expr_map[expr]);
	if (expr[0] >= '0'&& expr[0] <= '9')
		return make_tuple(ET_CONST, atoi(expr.c_str()));
	if (str_var_map_.find(expr) != str_var_map_.end())
		return make_tuple(ET_VAR, str_var_map_[expr]->uid);

	cout << "undefined" << endl;
	return make_tuple(ET_NULL, INT_MIN);
}

ExpType HModel::get_type(const int expr) {
	if (expr <= INT_MIN + 3)
		return static_cast<ExpType>(expr);
	if (expr<MAX_OPT&&expr>INT_MIN + 3)
		return ET_OP;
	if (expr > MAX_OPT&& expr < MAX_VALUE)
		return ET_CONST;
	if (expr > MAX_VALUE)
		return ET_VAR;
	return ET_NULL;
}

void HModel::subscript(HTab *t) {
	for (auto v : t->scope)
		subscriptions[v].push_back(t);
	neighbor(t);
}

void HModel::neighbor(HTab* t) {
	if (neighborhoods.empty())
		neighborhoods.resize(vars.size(), vector<vector<int>>(vars.size()));

	for (auto x : t->scope)
		for (auto y : t->scope)
			if (x != y) {
				neighborhoods[x->id][y->id].push_back(t->id);
				if (neighborhoods[x->id][y->id].size() > 1) {
					have_same_scope_ = true;
				}
			}
}

void HModel::get_scope(vector<string>& scp_str, vector<HVar*>& scp) {
	scp.resize(scp_str.size());
	for (int i = 0; i < scp_str.size(); ++i)
		scp[i] = str_var_map_[scp_str[i]];
}

int HModel::get_var_id(const int id) const {
	return var_uid_ - MAX_VALUE - 1;
}

int HModel::generate_exp_uid() {
	return ++exp_id_;
}

int HModel::generate_var_uid() {
	return ++var_uid_;
}

void HModel::GetSTDTuple(vector<int>& src_tuple, vector<int>& std_tuple, vector<HVar*>& scp) {
	for (size_t i = 0; i < src_tuple.size(); ++i)
		std_tuple[i] = scp[i]->val_map[src_tuple[i]];
}

void HModel::GetORITuple(vector<int>& std_tuple, vector<int>& ori_tuple, vector<HVar*>& scp) {
	for (size_t i = 0; i < std_tuple.size(); ++i)
		ori_tuple[i] = scp[i]->anti_map[std_tuple[i]];
}


void HModel::get_ori_tuple_by_index(int idx, std::vector<int>& t, const vector<HVar*> scp) {
	for (int i = scp.size() - 1; i >= 0; --i) {
		const int size = scp[i]->vals.size();
		t[i] = idx%size;
		idx /= size;
	}
}

void HModel::result(const int op, vector<int>& result, const int len) {
	vector<int> a(len);
	for (int i = len - 1; i >= 0; --i) {
		a[i] = result.back();
		result.pop_back();
		//cout << result.size() << endl;
	}
	result.push_back(Funcs::int_expr_map[op](a));
}


HModel::~HModel() {
	for (auto i : vars)
		delete i;
	vars.clear();

	for (auto i : tabs)
		delete i;
	tabs.clear();
}

} /* namespace cp */