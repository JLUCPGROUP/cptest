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

	PropagationState NSAC::propagate(vector<QVar*>& x_evt, const int p) {
		q_.clear();
		int res = false;

		if (p == 0)
			for (auto v : x_evt)
				q_.push(*v, 0);
		else
			for (auto v : neighborhood[x_evt[0]->id])
				if (!I.assigned(*v))
					q_.push(*v, p);

		res = ac(x_evt, p);

		if (!res) {
			ps_.state = false;
			return ps_;
		}

		while (!q_.empty()) {
			QVar* x = q_.pop(p);

			auto changed = false;
			for (auto a = x->head(p); a != Limits::INDEX_OVERFLOW; x->next_value(a, p)) {
				copy_level(p, tmp_);
				x->reduce_to(a, tmp_);
				res = neibor_ac(*x, tmp_);

				if (!res) {
					changed = true;
					x->remove_value(a, p);

					if (x->faild(p)) {
						res = false;
						ps_.state = false;
						return ps_;
					}
				}
			}

			if (changed&&res)
				for (auto v : neighborhood[x->id])
					if (!I.assigned(*v))
						q_.push(*x, p);
		}

		ps_.state = true;
		return ps_;

	}

	int NSAC::ac(vector<QVar*>& x_evt, const int p) {

		for (auto x : x_evt)
			for (auto c : subscription[x->id])
				for (auto v : c->scope)
					if (!I.assigned(*v) && v != x)
						Q.push(arc{ *c, *v });

		while (!Q.empty()) {
			arc c_x = Q.pop();

			if (revise(*c_x.c, *c_x.v, p)) {
				if (c_x.v->faild(p))
					return false;

				for (auto c : subscription[c_x.v->id])
					if (c != c_x.c)
						for (auto v : c->scope)
							if ((v != c_x.v) && (!I.assigned(*v)))
								Q.push(arc(*c, *v));
			}
		}

		return true;
	}

	int NSAC::ac(vars_heap& x_evt, const int p) {

		while (!x_evt.empty()) {
			const auto x = x_evt.pop(p);
			for (auto c : subscription[x->id])
				for (auto v : c->scope)
					if (!I.assigned(*v) && v != x)
						Q.push(arc{ *c, *v });
		}

		while (!Q.empty()) {
			arc c_x = Q.pop();

			if (revise(*c_x.c, *c_x.v, p)) {
				if (c_x.v->faild(p))
					return false;

				for (auto c : subscription[c_x.v->id])
					if (c != c_x.c)
						for (auto v : c->scope)
							if ((v != c_x.v) && (!I.assigned(*v)))
								Q.push(arc(*c, *v));
			}
		}

		return true;
	}


	int NSAC::neibor_ac(QVar& x, const int p) {
		for (auto c : subscription[x.id])
			for (auto v : c->scope)
				if (!I.assigned(*v) && v->id != x.id)
					Q.push(arc{ *c, *v });

		while (!Q.empty()) {
			const arc c_x = Q.pop();

			if (revise(*c_x.c, *c_x.v, p)) {
				if (c_x.v->faild(p))
					return false;

				for (auto c : subscription[c_x.v->id])
					if (c != c_x.c && neibor(x, *c))
						for (auto v : c->scope)
							if (v != c_x.v && !I.assigned(*v))
								Q.push(arc(*c, *v));
			}
		}

		return true;
	}

	bool NSAC::neibor(const QVar& v, const QTab& c) const {
		if (c.scope[0]->id == v.id || c.scope[1]->id == v.id)
			return true;

		if (neibor(*c.scope[0], v) && neibor(*c.scope[1], v))
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


