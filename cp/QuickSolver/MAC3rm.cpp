#include "BacktrackingSearch.h"
namespace cp {

	//MAC3rm::MAC3rm(const HModel& h) :BacktrackingSearch(h) {
	//	Exclude(tmp_tuple_);
	//	res_.resize(tabs.size()*max_dom_size*max_arity, tmp_tuple_);
	//}
	MAC3rm::MAC3rm(const HModel& h) :
	BacktrackingSearch(h) {
		Exclude(tmp_tuple_);
		res_.resize(num_cva, tmp_tuple_);
		//const int num_res_ = num_tabs * max_dom_size * max_arity;
	}

	MAC3rm::~MAC3rm() {

	}

	inline PropagationState MAC3rm::propagate(vector<QVar*>& x_evt, const int p) {
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
						if (!I_.assigned(*z)) {
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

	inline bool MAC3rm::revise(const QTab & c, const QVar & v, const int level) {
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

	inline bool MAC3rm::seek_support(const QTab& c, const QVar& v, const int a, const int p) {
		tmp_tuple_ = res_[get_QConVal_index(c, v, a)];
		if (c.is_valid(tmp_tuple_, p))
			return true;

		c.get_first_valid_tuple(v, a, tmp_tuple_, p);
		while (Existed(tmp_tuple_)) {
			if (c.sat(tmp_tuple_)) {
				for (int i = 0; i < c.arity; ++i)
					res_[get_QConVal_index(c, i, tmp_tuple_[i])] = tmp_tuple_;
				return true;
			}
			c.get_next_valid_tuple(v, a, tmp_tuple_, p);
		}

		return false;
	}

	//bool MAC3rm::seek_support(const QTab& c, const QVar& v, const int a, const int p) {

	//}
}
