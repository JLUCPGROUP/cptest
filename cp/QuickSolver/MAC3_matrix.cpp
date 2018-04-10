#include "BacktrackingSearch.h"
namespace cp {

	MAC3_matrix::MAC3_matrix(const HModel& h, const bool backtrackable)
		:BacktrackingSearch(h, backtrackable) {

		rel_.resize(num_vars, vector<vector<vector<int>>>(num_vars, vector<vector<int>>(max_dom_size, vector<int>(max_dom_size, false))));
		for (QTab* c : tabs) {
			const int x = c->scope[0]->id;
			const int y = c->scope[1]->id;
			for (auto& t : c->tuples) {
				rel_[x][y][t[0]][t[1]] = true;
				rel_[y][x][t[1]][t[0]] = true;
			}
		}
	}
	MAC3_matrix::~MAC3_matrix() {

	}

	PropagationState MAC3_matrix::propagate(vector<QVar*>& x_evt, const int p) {
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
										ps_.tab = c;
										ps_.var = z;
										++(con_weight[c->id]);
										ps_.state = false;
										return ps_;
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

		ps_.state = true;
		return ps_;
	}

	inline bool MAC3_matrix::revise(const QTab & c, const QVar & v, const int level) {
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

	inline bool MAC3_matrix::seek_support(const QTab& c, const QVar& x, const int a, const int p) {
		const auto y = (c.scope[0]->id == x.id) ? c.scope[1] : c.scope[0];

		for (auto b = y->head(p); b != Limits::INDEX_OVERFLOW; y->next_value(b, p)) {
			if (is_consistent(x, a, *y, b)) {
				return true;
			}
		}
		return false;
	}

	inline bool MAC3_matrix::is_consistent(const QVar& x, const int a, const QVar& y, const int b) {
		return rel_[x.id][y.id][a][b];
	}
}
