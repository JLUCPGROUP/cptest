#include <unordered_set>
#include "Timer.h"
#include "BacktrackingSearch.h"
#include <set>

namespace  cp {
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

vector<QVar*> BacktrackingSearch::get_neighbor(QVar* v) {
	unordered_set<QVar*> vs;
	for (auto c : subscription[v])
		for (auto x : c->scope)
			if (x != v)
				vs.insert(x);

	return vector<QVar*>(vs.begin(), vs.end());
}

BacktrackingSearch::BacktrackingSearch(HModel* h) :
	max_arity(h->max_arity()),
	max_dom_size(h->max_domain_size()),
	max_bitDom_size(ceil(float(h->max_domain_size()) / BITSIZE)),
	num_vars(h->vars.size()),
	num_tabs(h->tabs.size()),
	top_(0),
	tmp_(h->vars.size() + 1) {

	vars.reserve(num_vars);
	tabs.reserve(num_tabs);
	neibor_vars.resize(num_vars);

	for (auto hv : h->vars) {
		QVar *v = new QVar(hv);
		v->runtime(num_vars);
		vars.push_back(v);
	}

	for (auto ht : h->tabs) {
		QTab *t = new QTab(ht, get_scope(ht));
		tabs.push_back(t);
	}

	for (auto t : tabs)
		for (auto v : t->scope)
			subscription[v].push_back(t);

	for (auto v : vars) {
		neighborhood[v] = get_neighbor(v);
		neibor_vars[v->id] = neighborhood[v];
	}

	con_stamp.resize(num_tabs, 0);
	var_stamp.resize(num_tabs, 0);
	con_weight.resize(num_tabs, 0);
	q_.initial(num_vars);
	tmp_tuple_.resize(max_arity);
	Exclude(tmp_tuple_);

	I_.initial(h);
}

BacktrackingSearch::~BacktrackingSearch() {
	for (auto v : vars)
		delete v;
	for (auto t : tabs)
		delete t;
}

vector<QVar*> BacktrackingSearch::get_scope(HTab* t) {
	vector<QVar*> tt(t->scope.size());
	for (int i = 0; i < t->scope.size(); ++i)
		tt[i] = vars[t->scope[i]->id];
	return tt;
}
void BacktrackingSearch::insert(QVar* v) {
	q_.push(v);
	++time;
	var_stamp[v->id] = time;
}

SearchStatistics BacktrackingSearch::statistics() const {
	return ss_;
}

SearchStatistics BacktrackingSearch::solve(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits) {
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
		//cout << "=" << v_a << endl;
		top_ = new_level();
		I_.push(v_a);
		++ss_.num_positive;
		v_a.v->reduce_to(v_a.a, top_);
		x_evt.push_back(v_a.v);
		consistent_ = propagate(x_evt, top_).state;
		x_evt.clear();
		//I.update_model_assigned();
		if (consistent_&&I_.full()) {
			cout << I_ << endl;
			finished_ = true;
			//++sol_count_;
			//consistent_ = false;
		}

		while (!consistent_ && !I_.empty()) {
			v_a = I_.pop();
			//cout << "!" << v_a << endl;
			top_ = back_level();
			v_a.v->remove_value(v_a.a, top_);
			++ss_.num_negative;
			x_evt.push_back(v_a.v);
			consistent_ = v_a.v->size(top_) && propagate(x_evt, top_).state;
			x_evt.clear();
		}

		if (!consistent_)
			finished_ = true;
	}

	ss_.solve_time = t.elapsed();
	return ss_;
}

QVal BacktrackingSearch::select_QVal(const Heuristic::Var varh, const Heuristic::Val valh, const int p) {
	QVar* v = select_QVar(varh, p);
	const int a = select_value(v, valh, p);
	QVal val(v, a);
	return val;
}

QVar* BacktrackingSearch::select_QVar(const Heuristic::Var varh, const int p) {
	QVar* var = nullptr;
	double min_size = DBL_MAX;
	switch (varh) {
	case Heuristic::VRH_DOM_MIN: {
		for (auto v : vars)
			if (!I_.assigned(v))
				if (v->size(p) < min_size) {
					min_size = v->size(p);
					var = v;
				}
	} return var;
	case Heuristic::VRH_LEX:
		var = vars[I_.size() + 1];
		break;
	case Heuristic::VRH_VWDEG: break;
	case Heuristic::VRH_DOM_DEG_MIN: {
		for (auto v : vars)
			if (!I_.assigned(v)) {
				int dom_deg;
				if (neighborhood[v].size() == 0)
					dom_deg = -1;
				else
					dom_deg = v->size(p) / neighborhood[v].size();
				if (dom_deg < min_size) {
					min_size = dom_deg;
					var = v;
				}
			}
	}return var;
	case Heuristic::VRH_DOM_WDEG_MIN: {
		for (auto x : vars) {
			if (!I_.assigned(x)) {
				double x_w = 0.0;
				double x_dw;

				for (auto c : subscription[x]) {
					int cnt = 0;
					for (auto y : c->scope)
						if (!I_.assigned(y))
							++cnt;
					if (cnt > 1)
						x_w += con_weight[c->id];
				}

				if (x->size(p) == 1 || x_w == 0)
					x_dw = -1;
				else
					x_dw = x->size(p) / x_w;

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

int BacktrackingSearch::select_value(QVar* v, const Heuristic::Val valh, const int p) {
	int val = -1;
	switch (valh) {
	case Heuristic::VLH_MIN:
		val = v->head(p);
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
MAC3::MAC3(HModel* h) :BacktrackingSearch(h) {}
PropagationState MAC3::propagate(vector<QVar*>& x_evt, const int level) {
	q_.clear();
	ps_.level = level;
	ps_.num_delete = 0;

	for (auto v : x_evt)
		insert(v);
	while (!q_.empty()) {
		QVar* x = q_.pop();
		//q_.pop_back();
		for (QTab* c : subscription[x]) {
			if (var_stamp[x->id] > con_stamp[c->id]) {
				for (auto y : c->scope) {
					if (!I_.assigned(y)) {
						bool aa = false;
						for (auto z : c->scope)
							if ((z != x) && var_stamp[z->id] > con_stamp[c->id])
								aa = true;

						if ((y != x) || aa)
							if (revise(c, y, level)) {
								if (y->faild(level)) {
									ps_.tab = c;
									ps_.var = y;
									++(con_weight[c->id]);
									ps_.state = false;
									return ps_;
								}
								insert(y);
							}
					}
				}
				++time;
				con_stamp[c->id] = time;
			}
		}
	}

	ps_.state = true;
	return ps_;
}
bool MAC3::revise(QTab* c, QVar* v, const int level) {
	const int num_elements = v->size(level);
	int a = v->head(level);

	while (a != Limits::INDEX_OVERFLOW) {
		if (!seek_support(c, v, a, level)) {
			v->remove_value(a, level);
			//cout << "(" << v->id << ", " << a << ")" << endl;
			++ps_.num_delete;
		}
		a = v->next(a, level);
	}

	return num_elements != v->size(level);
}

bool MAC3::seek_support(QTab* c, QVar* v, const int a, const int level) {
	c->get_first_valid_tuple(v, a, tmp_tuple_, level);
	while (Existed(tmp_tuple_)) {
		//cout << "tuple: " << tmp_tuple_[0] << "," << tmp_tuple_[1] << endl;
		if (c->sat(tmp_tuple_))
			return true;
		else
			c->get_next_valid_tuple(v, a, tmp_tuple_, level);
	}
	return false;
}
////////////////////////////////////////////////////////////////////////////////

MAC3bit::MAC3bit(HModel* h) :BacktrackingSearch(h) {
	bitSup_.resize(num_tabs*max_dom_size*max_arity, vector<u64>(max_bitDom_size, 0));
	for (QTab* c : tabs) {
		for (auto t : c->tuples) {
			const int index[] = { get_QConVal_index(c, 0, t[0]), get_QConVal_index(c, 1, t[1]) };
			const tuple<int, int> idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
			bitSup_[index[0]][get<0>(idx[1])] |= U64_MASK1[get<1>(idx[1])];
			bitSup_[index[1]][get<0>(idx[0])] |= U64_MASK1[get<1>(idx[0])];
		}
	}
}

PropagationState MAC3bit::propagate(vector<QVar*>& x_evt, const int level) {
	q_.clear();
	ps_.level = level;
	ps_.num_delete = 0;

	for (auto v : x_evt)
		insert(v);
	while (!q_.empty()) {
		QVar* x = q_.pop();
		//q_.pop_back();
		for (QTab* c : subscription[x]) {
			if (var_stamp[x->id] > con_stamp[c->id]) {
				for (auto y : c->scope) {
					if (!I_.assigned(y)) {
						bool aa = false;
						for (auto z : c->scope)
							if ((z != x) && var_stamp[z->id] > con_stamp[c->id])
								aa = true;

						if ((y != x) || aa)
							if (revise(c, y, level)) {
								if (y->faild(level)) {
									ps_.tab = c;
									ps_.var = y;
									++(con_weight[c->id]);
									ps_.state = false;
									return ps_;
								}
								insert(y);
							}
					}
				}
				++time;
				con_stamp[c->id] = time;
			}
		}
	}

	ps_.state = true;
	return ps_;
}

bool MAC3bit::revise(QTab* c, QVar* v, const int level) {
	const int num_elements = v->size(level);
	int a = v->head(level);
	while (a != Limits::INDEX_OVERFLOW) {
		if (!seek_support(c, v, a, level)) {
			v->remove_value(a, level);
			//cout << "(" << v->id << ", " << a << ")" << endl;
			++ps_.num_delete;
		}
		a = v->next(a, level);
	}
	return num_elements != v->size(level);
}

bool MAC3bit::seek_support(QTab* c, QVar* v, const int a, const int p) {
	const int idx = get_QConVal_index(c, v, a);
	for (QVar* y : c->scope)
		if (y->id != v->id)
			for (int i = 0; i < y->bitDom(p).size(); ++i)
				if (bitSup_[idx][i] & y->bitDom(p)[i])
					return true;
	return false;
}

//////////////////////////////////////////////////////////////////////////////

MAC3rm::MAC3rm(HModel* h) :BacktrackingSearch(h) {
	Exclude(tmp_tuple_);
	res_.resize(tabs.size()*max_dom_size*max_arity, tmp_tuple_);
}

PropagationState MAC3rm::propagate(vector<QVar*>& x_evt, const int level) {
	q_.clear();
	ps_.level = level;
	ps_.num_delete = 0;

	for (auto v : x_evt)
		insert(v);
	while (!q_.empty()) {
		QVar* x = q_.pop();
		//q_.pop_back();
		for (QTab* c : subscription[x]) {
			if (var_stamp[x->id] > con_stamp[c->id]) {
				for (auto y : c->scope) {
					if (!I_.assigned(y)) {
						bool aa = false;
						for (auto z : c->scope)
							if ((z != x) && var_stamp[z->id] > con_stamp[c->id])
								aa = true;

						if ((y != x) || aa)
							if (revise(c, y, level)) {
								if (y->faild(level)) {
									ps_.tab = c;
									ps_.var = y;
									++(con_weight[c->id]);
									ps_.state = false;
									return ps_;
								}
								insert(y);
							}
					}
				}
				++time;
				con_stamp[c->id] = time;
			}
		}
	}

	ps_.state = true;
	return ps_;
}

bool MAC3rm::revise(QTab* c, QVar* v, const int level) {
	const int num_elements = v->size(level);
	int a = v->head(level);
	while (a != Limits::INDEX_OVERFLOW) {
		if (!seek_support(c, v, a, level)) {
			v->remove_value(a, level);
			//cout << "(" << v->id << ", " << a << ")" << endl;
			++ps_.num_delete;
		}
		a = v->next(a, level);
	}
	return num_elements != v->size(level);
}

bool MAC3rm::seek_support(QTab* c, QVar* v, const int a, const int p) {
	tmp_tuple_ = res_[get_QConVal_index(c, v, a)];
	if (c->is_valid(tmp_tuple_, p))
		return true;

	c->get_first_valid_tuple(v, a, tmp_tuple_, p);
	while (Existed(tmp_tuple_)) {
		if (c->sat(tmp_tuple_)) {
			for (int i = 0; i < c->arity; ++i)
				res_[get_QConVal_index(c, i, tmp_tuple_[i])] = tmp_tuple_;
			return true;
		}
		c->get_next_valid_tuple(v, a, tmp_tuple_, p);
	}

	return false;
}
//////////////////////////////////////////////////////////////////

lMaxRPC_bit_rm::lMaxRPC_bit_rm(HModel* h) :BacktrackingSearch(h) {
	//初始化约束查找矩阵
	neibor_matrix.resize(vars.size(), vector<QTab*>(vars.size(), nullptr));
	for (auto c : tabs) {
		neibor_matrix[c->scope[0]->id][c->scope[1]->id] = c;
		neibor_matrix[c->scope[1]->id][c->scope[0]->id] = c;
	}

	pc_nei_.resize(vars.size(), vector<vector<QVar*>>(vars.size()));
	set<QVar*> vars_map;
	vector<bool> vars_in(vars.size(), false);
	for (auto x : vars) {
		for (auto y : vars) {
			for (auto z : vars) {
				if (neibor_matrix[x->id][z->id] != nullptr&&neibor_matrix[y->id][z->id]) {
					if (!vars_in[z->id]) {
						vars_in[z->id] = true;
						pc_nei_[x->id][y->id].push_back(z);
					}
				}
			}
			vars_in.assign(vars_in.size(), false);
		}
	}

	last_pc.resize(num_tabs*max_dom_size*max_arity*max_arity, Limits::INDEX_OVERFLOW);
	last_ac.resize(num_tabs*max_dom_size*max_arity*max_arity, Limits::INDEX_OVERFLOW);

	//initial bitSup
	bitSup_.resize(num_tabs*max_dom_size*max_arity, vector<u64>(max_bitDom_size, 0));
	for (QTab* c : tabs) {
		for (auto t : c->tuples) {
			const int index[] = { get_QConVal_index(c, 0, t[0]), get_QConVal_index(c, 1, t[1]) };
			const tuple<int, int> idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
			bitSup_[index[0]][get<0>(idx[1])] |= U64_MASK1[get<1>(idx[1])];
			bitSup_[index[1]][get<0>(idx[0])] |= U64_MASK1[get<1>(idx[0])];
		}
	}
}

PropagationState lMaxRPC_bit_rm::propagate(vector<QVar*>& x_evt, const int p) {
	//q_.clear();
	//for (auto i : x_evt) {
	//	for (int a = i->head(p); a != Limits::INDEX_OVERFLOW; i->next_value(a, p)) {
	//		for (auto j : neibor_vars[i->id]) {
	//			if (have_no_PC_support(i, a, j, p)) {
	//				i->remove_value(a, p);
	//				if (!i->faild(p)) {
	//					q_.push(i);
	//				}
	//				else {
	//					++con_weight[neibor_matrix[i->id][j->id]->id];
	//					ps_.state = false;
	//					return ps_;
	//				}
	//			}
	//		}
	//	}
	//}
	//q_.clear();
	//bool changed = false;
	//for (auto v : x_evt)
	//	q_.push(v);

	//while (!q_.empty()) {
	//	const auto j = q_.pop();
	//	for (auto i : neibor_vars[j->id]) {
	//		for (int a = i->head(p); a != Limits::INDEX_OVERFLOW; i->next_value(a, p)) {
	//			const auto c = neibor_matrix[i->id][j->id];
	//			const auto idx = get_QConVal_index(c, i, a);
	//			if (j->have(last_pc[idx], p) && have_no_PC_support(i, a, j, p)) {
	//				i->remove_value(a, p);
	//				if (!i->faild(p)) {
	//					q_.push(i);
	//				}
	//				else {
	//					++con_weight[c->id];
	//					ps_.state = false;
	//					return ps_;
	//				}
	//			}
	//		}
	//	}
	//}

	//ps_.state = true;
	//return ps_;
	q_.clear();
	bool changed = false;
	for (auto v : x_evt)
		q_.push(v);

	while (!q_.empty()) {
		const auto j = q_.pop();
		for (auto i : neibor_vars[j->id]) {
			if (I_.assigned(i))
				continue;

			const auto c = neibor_matrix[i->id][j->id];
			for (int a = j->head(p); a != Limits::INDEX_OVERFLOW; j->next_value(a, p)) {
				if (!search_pc_sup(i, a, j, p)) {
					i->remove_value(a, p);
					changed = true;
				}
			}

			if (changed) {
				if (i->faild(p)) {
					++con_weight[c->id];
					ps_.state = false;
					return ps_;
				}
				q_.push(i);
			}
		}
	}

	ps_.state = true;
	return ps_;
}

bool lMaxRPC_bit_rm::search_pc_sup(QVar* const i, const int a, QVar* j, const int p) {
	const auto c = neibor_matrix[i->id][j->id];
	const int cval_cia_idx = get_QConVal_index(c, i, a);
	const int lastPCiaj = last_pc[cval_cia_idx];
	if (lastPCiaj != Limits::INDEX_OVERFLOW && j->have(lastPCiaj, p))
		return true;
	for (int b = j->head(p); b != Limits::INDEX_OVERFLOW; j->next_value(b, p)) {
		if (is_consistent(i, a, j, b, p)) {
			if (search_pc_wit(i, a, j, b, p)) {
				const int cval_cjb_idx = get_QConVal_index(c, j, b);
				last_pc[cval_cia_idx] = b;
				last_pc[cval_cjb_idx] = a;
				return true;
			}
		}
	}
	return false;
}
bool lMaxRPC_bit_rm::is_consistent(QVar* const i, const int a, QVar* j, const int b, const int p) {
	const auto c = neibor_matrix[i->id][j->id];
	const int idx[] = { get_QConVal_index(c, i, a) , get_QConVal_index(c, j, b) };
	const tuple<int, int> index[] = { GetBitIdx(a),GetBitIdx(b) };
	if (!(bitSup_[idx[0]][get<0>(index[1])] & j->bitDom(p)[get<0>(index[1])]))
		return false;
	if (!(bitSup_[idx[1]][get<0>(index[0])] & j->bitDom(p)[get<0>(index[0])]))
		return false;
	return true;
}

bool lMaxRPC_bit_rm::search_pc_wit(QVar* const i, const int a, QVar* j, const int b, const int p) {
	for (auto k : pc_nei_[i->id][j->id]) {
		bool maxRPCsupport = false;
		const auto c_ik = neibor_matrix[i->id][k->id];
		const auto c_jk = neibor_matrix[j->id][k->id];
		const auto cva_ikia = get_QConVal_index(c_ik, i, a);
		const auto cva_jkjb = get_QConVal_index(c_jk, j, b);

		const int bitdom_size = k->bitDom(p).size();
		for (int c = 0; c < bitdom_size; ++c)
			if (bitSup_[cva_ikia][c] & bitSup_[cva_jkjb][c] & k->bitDom(p)[c]) {
				maxRPCsupport = true;
				break;
			}

		if (!maxRPCsupport)
			return false;
	}
	return true;
}

bool lMaxRPC_bit_rm::have_no_PC_support(QVar* i, const int a, QVar* j, const int p) {
	const auto c = neibor_matrix[i->id][j->id];
	const auto idx1 = get_QConVal_index(c, i, a);

	for (int b = j->head(p); b != Limits::INDEX_OVERFLOW; j->next_value(b, p)) {
		int PCWitness = true;
		auto b_idx = GetBitIdx(b);
		if (bitSup_[idx1][get<0>(b_idx)] | U64_MASK1[get<1>(b_idx)]) {
			for (auto k : pc_nei_[i->id][j->id]) {
				if (!have_PC_wit(i, a, j, b, k, p)) {
					PCWitness = false;
					break;
				}
			}
			if (PCWitness != false) {
				const auto idx2 = get_QConVal_index(c, j, b);
				last_pc[idx1] = b;
				last_pc[idx2] = a;
				last_ac[idx1] = b / BITSIZE;
				return false;
			}
		}
	}

	return true;
}
bool lMaxRPC_bit_rm::have_PC_wit(QVar* i, const int a, QVar* j, const int b, QVar* k, const int p) {
	const auto c_ik = neibor_matrix[i->id][k->id];
	const auto c_jk = neibor_matrix[j->id][k->id];
	const auto cva_ikia = get_QConVal_index(c_ik, i, a);
	const auto cva_jkjb = get_QConVal_index(c_jk, j, b);
	//const auto cva_ikia = m_->GetIntConValIndex(c_ik->id(), i->id(), a);
	if (last_ac[cva_ikia] != Limits::INDEX_OVERFLOW) {
		const auto d = last_ac[cva_ikia];
		if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k->bitDom(p)[d]) {
			return true;
		}
		//const auto d = last_ac[cva_ikia];
		//const auto idx = GetBitIdx(d);
		//if (bitSup_[cva_ikia][get<0>(idx)].test(get<1>(idx))&
		//	bitSup_[cva_jkjb][get<0>(idx)].test(get<1>(idx))&
		//	k->bitDom(level_)[get<0>(idx)].test(get<1>(idx))) {
		//	return true;
		//}
	}

	if (last_ac[cva_jkjb] != Limits::INDEX_OVERFLOW) {
		const auto d = last_ac[cva_jkjb];
		if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k->bitDom(p)[d]) {
			return true;
		}
		//const auto idx = GetBitIdx(d);
		//if (bitSup_[cva_ikia][get<0>(idx)].test(get<1>(idx))&
		//	bitSup_[cva_jkjb][get<0>(idx)].test(get<1>(idx))&
		//	k->bitDom(level_)[get<0>(idx)].test(get<1>(idx))) {
		//	return true;
		//}
	}

	//const auto c_ik = nei_[i->id()][k->id()];
	//const auto c_jk = nei_[j->id()][k->id()];
	//const int cva_ikia = m_->GetIntConValIndex(c_ik->id(), i->id(), a);
	//const int cva_ikia = m_->GetIntConValIndex(c_jk->id(), j->id(), b);
	const int bitdom_size = k->bitDom(p).size();
	for (int c = 0; c < bitdom_size; ++c)
		if (bitSup_[cva_ikia][c] & bitSup_[cva_jkjb][c] & k->bitDom(p)[c]) {
			last_ac[cva_ikia] = c;
			last_ac[cva_jkjb] = c;
			return  true;
		}
	return false;
}
}



