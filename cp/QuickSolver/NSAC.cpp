#include "BacktrackingSearch.h"
namespace cp {
	NSAC::NSAC(const HModel& h, const bool backtrackable) :BacktrackingSearch(h, backtrackable) {
		//initial bitSup
		const int n = num_tabs * max_dom_size * max_arity;
		bitSup_ = new u64*[n];
		for (size_t i = 0; i < n; i++)
			bitSup_[i] = new u64[max_bitDom_size]();

		for (QTab* c : tabs) {
			for (auto t : c->tuples) {
				const int index[] = { get_QConVal_index(*c,0, t[0]), get_QConVal_index(*c, 1, t[1]) };
				const BitIndex idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
				bitSup_[index[0]][idx[1].x] |= U64_MASK1[idx[1].y];
				bitSup_[index[1]][idx[0].x] |= U64_MASK1[idx[0].y];
			}
		}
		/////
		q_nei_.initial(num_vars);
		N.resize(num_vars);
		//生成邻域子网矩阵
		for (auto v : vars) {
			for (auto x : neighborhood[v->id]) {
				for (auto y : neighborhood[v->id]) {
					//只计算三角形，避免重复加入约束
					if (x->id < y->id && !neibor_matrix[x->id][y->id].empty()) {
						N[v->id].push_back(neibor_matrix[x->id][y->id][0]);
					}
				}
			}
		}

		rel_.resize(num_vars, vector<vector<vector<int>>>(num_vars, vector<vector<int>>(max_dom_size, vector<int>(max_dom_size, false))));
		for (QTab* c : tabs) {
			const int x = c->scope[0]->id;
			const int y = c->scope[1]->id;
			for (auto& t : c->tuples) {
				rel_[x][y][t[0]][t[1]] = true;
				rel_[y][x][t[1]][t[0]] = true;
			}
		}

		neibor_cons_.resize(num_vars, vector<vector<QTab*>>(num_vars));

		for (auto v : vars) {
			for (auto x : neighborhood[v->id]) {
				for (auto y : neighborhood[v->id]) {
					//只计算三角形，避免重复加入约束
					if (x->id < y->id && !neibor_matrix[x->id][y->id].empty()) {
						N[v->id].push_back(neibor_matrix[x->id][y->id][0]);
					}
				}
			}
		}

		Q.MakeQue(num_tabs, max_arity);
		//var_mark_.resize(num_vars, -1);
	}

	NSAC::~NSAC() {
		Q.DeleteQue();
		// free bitSup
		const int n = num_tabs * max_dom_size*max_arity;
		for (int i = 0; i < n; ++i)
			delete[] bitSup_[i];
		delete[] bitSup_;
	}

	PropagationState NSAC::propagate(vector<QVar*>& x_evt, const int p) {}

	int NSAC::ac(vector<QVar*>& x_evt, const int p) {
		q_.clear();
		ps_.level = p;
		ps_.num_delete = 0;

		for (auto v : x_evt)
			insert(*v, p);

		while (!q_.empty()) {
			QVar* x = q_.pop(p);
			//cout << "pop:" << x->id << endl;
			for (QTab* c : subscription[x->id]) {
				if (var_stamp[x->id] > con_stamp[c->id]) {
					QVar* y = (c->scope[0] == x) ? c->scope[1] : c->scope[0];
					const auto a = var_stamp[y->id] > con_stamp[c->id];
					for (auto z : c->scope) {
						if (!I.assigned(*z)) {
							if ((z != x) || a) {
								//cout << "revise: " << c->id << "," << z->id << endl;
								if (revise(*c, *z, p)) {
									if (z->faild(p)) {
										return false;
									}
									insert(*z, p);
									//cout << "push:" << z->id << endl;
								}
							}
						}
					}
					con_stamp[c->id] = ++time;
				}
			}
		}

		return true;
	}


	int NSAC::neibor_ac(QVar& v, const int p) {
		q_.clear();
		ps_.level = p;
		ps_.num_delete = 0;

		insert(v, p);

		while (!q_.empty()) {
			QVar* x = q_.pop(p);
			//cout << "pop:" << x->id << endl;
			for (QTab* c : subscription[x->id]) {
				if (var_stamp[x->id] > con_stamp[c->id]) {
					QVar* y = (c->scope[0] == x) ? c->scope[1] : c->scope[0];
					const auto a = var_stamp[y->id] > con_stamp[c->id];
					for (auto z : c->scope) {
						if (!I.assigned(*z)) {
							if ((z != x) || a) {
								//cout << "revise: " << c->id << "," << z->id << endl;
								if (revise(*c, *z, p)) {
									if (z->faild(p)) {
										return false;
									}
									if (true) {

									}
									insert(*z, p);
									//cout << "push:" << z->id << endl;
								}
							}
						}
					}
					con_stamp[c->id] = ++time;
				}
			}
		}

		return true;
	}

	bool NSAC::neibor(const QVar& v, const QTab& c) const {
		if (c.scope[0]->id == v.id || c.scope[1]->id == v.id)
			return true;

		if (neibor(*c.scope[0], v) || neibor(*c.scope[1], v))
			return true;

		return false;
	}

	bool NSAC::neibor(const QVar& x, const QVar& y) const {
		return !neibor_matrix[x.id][y.id].empty();
	}

	inline bool NSAC::revise(const QTab & c, const QVar & v, const int level) {
		++ss_.num_revisions;
		const int num_elements = v.size(level);
		int a = v.head(level);
		while (a != Limits::INDEX_OVERFLOW) {
			if (!seek_support(c, v, a, level)) {
				v.remove_value(a, level);
				//cout << "(" << v.id << ", " << a << ")" << endl;
				++ps_.num_delete;
			}
			a = v.next(a, level);
		}
		return num_elements != v.size(level);
	}

	inline bool NSAC::seek_support(const QTab& c, const QVar& x, const int a, const int p) {
		const auto y = (c.scope[0]->id == x.id) ? c.scope[1] : c.scope[0];

		for (auto b = y->head(p); b != Limits::INDEX_OVERFLOW; y->next_value(b, p)) {
			if (is_consistent(x, a, *y, b)) {
				return true;
			}
		}
		return false;
	}

	inline bool NSAC::is_consistent(const QVar& x, const int a, const QVar& y, const int b) {
		return rel_[x.id][y.id][a][b];
	}

}
