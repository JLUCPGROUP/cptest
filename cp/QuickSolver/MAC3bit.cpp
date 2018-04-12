#include "BacktrackingSearch.h"
namespace cp {
	MAC3bit::MAC3bit(const HModel& h, const bool backtrackable) :
		BacktrackingSearch(h, backtrackable) {
		name = "MAC3bit";
		bitSup_ = new u64*[num_cva];
		for (size_t i = 0; i < num_cva; i++)
			bitSup_[i] = new u64[max_bitDom_size]();

		for (QTab* c : tabs) {
			for (auto t : c->tuples) {
				const int index[] = { get_QConVal_index(*c,0, t[0]), get_QConVal_index(*c, 1, t[1]) };
				const BitIndex idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
				bitSup_[index[0]][idx[1].x] |= U64_MASK1[idx[1].y];
				bitSup_[index[1]][idx[0].x] |= U64_MASK1[idx[0].y];
			}
		}
	}

	MAC3bit::~MAC3bit() {
		for (int i = 0; i < num_cva; ++i)
			delete[] bitSup_[i];
		delete[] bitSup_;
	}

	inline PropagationState MAC3bit::propagate(vector<QVar*>& x_evt, const int p) {
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

	inline bool MAC3bit::revise(const QTab& c, const QVar& v, const int level) {
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

	inline bool MAC3bit::seek_support(const QTab& c, const QVar& v, const int a, const int p) const {
		const int idx = get_QConVal_index(c, v, a);
		for (QVar* y : c.scope)
			if (y->id != v.id) {
				//cout << y->id << " " << v.id << endl;
				for (int i = 0; i < y->num_bit; ++i)
					if (bitSup_[idx][i] & y->bitDom(p)[i])
						return true;
			}
		return false;
	}

	bool MAC3bit::revise(const QVar& y, const QTab& c, const QVar& v, const int level) {
		QVar* x = (c.scope[0]->id == v.id) ? c.scope[1] : c.scope[0];
		++ss_.num_revisions;
		const int num_elements = v.size(level);
		int a = v.head(level);
		while (a != Limits::INDEX_OVERFLOW) {
			if (!seek_support(*x, c, v, a, level)) {
				v.remove_value(a, level);
				//cout << "(" << v.id << ", " << a << ")" << endl;
				++ps_.num_delete;
			}
			a = v.next(a, level);
		}
		return num_elements != v.size(level);
	}

	bool MAC3bit::seek_support(const QVar& y, const QTab& c, const QVar& v, const int a, const int p) const {
		//cout << y.id << " " << v.id << endl;
		const int idx = get_QConVal_index(c, v, a);
		for (int i = 0; i < y.num_bit; ++i)
			if (bitSup_[idx][i] & y.bitDom(p)[i])
				return true;
		return false;
	}
}
