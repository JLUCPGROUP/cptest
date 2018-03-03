/*
* HModel.h
*
*  Created on: 2017年9月26日
*      Author: leezear
*/

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <cmath>
#include <numeric>
#include <valarray>
#include <iostream>

namespace cp {

const int MAX_VALUE = INT_MAX - 4096;
const int MAX_OPT = INT_MIN + 4096;
const int MIN_USER_OPT = MAX_OPT - 1024;

enum ExpType {
	ET_OP,
	ET_CONST,
	ET_VAR,
	ET_NONE = INT_MIN,
	ET_LPAR = INT_MIN + 1,
	ET_RPAR = INT_MIN + 2,
	ET_COMMA = INT_MIN + 3,
	ET_PARAMS = MAX_OPT,
	ET_MARK,
	ET_NULL
};

enum ConType {
	CT_EXT, CT_INT
};

typedef std::unordered_map<int, std::function<int(std::vector<int>&)>> func_map;

namespace Funcs {
//auto Opposite = 
//unordered_map<string, function<int(int, int)>> predictions = {
//	{ '*', [](int i, int j) {return i * j; } },
//};
//auto mod = [](int i, int j) {return i%j; };
namespace ops {
static auto nullexp = [](std::vector<int>& a) {return INT_MIN; };
static auto neg = [](std::vector<int>& a) {return 0 - a[0]; };
static auto abs = [](std::vector<int>& a) {return std::abs(a[0]); };
static auto add = [](std::vector<int>& a) {return accumulate(a.begin(), a.end(), 0); };
static auto sub = [](std::vector<int>& a) {return a[0] - a[1]; };
static auto mul = [](std::vector<int>& a) {return accumulate(a.begin(), a.end(), 1, std::multiplies<int>()); };
static auto div = [](std::vector<int>& a) {return a[0] / a[1]; };
static auto mod = [](std::vector<int>& a) {return a[0] % a[1]; };
static auto sqr = [](std::vector<int>& a) {return static_cast<int>(sqrt(a[0])); };
static auto pow = [](std::vector<int>& a) {return std::pow(a[0], a[1]); };
static auto min = [](std::vector<int>& a) {return *min_element(a.begin(), a.end()); };
static auto max = [](std::vector<int>& a) {return *max_element(a.begin(), a.end()); };
static auto dist = [](std::vector<int>& a) {return std::abs(a[0] - a[1]); };

static auto le = [](std::vector<int>& a) {return a[0] <= a[1]; };
static auto lt = [](std::vector<int>& a) {return a[0] < a[1]; };
static auto ge = [](std::vector<int>& a) {return a[0] >= a[1]; };
static auto gt = [](std::vector<int>& a) {return a[0] > a[1]; };
static auto ne = [](std::vector<int>& a) {return a[0] != a[1]; };
static auto eq = [](std::vector<int>& a) {return std::all_of(a.begin(), a.end(), [&a](int n) {return n == a[0]; }); };

static auto not = [](std::vector<int>& a) {return !a[0]; };
static auto and = [](std::vector<int>& a) {return std::all_of(a.begin(), a.end(), [&a](int n) {return n && a[0]; }); };
static auto or = [](std::vector<int>& a) {return std::any_of(a.begin(), a.end(), [&a](int n) {return n || a[0]; }); };

//auto xor =[](std::vector<int>& a){return std::for_each(a.begin(), a.end(),[](int b))}
}

static std::unordered_map<std::string, int> str_expr_map = {
	{ "", INT_MIN },
	{ "(", INT_MIN + 1 },
	{ ")", INT_MIN + 2 },
	{ ",", INT_MIN + 3 },
	{ "sub", INT_MIN + 4 },
	{ "mul", INT_MIN + 5 },
	{ "div", INT_MIN + 6 },
	{ "mod", INT_MIN + 7 },
	{ "sqr", INT_MIN + 8 },
	{ "pow", INT_MIN + 9 },
	{ "min", INT_MIN + 10 },
	{ "max", INT_MIN + 11 },
	{ "dist", INT_MIN + 12 },
	{ "le", INT_MIN + 13 },
	{ "lt", INT_MIN + 14 },
	{ "ge", INT_MIN + 15 },
	{ "gt", INT_MIN + 16 },
	{ "ne", INT_MIN + 17 },
	{ "eq", INT_MIN + 18 },
	{ "not", INT_MIN + 19 },
	{ "and", INT_MIN + 20 },
	{ "or", INT_MIN + 21 },
	{ "abs", INT_MIN + 22 },
	{ "add", INT_MIN + 23 },
	{ "neg", INT_MIN + 24 },
};

static func_map int_expr_map = {
	{ INT_MIN, ops::nullexp },
	{ INT_MIN + 1, ops::nullexp },
	{ INT_MIN + 2, ops::nullexp },
	{ INT_MIN + 3, ops::nullexp },
	{ INT_MIN + 4, ops::sub },
	{ INT_MIN + 5, ops::mul },
	{ INT_MIN + 6, ops::div },
	{ INT_MIN + 7, ops::mod },
	{ INT_MIN + 8, ops::sqr },
	{ INT_MIN + 9, ops::pow },
	{ INT_MIN + 10, ops::min },
	{ INT_MIN + 11, ops::max },
	{ INT_MIN + 12, ops::dist },
	{ INT_MIN + 13, ops::le },
	{ INT_MIN + 14, ops::lt },
	{ INT_MIN + 15, ops::ge },
	{ INT_MIN + 16, ops::gt },
	{ INT_MIN + 17, ops::ne },
	{ INT_MIN + 18, ops::eq },
	{ INT_MIN + 19, ops::not},
	{ INT_MIN + 20, ops::and },
	{ INT_MIN + 21, ops:: or },
	{ INT_MIN + 22, ops::abs },
	{ INT_MIN + 23, ops::add },
	{ INT_MIN + 24, ops::neg },
};

}

using namespace std;

class HVar {
public:
	int id;
	int uid;
	string name;
	vector<int> vals;
	unordered_map<int, int> val_map;
	vector<int> anti_map;
	const int std_min = 0;
	const int std_max;
	HVar(const int id, const int uid, const string name, const int min_val, const int max_val);
	HVar(const int id, const int uid, const string name, vector<int>& v);
	~HVar();
	void Show();
};

class HTab {
public:
	int id;
	vector<HVar*> scope;
	bool isSTD = false;
	bool semantics;
	vector<vector<int>> tuples;
	HTab(const int id, const bool sem, vector<vector<int>>& ts, vector<HVar*>& scp, const bool STD);
	HTab(HTab* t, vector<HVar*>& scp);
	int GetAllSize() const;
	void GetSTDTuple(vector<int>& src_tuple, vector<int>& std_tuple);
	void GetORITuple(vector<int>& std_tuple, vector<int>& ori_tuple);
	bool SAT(vector<int>& t);
	bool SAT_STD(vector<int>& t);
	void Show();
	void GetTuple(int idx, vector<int>& t, vector<int>& t_idx);
	bool sat(vector<int>& t);
private:
	//临时变量
	vector<int> tmp_t_;
	friend ostream& operator<<(ostream &os, const vector<HVar*>& a);
};

class HModel {
public:
	vector<HVar*> vars;
	vector<HTab*> tabs;
	unordered_map<HVar*, vector<HTab*>> subscriptions;
	//两个矩阵组成的矩阵,矩阵内容为，作用在两个变量之间约束的个数
	//.empty()表示无约束作用在该两个变量之间
	vector<vector<vector<int>>> neighborhoods;

	HModel();
	virtual ~HModel();
	int AddVar(const string name, const int min_val, const int max_val);
	int AddVar(const string name, vector<int>& v);
	int AddTab(const bool sem, vector<vector<int>>& ts, vector<int>& scp);
	int AddTab(const bool sem, vector<vector<int>>& ts, vector<string>& scp);
	int AddTab(const bool sem, vector<vector<int>>& ts, vector<HVar*>& scp, const bool STD = false);
	int AddTab(const string expr);
	int AddTabAsPrevious(HTab* t, vector<string>& scp);
	int max_domain_size() const { return mds_; }
	int max_arity() const { return mas_; };
	void show();
	int regist(string exp_name, function<int(vector<int>&)>);
	static int calculate(vector<int> &stack, vector<int>& params_len);
	vector<HTab*> solution_check(vector<int>& sol);
	bool have_same_scope() const { return have_same_scope_; }
private:
	void get_postfix(const string expr, vector<int>& data, vector<int>& params, vector<int>& num_op_params, vector<HVar*>& scp);
	tuple<ExpType, int> get_type(string expr);
	static ExpType get_type(const int expr);
	void subscript(HTab *t);
	void neighbor(HTab* t);
	void get_scope(vector<string>& scp_str, vector<HVar*>& scp);
	int get_var_id(const int id) const;
	int generate_exp_uid();
	int generate_var_uid();
	static void GetSTDTuple(vector<int>& src_tuple, vector<int>& std_tuple, vector<HVar*>& scp);
	static void GetORITuple(vector<int>& std_tuple, vector<int>& ori_tuple, vector<HVar*>& scp);
	static void get_ori_tuple_by_index(int idx, std::vector<int>& t, const vector<HVar*> scp);
	static void result(int op, vector<int>& result, const int len);
	unordered_map<string, HVar*> str_var_map_;
	unordered_map<int, HVar*> int_var_map_;
	size_t mds_ = 0;
	size_t mas_ = 0;
	int exp_id_ = MIN_USER_OPT;
	int var_uid_ = MAX_VALUE;
	bool have_same_scope_ = false;
};

} /* namespace cp */

