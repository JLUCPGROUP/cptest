#include "BacktrackingSearch.h"

namespace cp {
	SAC::SAC(const HModel& h, const bool backtrackable) :
		BacktrackingSearch(h, backtrackable) {
		copy_level(0, tmp_);
		x_evt_.reserve(num_vars);
	}
	int SAC::num_del() const {
		return ps_.num_delete;
	}
	/////////////////////////////////////////////////////////////////////////
	SAC1bit::SAC1bit(const HModel& h, const bool backtrackable) :
		SAC(h, backtrackable) {
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

	SAC1bit::~SAC1bit() {
		for (int i = 0; i < num_cva; ++i)
			delete[] bitSup_[i];
		delete[] bitSup_;
	}

	PropagationState SAC1bit::propagate(vector<QVar*>& x_evt, const int p) {
		int del = 0;
		bool result = enforce_ac(vars, del, 0);
		ps_.num_delete += del;
		x_evt_.clear();
		auto modified = false;
		//cout << "ac remove: " << del << endl;
		//cout << "initial ac: " << ps_.num_delete << endl;

		//show(tmp_);
		if (!result) {
			ps_.state = false;
			return ps_;
		}

		do {
			modified = false;
			for (auto x : vars) {
				for (int a = x->head(p); a != Limits::INDEX_OVERFLOW; x->next_value(a, p)) {
					//cout << "(" << x->id << "," << a << ")" << endl;
					copy_level(p, tmp_);
					x->reduce_to(a, tmp_);
					x_evt_.push_back(x);
					result = enforce_ac(x_evt_, del, tmp_);
					x_evt_.clear();

					if (!result) {
						//cout << "delete: (" << x->id << "," << a << ")" << endl;
						//cout << "remove: 1" << endl;
						//cout << "num_delete: " << ps_.num_delete << endl;
						++ps_.num_delete;
						x->remove_value(a, p);
						x_evt_.push_back(x);
						result = enforce_ac(x_evt_, del, p);
						x_evt_.clear();
						ps_.num_delete += del;
						//cout << "ac remove: " << del << endl;
						//cout << "num_delete: " << ps_.num_delete << endl;

						if (!result) {
							ps_.state = false;
							return ps_;
						}

						modified = true;
					}
				}
			}
		} while (modified);

		ps_.state = true;
		return ps_;
	}


	inline bool SAC1bit::enforce_ac(vector<QVar*>& x_evt, int& del, const int p) {
		del = 0;
		q_.clear();
		for (auto v : x_evt)
			insert(*v, p);

		while (!q_.empty()) {
			QVar* x = q_.pop(p);
			for (QTab* c : subscription[x->id]) {
				if (var_stamp[x->id] > con_stamp[c->id]) {
					QVar* y = (c->scope[0] == x) ? c->scope[1] : c->scope[0];
					const auto a = var_stamp[y->id] > con_stamp[c->id];
					for (auto z : c->scope) {
						if (!I.assigned(*z)) {
							if ((z != x) || a) {
								const auto revise_del = revise(*c, *z, p);
								del += revise_del;
								if (revise_del) {
									if (z->faild(p)) {
										++(con_weight[c->id]);
										return false;
									}
									insert(*z, p);
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

	inline int SAC1bit::revise(const QTab& c, const QVar& v, const int level) const {
		int del = 0;
		const int num_elements = v.size(level);
		int a = v.head(level);
		while (a != Limits::INDEX_OVERFLOW) {
			if (!seek_support(c, v, a, level)) {
				v.remove_value(a, level);
				++del;
			}
			a = v.next(a, level);
		}
		return del;
	}

	inline bool SAC1bit::seek_support(const QTab& c, const QVar& v, const int a, const int p) const {
		const u64 idx = get_QConVal_index(c, v, a);
		for (QVar* y : c.scope)
			if (y->id != v.id)
				for (int i = 0; i < y->num_bit; ++i)
					if (bitSup_[idx][i] & y->bitDom(p)[i])
						return true;
		return false;
	}


}
