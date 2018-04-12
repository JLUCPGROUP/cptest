//#include "BacktrackingSearch.h"
//namespace cp {
//	FC::FC(const HModel& h, const bool backtrackable)
//		:BacktrackingSearch(h, backtrackable) {
//
//		rel_.resize(num_vars, vector<vector<vector<int>>>(num_vars, vector<vector<int>>(max_dom_size, vector<int>(max_dom_size, false))));
//		for (QTab* c : tabs) {
//			const int x = c->scope[0]->id;
//			const int y = c->scope[1]->id;
//			for (auto& t : c->tuples) {
//				rel_[x][y][t[0]][t[1]] = true;
//				rel_[y][x][t[1]][t[0]] = true;
//			}
//		}
//	}
//
//	FC::~FC() {
//
//	}
//
//	PropagationState FC::propagate(vector<QVar*>& x_evt, const int p) {
//		if (p > 0) {
//			if (x_evt.size() > 1) {
//				cout << "find!" << endl;
//			}
//			auto x = x_evt[0];
//			for (auto c : subscription[x->id]) {
//				for (auto y : c->scope) {
//					if (revise(*c, *y, p)) {
//						if (y->faild(p)) {
//							ps_.state = false;
//							return ps_;
//						}
//					}
//				}
//			}
//		}
//		ps_.state = true;
//		return ps_;
//	}
//
//	inline bool FC::revise(const QTab & c, const QVar & v, const int level) {
//		++ss_.num_revisions;
//		const int num_elements = v.size(level);
//		int a = v.head(level);
//		while (a != Limits::INDEX_OVERFLOW) {
//			if (!seek_support(c, v, a, level)) {
//				v.remove_value(a, level);
//				//cout << "(" << v.id << ", " << a << ")" << endl;
//				++ps_.num_delete;
//			}
//			a = v.next(a, level);
//		}
//		return num_elements != v.size(level);
//	}
//
//	inline bool FC::seek_support(const QTab& c, const QVar& x, const int a, const int p) {
//		const auto y = (c.scope[0]->id == x.id) ? c.scope[1] : c.scope[0];
//
//		for (auto b = y->head(p); b != Limits::INDEX_OVERFLOW; y->next_value(b, p)) {
//			if (is_consistent(x, a, *y, b)) {
//				return true;
//			}
//		}
//		return false;
//	}
//
//	inline bool FC::is_consistent(const QVar& x, const int a, const QVar& y, const int b) {
//		return rel_[x.id][y.id][a][b];
//	}
//
//}