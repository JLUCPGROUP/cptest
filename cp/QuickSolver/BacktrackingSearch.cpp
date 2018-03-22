#include <unordered_set>
#include "Timer.h"
#include "BacktrackingSearch.h"
#include <set>
#include <sstream>
#include <map>

namespace  cp {
	typedef pair<int, int> PAIR;
	struct CmpByValue {
		bool operator()(const PAIR& lhs, const PAIR& rhs) const {
			return lhs.second > rhs.second;
		}
	};

	int BacktrackingSearch::new_level() {
		++top_;
		for (auto v : vars)
			v->new_level(top_ - 1, top_);
		return top_;
	}

	int BacktrackingSearch::back_level() {
		--top_;
		//for (auto v : vars)
		//	v->back_to(top_ + 1, top_);
		return top_;
	}

	void BacktrackingSearch::copy_level(const int src, const int dest) {
		if (src == dest)
			return;
		for (auto v : vars)
			v->copy_level(src, dest);
	}

	vector<QVar*> BacktrackingSearch::get_neighbor(const QVar& v) {
		unordered_set<QVar*> vs;
		for (auto c : subscription[v.id])
			for (auto x : c->scope)
				if (x->id != v.id)
					vs.insert(x);

		return vector<QVar*>(vs.begin(), vs.end());
	}

	inline void BacktrackingSearch::next_val(QVal& val, const int p) {
		val.v->next_value(val.a, p);
	}

	void BacktrackingSearch::get_solution() {
		for (int i = 0; i < num_vars; ++i) {
			solution_[i] = vars[i]->vals[I.v_[i]];
			sol_std[i] = I.v_[i];
		}
		solutions.push_back(solution_);
		get_solution_str();
	}

	BacktrackingSearch::BacktrackingSearch(const HModel& h, const bool backtrackable) :
		max_arity(h.max_arity()),
		max_dom_size(h.max_domain_size()),
		max_bitDom_size(ceil(float(h.max_domain_size()) / BITSIZE)),
		num_vars(h.vars.size()),
		num_tabs(h.tabs.size()),
		num_cva(num_tabs*max_dom_size*max_arity),
		tmp_(h.vars.size() + 1) {

		vars.reserve(num_vars);
		tabs.reserve(num_tabs);
		subscription.resize(num_tabs);
		neighborhood.resize(num_vars);
		deg.resize(num_vars, 0);
		var_deg_que.resize(num_vars);

		for (auto hv : h.vars) {
			auto*v = new QVar(hv);
			vars.push_back(v);
		}
		if (backtrackable)
			enable_backtracking();

		for (auto ht : h.tabs) {
			QTab *t = new QTab(ht, get_scope(*ht));
			tabs.push_back(t);
		}

		//变量参与的约束
		for (const auto t : tabs)
			for (auto v : t->scope)
				subscription[v->id].push_back(t);
		list<QVar*> q;
		map<int, int> var_map;
		for (int i = 0; i < num_vars; i++) {
			deg[i] = subscription[i].size();
			var_map[i] = deg[i];
			//cout << deg[i] << endl;
		}

		vector<PAIR> score(var_map.begin(), var_map.end());
		sort(score.begin(), score.end(), CmpByValue());

		for (int i = 0; i < num_vars; ++i) {
			var_deg_que[i] = score[i].first;
			//cout << var_deg_que[i] << endl;
		}

		//初始化约束查找矩阵
		neibor_matrix.resize(num_vars, vector<vector<QTab*>>(num_vars, vector<QTab*>()));
		for (const auto c : tabs) {
			neibor_matrix[c->scope[0]->id][c->scope[1]->id].push_back(c);
			neibor_matrix[c->scope[1]->id][c->scope[0]->id].push_back(c);
		}
		//for (auto v : vars) {
		//	neighborhood[v->id] = get_neighbor(*v);
		//	//neibor_vars[v->id] = neighborhood[v->id];
		//}
		for (const auto x : vars)
			for (const auto y : vars)
				if ((x != y) && !neibor_matrix[x->id][y->id].empty())
					neighborhood[x->id].push_back(y);

		con_stamp.resize(num_tabs, 0);
		var_stamp.resize(num_tabs, 0);
		con_weight.resize(num_tabs, 1);
		q_.initial(num_vars);
		tmp_tuple_.resize(max_arity);
		Exclude(tmp_tuple_);

		I.initial(h);
		solutions.reserve(1);
		solution_.resize(num_vars);
		sol_std.resize(num_vars);
	}

	BacktrackingSearch::~BacktrackingSearch() {
		for (auto v : vars)
			delete v;
		for (auto t : tabs)
			delete t;
	}

	u64 BacktrackingSearch::get_QConVal_index(const QConVal& c_val) const {
		return  c_val.c->id * max_arity * max_dom_size + c_val.get_var_index() * max_dom_size + c_val.a;
	}

	u64 BacktrackingSearch::get_QConVal_index(const QTab& c, const QVar& v, const int a) const {
		return  c.id * max_arity * max_dom_size + c.index(v) * max_dom_size + a;
	}

	u64 BacktrackingSearch::get_QConVal_index(const QTab& c, const int v_idx, const int a) const {
		return  c.id * max_arity * max_dom_size + v_idx * max_dom_size + a;
	}

	void BacktrackingSearch::enable_backtracking() {
		tmp_ = num_vars + 1;
		if (!backtrackable_) {
			backtrackable_ = true;
			for (auto v : vars)
				v->enable_backtracking(num_vars);
		}
	}

	void BacktrackingSearch::disable_backtracking() {
		if (backtrackable_) {
			backtrackable_ = false;
			for (auto v : vars)
				v->disable_backtracking();
		}
	}

	void BacktrackingSearch::show(const int p) {
		for (auto x : vars)
			x->show(p);
	}

	string BacktrackingSearch::get_solution_str() {
		if (solutions.empty()) {
			sol_str = "";
			return "";
		}
		else {
			stringstream strs;
			for (int a : solutions[0]) {
				strs << a << " ";
			}
			sol_str = strs.str();
			sol_str.pop_back();
		}
		return sol_str;
	}

	bool BacktrackingSearch::solution_check() {
		if (solutions.empty())
			return false;

		vector<int> tuple(max_arity);
		tuple.clear();
		for (auto c : tabs) {
			for (auto v : c->scope)
				tuple.push_back(sol_std[v->id]);
			if (!c->sat(tuple))
				return false;
			tuple.clear();
		}
		return true;
	}

	vector<QVar*> BacktrackingSearch::get_scope(const HTab& t) {
		vector<QVar*> tt(t.scope.size());
		for (int i = 0; i < t.scope.size(); ++i)
			tt[i] = vars[t.scope[i]->id];
		return tt;
	}

	void BacktrackingSearch::insert(QVar& v, const int p) {
		q_.push(v, p);
		var_stamp[v.id] = ++time;
	}

	SearchStatistics BacktrackingSearch::statistics() const {
		return ss_;
	}

	SearchStatistics BacktrackingSearch::binary_search(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits) {
		Timer t;
		vector<QVar*> x_evt;
		bool consistent_ = propagate(vars, 0).state;
		x_evt.clear();
		bool finished_ = false;
		if (!consistent_) {
			ss_.solve_time = t.elapsed();
			return ss_;
		}

		while (!finished_) {
			if (t.elapsed() > time_limits) {
				ss_.solve_time = t.elapsed();
				ss_.time_out = true;
				return ss_;
			}

			QVal v_a = select_QVal(varh, valh, top_);
			//cout << "---------------------------" << endl;
			//cout << "push: " << v_a << endl;
			//cout << "---------------------------" << endl;
			top_ = new_level();
			I.push(v_a);
			++ss_.num_positives;

			//选出的变量论域大小为1
			if (v_a.v->size(top_) != 1 && consistent_) {
				//++ss_.num_positives;
				v_a.v->reduce_to(v_a.a, top_);
				x_evt.push_back(v_a.v);
				consistent_ = propagate(x_evt, top_).state;
				x_evt.clear();
			}

			if (consistent_&&I.full()) {
				//cout << I << endl;
				finished_ = true;
				ss_.solve_time = t.elapsed();
				get_solution();
				return ss_;
				//++sol_count_;
				//consistent_ = false;
			}

			while (!consistent_ && !I.empty()) {
				v_a = I.pop();
				//cout << "---------------------------" << endl;
				//cout << "pop:  " << v_a << endl;
				//cout << "---------------------------" << endl;
				top_ = back_level();
				//选出的变量论域大小不为1
				if (v_a.v->size(top_) != 1) {
					v_a.v->remove_value(v_a.a, top_);
					++ss_.num_negatives;
					x_evt.push_back(v_a.v);
					consistent_ = v_a.v->size(top_) && propagate(x_evt, top_).state;
					x_evt.clear();
				}
			}

			if (!consistent_)
				finished_ = true;
		}

		ss_.solve_time = t.elapsed();
		return ss_;
	}

	SearchStatistics BacktrackingSearch::nonbinary_search(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits) {
		Timer t;
		vector<QVar*> x_evt;
		bool consistent_ = propagate(vars, 0).state;
		x_evt.clear();
		bool finished_ = false;
		if (!consistent_) {
			ss_.solve_time = t.elapsed();
			return ss_;
		}
		QVal val = select_QVal(varh, valh, 0);
		//		const QVal NullNode = QVal(nullptr, -2);
		while ((!I.empty()) || !val.is_null_node()) {
			//search time out
			if (t.elapsed() > time_limits) {
				ss_.solve_time = t.elapsed();
				ss_.time_out = true;
				return ss_;
			}

			// val is exist
			if (!val.is_null_node()) {
				//cout << val << endl;
				top_ = new_level();
				I.push(val);
				++ss_.num_positives;
				val.v->reduce_to(val.a, top_);
				x_evt.push_back(val.v);
				consistent_ = propagate(x_evt, top_).state;
				x_evt.clear();

			}

			if (consistent_) {
				if (I.full()) {
					//meet solution
					ss_.solve_time = t.elapsed();
					get_solution();
					return ss_;
				}
				else {
					// select v-value
					val = select_QVal(varh, valh, top_);
				}
			}

			if (!consistent_) {
				//backtracking
				back_level();
				val = I.pop();
				next_val(val, top_);
				//cout << "next: ";
			}

		}

		ss_.solve_time = t.elapsed();
		return ss_;
	}

	QVal BacktrackingSearch::select_QVal(const Heuristic::Var varh, const Heuristic::Val valh, const int p) {
		QVar* v = select_QVar(varh, p);
		const int a = select_value(*v, valh, p);
		QVal val(v, a);
		return val;
	}

	QVar* BacktrackingSearch::select_QVar(const Heuristic::Var varh, const int p) {
		QVar* var = nullptr;
		double min_size = DBL_MAX;
		switch (varh) {
		case Heuristic::VRH_DOM_MIN: {
			for (auto v : vars)
				if (!I.assigned(*v))
					if (v->size(p) < min_size) {
						min_size = v->size(p);
						var = v;
					}
		} return var;
		case Heuristic::VRH_LEX:
			var = vars[I.size()];
			break;
		case Heuristic::VRH_DEG_MIN:
			var = vars[var_deg_que[I.size()]];
			break;
		case Heuristic::VRH_VWDEG: break;
		case Heuristic::VRH_DOM_DEG_MIN: {
			for (auto v : vars)
				if (!I.assigned(*v)) {
					int dom_deg;
					if (neighborhood[v->id].empty())
						dom_deg = -1;
					else
						dom_deg = v->size(p) / neighborhood[v->id].size();
					if (dom_deg < min_size) {
						min_size = dom_deg;
						var = v;
					}
				}
		}return var;
		case Heuristic::VRH_DOM_DDEG_MIN: {
			for (auto x : vars) {
				if (!I.assigned(*x)) {
					const auto cur_size = double(x->size(p));
					if (cur_size == 1)
						return x;

					double ddeg = 0;
					double d_ddeg;
					for (auto c : subscription[x->id]) {
						for (auto y : c->scope) {
							if (y != x) {
								if (!I.assigned(*y)) {
									++ddeg;
									break;
								}
							}
						}
					}

					if (ddeg == 0)
						return x;
					else
						d_ddeg = cur_size / ddeg;

					if (d_ddeg < min_size) {
						min_size = d_ddeg;
						var = x;
					}
				}
			}
		}return var;
		case Heuristic::VRH_DOM_WDEG_MIN: {
			for (auto x : vars) {
				if (!I.assigned(*x)) {
					const auto cur_size = double(x->size(p));
					if (cur_size == 1)
						return x;

					double x_w = 0.0;
					double x_dw;

					for (auto c : subscription[x->id]) {
						int asd_cnt = 0;
						for (auto y : c->scope)
							if (I.assigned(*y))
								++asd_cnt;
						if ((asd_cnt + 1) < c->arity)
							x_w += con_weight[c->id];
					}

					if (!x_w)
						return x;
					else
						x_dw = cur_size / x_w;

					if (x_dw < min_size) {
						min_size = x_dw;
						var = x;
					}
				}
			}
		} return var;
		default:
			var = nullptr;
			break;
		}
		return var;
	}

	int BacktrackingSearch::select_value(const QVar& v, const Heuristic::Val valh, const int p) {
		int val = -1;
		switch (valh) {
		case Heuristic::VLH_MIN:
			val = v.head(p);
			break;
		case Heuristic::VLH_MIN_DOM: break;
		case Heuristic::VLH_MIN_INC: break;
		case Heuristic::VLH_MAX_INC: break;
		case Heuristic::VLH_VWDEG: val = -1; break;
		default:;
		}
		return val;
	}


	///////////////////////////////////////////////////////////////
	//MAC3::MAC3(HModel* h) :BacktrackingSearch(h) {}
	//PropagationState MAC3::propagate(vector<QVar*>& x_evt, const int level) {
	//	q_.clear();
	//	ps_.level = level;
	//	ps_.num_delete = 0;

	//	for (auto v : x_evt)
	//		insert(v, level);
	//	while (!q_.empty()) {
	//		QVar* x = q_.pop(level);
	//		//q_.pop_back();
	//		for (QTab* c : subscription[x]) {
	//			if (var_stamp[x->id] > con_stamp[c->id]) {
	//				for (auto y : c->scope) {
	//					if (!I.assigned(y)) {
	//						bool aa = false;
	//						for (auto z : c->scope)
	//							if ((z != x) && var_stamp[z->id] > con_stamp[c->id])
	//								aa = true;

	//						if ((y != x) || aa)
	//							if (revise(c, y, level)) {
	//								if (y->faild(level)) {
	//									ps_.tab = c;
	//									ps_.var = y;
	//									++(con_weight[c->id]);
	//									ps_.state = false;
	//									return ps_;
	//								}
	//								insert(y, level);
	//							}
	//					}
	//				}
	//				++time;
	//				con_stamp[c->id] = time;
	//			}
	//		}
	//	}

	//	ps_.state = true;
	//	return ps_;
	//}
	//bool MAC3::revise(QTab* c, QVar* v, const int level) {
	//	const int num_elements = v->size(level);
	//	int a = v->head(level);

	//	while (a != Limits::INDEX_OVERFLOW) {
	//		if (!seek_support(c, v, a, level)) {
	//			v->remove_value(a, level);
	//			//cout << "(" << v->id << ", " << a << ")" << endl;
	//			++ps_.num_delete;
	//		}
	//		a = v->next(a, level);
	//	}

	//	return num_elements != v->size(level);
	//}

	//bool MAC3::seek_support(QTab* c, QVar* v, const int a, const int level) {
	//	c->get_first_valid_tuple(v, a, tmp_tuple_, level);
	//	while (Existed(tmp_tuple_)) {
	//		//cout << "tuple: " << tmp_tuple_[0] << "," << tmp_tuple_[1] << endl;
	//		if (c->sat(tmp_tuple_))
	//			return true;
	//		else
	//			c->get_next_valid_tuple(v, a, tmp_tuple_, level);
	//	}
	//	return false;
	//}
	////////////////////////////////////////////////////////////////////////////////



	//////////////////////////////////////////////////////////////////////////////

	//MAC3rm::MAC3rm(HModel* h) :BacktrackingSearch(h) {
	//	Exclude(tmp_tuple_);
	//	res_.resize(tabs.size()*max_dom_size*max_arity, tmp_tuple_);
	//}

	//PropagationState MAC3rm::propagate(vector<QVar*>& x_evt, const int level) {
	//	q_.clear();
	//	ps_.level = level;
	//	ps_.num_delete = 0;

	//	for (auto v : x_evt)
	//		insert(v, level);
	//	while (!q_.empty()) {
	//		QVar* x = q_.pop(level);
	//		//q_.pop_back();
	//		for (QTab* c : subscription[x]) {
	//			if (var_stamp[x->id] > con_stamp[c->id]) {
	//				for (auto y : c->scope) {
	//					if (!I.assigned(y)) {
	//						bool aa = false;
	//						for (auto z : c->scope)
	//							if ((z != x) && var_stamp[z->id] > con_stamp[c->id])
	//								aa = true;

	//						if ((y != x) || aa)
	//							if (revise(c, y, level)) {
	//								if (y->faild(level)) {
	//									ps_.tab = c;
	//									ps_.var = y;
	//									++(con_weight[c->id]);
	//									ps_.state = false;
	//									return ps_;
	//								}
	//								insert(y, level);
	//							}
	//					}
	//				}
	//				++time;
	//				con_stamp[c->id] = time;
	//			}
	//		}
	//	}

	//	ps_.state = true;
	//	return ps_;
	//}

	//bool MAC3rm::revise(QTab* c, QVar* v, const int level) {
	//	const int num_elements = v->size(level);
	//	int a = v->head(level);
	//	while (a != Limits::INDEX_OVERFLOW) {
	//		if (!seek_support(c, v, a, level)) {
	//			v->remove_value(a, level);
	//			//cout << "(" << v->id << ", " << a << ")" << endl;
	//			++ps_.num_delete;
	//		}
	//		a = v->next(a, level);
	//	}
	//	return num_elements != v->size(level);
	//}

	//bool MAC3rm::seek_support(QTab* c, QVar* v, const int a, const int p) {
	//	tmp_tuple_ = res_[get_QConVal_index(c, v, a)];
	//	if (c->is_valid(tmp_tuple_, p))
	//		return true;

	//	c->get_first_valid_tuple(v, a, tmp_tuple_, p);
	//	while (Existed(tmp_tuple_)) {
	//		if (c->sat(tmp_tuple_)) {
	//			for (int i = 0; i < c->arity; ++i)
	//				res_[get_QConVal_index(c, c->scope[i], tmp_tuple_[i])] = tmp_tuple_;
	//			return true;
	//		}
	//		c->get_next_valid_tuple(v, a, tmp_tuple_, p);
	//	}

	//	return false;
	//}
	//////////////////////////////////////////////////////////////////

	//lMaxRPC_bit_rm::lMaxRPC_bit_rm(HModel* h) :BacktrackingSearch(h) {
	//	//初始化约束查找矩阵
	//	neibor_matrix.resize(vars.size(), vector<QTab*>(vars.size(), nullptr));
	//	for (auto c : tabs) {
	//		neibor_matrix[c->scope[0]->id][c->scope[1]->id] = c;
	//		neibor_matrix[c->scope[1]->id][c->scope[0]->id] = c;
	//	}

	//	common_neibor_.resize(vars.size(), vector<vector<QVar*>>(vars.size()));
	//	set<QVar*> vars_map;
	//	vector<bool> vars_in(vars.size(), false);
	//	for (auto x : vars) {
	//		for (auto y : vars) {
	//			for (auto z : vars) {
	//				if (neibor_matrix[x->id][z->id] != nullptr&&neibor_matrix[y->id][z->id]) {
	//					if (!vars_in[z->id]) {
	//						vars_in[z->id] = true;
	//						common_neibor_[x->id][y->id].push_back(z);
	//					}
	//				}
	//			}
	//			vars_in.assign(vars_in.size(), false);
	//		}
	//	}

	//	last_pc.resize(num_tabs*max_dom_size*max_arity*max_arity, Limits::INDEX_OVERFLOW);
	//	last_ac.resize(num_tabs*max_dom_size*max_arity*max_arity, Limits::INDEX_OVERFLOW);

	//	//initial bitSup
	//	bitSup_.resize(num_tabs*max_dom_size*max_arity, vector<u64>(max_bitDom_size, 0));
	//	for (QTab* c : tabs) {
	//		for (auto t : c->tuples) {
	//			const int index[] = { get_QConVal_index(c, c->scope[0], t[0]), get_QConVal_index(c, c->scope[1], t[1]) };
	//			const BitIndex idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
	//			bitSup_[index[0]][idx[1].x] |= U64_MASK1[idx[1].y];
	//			bitSup_[index[1]][idx[0].x] |= U64_MASK1[idx[0].y];
	//		}
	//	}
	//}

	//PropagationState lMaxRPC_bit_rm::propagate(vector<QVar*>& x_evt, const int p) {
	//	//q_.clear();
	//	//for (auto i : x_evt) {
	//	//	for (int a = i->head(p); a != Limits::INDEX_OVERFLOW; i->next_value(a, p)) {
	//	//		for (auto j : neibor_vars[i->id]) {
	//	//			if (have_no_PC_support(i, a, j, p)) {
	//	//				i->remove_value(a, p);
	//	//				if (!i->faild(p)) {
	//	//					q_.push(i);
	//	//				}
	//	//				else {
	//	//					++con_weight[neibor_matrix[i->id][j->id]->id];
	//	//					ps_.state = false;
	//	//					return ps_;
	//	//				}
	//	//			}
	//	//		}
	//	//	}
	//	//}
	//	//q_.clear();
	//	//bool changed = false;
	//	//for (auto v : x_evt)
	//	//	q_.push(v);

	//	//while (!q_.empty()) {
	//	//	const auto j = q_.pop();
	//	//	for (auto i : neibor_vars[j->id]) {
	//	//		for (int a = i->head(p); a != Limits::INDEX_OVERFLOW; i->next_value(a, p)) {
	//	//			const auto c = neibor_matrix[i->id][j->id];
	//	//			const auto idx = get_QConVal_index(c, i, a);
	//	//			if (j->have(last_pc[idx], p) && have_no_PC_support(i, a, j, p)) {
	//	//				i->remove_value(a, p);
	//	//				if (!i->faild(p)) {
	//	//					q_.push(i);
	//	//				}
	//	//				else {
	//	//					++con_weight[c->id];
	//	//					ps_.state = false;
	//	//					return ps_;
	//	//				}
	//	//			}
	//	//		}
	//	//	}
	//	//}

	//	//ps_.state = true;
	//	//return ps_;
	//	q_.clear();
	//	bool changed = false;
	//	for (auto v : x_evt)
	//		q_.push(v);

	//	while (!q_.empty()) {
	//		const auto j = q_.pop();
	//		for (auto i : neibor_vars[j->id]) {
	//			if (I.assigned(i))
	//				continue;

	//			const auto c = neibor_matrix[i->id][j->id];
	//			for (int a = j->head(p); a != Limits::INDEX_OVERFLOW; j->next_value(a, p)) {
	//				if (!search_pc_sup(i, a, j, p)) {
	//					i->remove_value(a, p);
	//					changed = true;
	//				}
	//			}

	//			if (changed) {
	//				if (i->faild(p)) {
	//					++con_weight[c->id];
	//					ps_.state = false;
	//					return ps_;
	//				}
	//				q_.push(i);
	//			}
	//		}
	//	}

	//	ps_.state = true;
	//	return ps_;
	//}

//	bool lMaxRPC_bit_rm::search_pc_sup(QVar* const i, const int a, QVar* j, const int p) {
//		const auto c = neibor_matrix[i->id][j->id];
//		const int cval_cia_idx = get_QConVal_index(c, i, a);
//		const int lastPCiaj = last_pc[cval_cia_idx];
//		if (lastPCiaj != Limits::INDEX_OVERFLOW && j->have(lastPCiaj, p))
//			return true;
//		for (int b = j->head(p); b != Limits::INDEX_OVERFLOW; j->next_value(b, p)) {
//			if (is_consistent(i, a, j, b, p)) {
//				if (search_pc_wit(i, a, j, b, p)) {
//					const int cval_cjb_idx = get_QConVal_index(c, j, b);
//					last_pc[cval_cia_idx] = b;
//					last_pc[cval_cjb_idx] = a;
//					return true;
//				}
//			}
//		}
//		return false;
//	}
//	bool lMaxRPC_bit_rm::is_consistent(QVar* const i, const int a, QVar* j, const int b, const int p) {
//		const auto c = neibor_matrix[i->id][j->id];
//		const int idx[] = { get_QConVal_index(c, i, a) , get_QConVal_index(c, j, b) };
//		const BitIndex index[] = { GetBitIdx(a),GetBitIdx(b) };
//		if (!(bitSup_[idx[0]][index[1].x] & j->bitDom(p)[index[1].x]))
//			return false;
//		if (!(bitSup_[idx[1]][index[0].x] & j->bitDom(p)[index[0].x]))
//			return false;
//		return true;
//	}
//
//	bool lMaxRPC_bit_rm::search_pc_wit(QVar* const i, const int a, QVar* j, const int b, const int p) {
//		for (auto k : common_neibor_[i->id][j->id]) {
//			bool maxRPCsupport = false;
//			const auto c_ik = neibor_matrix[i->id][k->id];
//			const auto c_jk = neibor_matrix[j->id][k->id];
//			const auto cva_ikia = get_QConVal_index(c_ik, i, a);
//			const auto cva_jkjb = get_QConVal_index(c_jk, j, b);
//
//			const int bitdom_size = k->num_bit;
//			for (int c = 0; c < bitdom_size; ++c)
//				if (bitSup_[cva_ikia][c] & bitSup_[cva_jkjb][c] & k->bitDom(p)[c]) {
//					maxRPCsupport = true;
//					break;
//				}
//
//			if (!maxRPCsupport)
//				return false;
//		}
//		return true;
//	}
//
//	bool lMaxRPC_bit_rm::have_no_PC_support(QVar* i, const int a, QVar* j, const int p) {
//		const auto c = neibor_matrix[i->id][j->id];
//		const auto idx1 = get_QConVal_index(c, i, a);
//
//		for (int b = j->head(p); b != Limits::INDEX_OVERFLOW; j->next_value(b, p)) {
//			int PCWitness = true;
//			const auto b_idx = GetBitIdx(b);
//			if (bitSup_[idx1][b_idx.x] | U64_MASK1[b_idx.y]) {
//				for (auto k : common_neibor_[i->id][j->id]) {
//					if (!have_PC_wit(i, a, j, b, k, p)) {
//						PCWitness = false;
//						break;
//					}
//				}
//				if (PCWitness != false) {
//					const auto idx2 = get_QConVal_index(c, j, b);
//					last_pc[idx1] = b;
//					last_pc[idx2] = a;
//					last_ac[idx1] = b / BITSIZE;
//					return false;
//				}
//			}
//		}
//
//		return true;
//	}
//	bool lMaxRPC_bit_rm::have_PC_wit(QVar* i, const int a, QVar* j, const int b, QVar* k, const int p) {
//		const auto c_ik = neibor_matrix[i->id][k->id];
//		const auto c_jk = neibor_matrix[j->id][k->id];
//		const auto cva_ikia = get_QConVal_index(c_ik, i, a);
//		const auto cva_jkjb = get_QConVal_index(c_jk, j, b);
//		//const auto cva_ikia = m_->GetIntConValIndex(c_ik->id(), i->id(), a);
//		if (last_ac[cva_ikia] != Limits::INDEX_OVERFLOW) {
//			const auto d = last_ac[cva_ikia];
//			if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k->bitDom(p)[d]) {
//				return true;
//			}
//			//const auto d = last_ac[cva_ikia];
//			//const auto idx = GetBitIdx(d);
//			//if (bitSup_[cva_ikia][get<0>(idx)].test(get<1>(idx))&
//			//	bitSup_[cva_jkjb][get<0>(idx)].test(get<1>(idx))&
//			//	k->bitDom(level_)[get<0>(idx)].test(get<1>(idx))) {
//			//	return true;
//			//}
//		}
//
//		if (last_ac[cva_jkjb] != Limits::INDEX_OVERFLOW) {
//			const auto d = last_ac[cva_jkjb];
//			if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k->bitDom(p)[d]) {
//				return true;
//			}
//			//const auto idx = GetBitIdx(d);
//			//if (bitSup_[cva_ikia][get<0>(idx)].test(get<1>(idx))&
//			//	bitSup_[cva_jkjb][get<0>(idx)].test(get<1>(idx))&
//			//	k->bitDom(level_)[get<0>(idx)].test(get<1>(idx))) {
//			//	return true;
//			//}
//		}
//
//		//const auto c_ik = nei_[i->id()][k->id()];
//		//const auto c_jk = nei_[j->id()][k->id()];
//		//const int cva_ikia = m_->GetIntConValIndex(c_ik->id(), i->id(), a);
//		//const int cva_ikia = m_->GetIntConValIndex(c_jk->id(), j->id(), b);
//		const int bitdom_size = k->num_bit;
//		for (int c = 0; c < bitdom_size; ++c)
//			if (bitSup_[cva_ikia][c] & bitSup_[cva_jkjb][c] & k->bitDom(p)[c]) {
//				last_ac[cva_ikia] = c;
//				last_ac[cva_jkjb] = c;
//				return  true;
//			}
//		return false;
//	}
}
