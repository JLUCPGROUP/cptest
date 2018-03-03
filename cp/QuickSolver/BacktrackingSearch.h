#pragma once
#include "QModel.h"
namespace cp {
class BacktrackingSearch {
public:
	BacktrackingSearch(HModel *h);
	~BacktrackingSearch();
	inline int new_level();
	inline int back_level();
	inline void copy_level(const int src, const int dest);
	//void clear_level(const int p);
	void insert(QVar* v);
	SearchStatistics statistics() const;
	virtual PropagationState propagate(vector<QVar*>& x_evt, const int level) = 0;

	virtual SearchStatistics solve(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limit);
	QVal select_QVal(const Heuristic::Var varh, const Heuristic::Val valh, const int level);
	QVar* select_QVar(const Heuristic::Var varh, const int level);
	int select_value(QVar* v, const Heuristic::Val valh, const int level);
	vector<QVar*> vars;
	vector<QTab*> tabs;
	vector<u64> con_stamp;
	vector<u64> var_stamp;
	vector<double> con_weight;
	u64 time = 0;
	unordered_map<QVar*, vector<QTab*>> subscription;
	unordered_map<QVar*, vector<QVar* >> neighborhood;
	vector<vector<QVar*>> neibor_vars;
	//  由于所有变量的域长度不一定相同 所以这里的c-value值不一定真实存在
	inline int get_QConVal_index(QConVal& c_val) const {
		return  c_val.c->id * max_arity * max_dom_size + c_val.c->index(c_val.v) * max_dom_size + c_val.a;
	}
	inline int get_QConVal_index(QTab* c, QVar* v, const int a) const {
		return  c->id * max_arity * max_dom_size + c->index(v) * max_dom_size + a;
	}
	inline int get_QConVal_index(QTab* c, const int v_idx, const int a) const {
		return  c->id * max_arity * max_dom_size + v_idx * max_dom_size + a;
	}
protected:
	vector<int> tmp_tuple_;
	vars_cir_que q_;
	vector<QVar*> get_scope(HTab* t);
	vector<QVar*> get_neighbor(QVar* v);
	const int max_arity;
	const int max_dom_size;
	const int max_bitDom_size;
	const int num_vars;
	const int num_tabs;
	PropagationState ps_;
	SearchStatistics ss_;
	assignments_stack I_;
	int top_ = 0;
	int tmp_;
};

class MAC3 :public BacktrackingSearch {
public:
	MAC3(HModel *h);
	~MAC3() {};
	PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
	bool revise(QTab* c, QVar* v, const int level);
	bool seek_support(QTab* c, QVar* v, int a, const int level);
};

class MAC3bit :public BacktrackingSearch {
public:
	MAC3bit(HModel *h);
	~MAC3bit() {};
	PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
	bool revise(QTab* c, QVar* v, const int level);
	bool seek_support(QTab* c, QVar* v, const int a, const int level);
protected:
	vector<vector<u64>> bitSup_;
};

class MAC3rm :public BacktrackingSearch {
public:
	MAC3rm(HModel *h);
	~MAC3rm() {};
	PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
	bool revise(QTab* c, QVar* v, const int level);
	bool seek_support(QTab* c, QVar* v, const int a, const int level);
protected:
	vector<vector<int>> res_;
};

class lMaxRPC_bit_rm :public BacktrackingSearch {
public:
	lMaxRPC_bit_rm(HModel *h);
	~lMaxRPC_bit_rm() {};
	PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
	bool search_pc_sup(QVar* const var, int i, QVar* var1, const int p);
	bool is_consistent(QVar* const var, int i, QVar* var1, int b, const int p);
	bool search_pc_wit(QVar* const i, const int a, QVar* j, int b, const int p);
	bool have_no_PC_support(QVar* x, const int a, QVar* y, const int p);
	bool have_PC_wit(QVar* x, const int a, QVar* y, const int b, QVar* z, const int p);
protected:
	vector<vector<QTab*>> neibor_matrix;
	vector<vector<vector<QVar*>>> pc_nei_;
	vector<int> last_pc;
	vector<int> last_ac;
	vector<vector<u64>> bitSup_;
};
}
