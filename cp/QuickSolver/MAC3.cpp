#include "BacktrackingSearch.h"
namespace cp {

	MAC3::MAC3(const HModel& h, const bool backtrackable)
		:BacktrackingSearch(h, backtrackable) {

	}
	MAC3::~MAC3() {

	}

	PropagationState MAC3::propagate(vector<QVar*>& x_evt, const int p) {
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
	inline bool MAC3::revise(const QTab & c, const QVar & v, const int level) {
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

	inline bool MAC3::seek_support(const QTab& c, const QVar& v, const int a, const int p) {
		c.get_first_valid_tuple(v, a, tmp_tuple_, p);
		while (Existed(tmp_tuple_)) {
			if (c.sat(tmp_tuple_))
				return true;
			else
				c.get_next_valid_tuple(v, a, tmp_tuple_, p);
		}
		return false;
	}
}
