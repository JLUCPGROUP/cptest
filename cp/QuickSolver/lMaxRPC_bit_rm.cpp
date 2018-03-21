#pragma once
#include "BacktrackingSearch.h"
#include <set>

namespace cp {

	lMaxRPC_bit_rm::lMaxRPC_bit_rm(const HModel& h, const bool backtrackable) :
		BacktrackingSearch(h, backtrackable) {
		//初始化变量三角关系
		common_neibor_.resize(num_vars, vector<vector<QVar*>>(num_vars));
		set<QVar*> vars_map;
		vector<bool> vars_in(num_vars, false);
		for (auto x : vars) {
			for (auto y : vars) {
				if (x != y) {
					for (auto z : vars) {
						if (!neibor_matrix[x->id][z->id].empty() && !neibor_matrix[y->id][z->id].empty()) {
							vars_map.insert(z);
							if (!vars_in[z->id]) {
								vars_in[z->id] = true;
								common_neibor_[x->id][y->id].push_back(z);
							}
						}
					}
				}
				vars_in.assign(vars_in.size(), false);
			}
		}

		last_pc.resize(num_tabs * max_dom_size*max_arity, Limits::INDEX_OVERFLOW);
		last_ac.resize(num_tabs * max_dom_size*max_arity, Limits::INDEX_OVERFLOW);

		//初始化bitSup
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
	}

	lMaxRPC_bit_rm::~lMaxRPC_bit_rm() {
		const int n = num_tabs * max_dom_size*max_arity;
		for (int i = 0; i < n; ++i)
			delete[] bitSup_[i];
		delete[] bitSup_;
	}

	PropagationState lMaxRPC_bit_rm::propagate(vector<QVar*>& x_evt, const int p) {
		q_.clear();
		ps_.level = p;
		ps_.num_delete = 0;

		for (auto v : x_evt)
			q_.push(*v, p);
		while (!q_.empty()) {
			const auto j = q_.pop(p);
			//cout << "pop: " << j->id << endl;
			for (auto i : neighborhood[j->id]) {
				if (I.assigned(*i)) {
					//cout << "assigned: " << i->id << endl;
					continue;
				}
				//cout << "neighborhood:" << i->id << endl;
				bool changed = false;
				const auto c = neibor_matrix[i->id][j->id][0];
				for (int a = i->head(p); a != Limits::INDEX_OVERFLOW; i->next_value(a, p)) {
					//cout << "have_pc_support(" << i->id << ", " << a << ", " << j->id << ", " << p << ")" << endl;
					if (!have_pc_support(*i, a, *j, p)) {
						i->remove_value(a, p);
						++ps_.num_delete;
						//cout << "delete: (" << i->id << "," << a << ")" << endl;
						if (i->faild(p)) {
							//cout << "faild: " << i->id << endl;
							++con_weight[c->id];
							ps_.state = false;
							return ps_;
						}
						changed = true;
					}
				}
				if (changed) {
					q_.push(*i, p);
					//cout << "push:" << i->id << endl;
				}
			}
		}
		ps_.state = true;
		return ps_;
	}

	bool lMaxRPC_bit_rm::have_pc_support(const QVar& i, const int a, const QVar& j, const int p) {
		const auto c = neibor_matrix[i.id][j.id][0];
		const auto cia_idx = get_QConVal_index(*c, i, a);
		const auto last_pc_iaj = last_pc[cia_idx];

		if (last_pc_iaj != Limits::INDEX_OVERFLOW && j.have(last_pc_iaj, p))
			return true;

		//for (auto b = j.head(p); b != Limits::INDEX_OVERFLOW; j.next_value(b, p)) {
		//	if (i.id == c->scope[0]->id) {
		//		tmp_tuple_[0] = a;
		//		tmp_tuple_[1] = b;
		//	}
		//	else {
		//		tmp_tuple_[0] = b;
		//		tmp_tuple_[1] = a;
		//	}

		//	if (c->sat(tmp_tuple_)) {
		//		bool pc_witness = true;
		//		for (auto k : common_neibor_[i.id][j.id]) {
		//			if (!I.assigned(*k)) {
		//				if (have_pc_wit(i, a, j, b, *k, p)) {
		//					const auto idx_ijjb = get_QConVal_index(*c, j, b);
		//					last_pc[cia_idx] = b;
		//					last_pc[idx_ijjb] = a;
		//					last_ac[cia_idx] = b / BITSIZE;
		//					return true;
		//				}
		//			}
		//		}
		//	}
		//}
		const int v = j.head(p);
		int b = next_support_bit(i, a, j, v, p);

		while (b != Limits::INDEX_OVERFLOW) {
			bool pc_witness = true;
			for (auto k : common_neibor_[i.id][j.id]) {
				if (!I.assigned(*k)) {
					if (!have_pc_wit(i, a, j, b, *k, p)) {
						pc_witness = false;
						break;
					}
				}
			}

			if (pc_witness) {
				const auto cjb_idx = get_QConVal_index(*c, j, b);
				last_pc[cia_idx] = b;
				last_pc[cjb_idx] = a;
				last_ac[cia_idx] = b / BITSIZE;
				return true;
			}
			b = next_support_bit(i, a, j, b + 1, p);
		}
		return false;
	}

	bool lMaxRPC_bit_rm::have_pc_wit(const QVar& i, const int a, const QVar& j, int b, const QVar& k, const int p) {

		const auto c_ik = neibor_matrix[i.id][k.id][0];
		const auto c_jk = neibor_matrix[j.id][k.id][0];
		const auto cva_ikia = get_QConVal_index(*c_ik, i, a);
		const auto cva_jkjb = get_QConVal_index(*c_jk, j, b);

		if (last_ac[cva_ikia] != Limits::INDEX_OVERFLOW) {
			const auto d = last_ac[cva_ikia];
			if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k.bitDom(p)[d]) {
				return true;
			}
		}

		if (last_ac[cva_jkjb] != Limits::INDEX_OVERFLOW) {
			const auto d = last_ac[cva_jkjb];
			if (bitSup_[cva_ikia][d] & bitSup_[cva_jkjb][d] & k.bitDom(p)[d]) {
				return true;
			}
		}

		for (int c = 0; c < k.num_bit; ++c)
			if (bitSup_[cva_ikia][c] & bitSup_[cva_jkjb][c] & k.bitDom(p)[c]) {
				last_ac[cva_ikia] = c;
				last_ac[cva_jkjb] = c;
				return true;
			}
		return false;
	}

	int lMaxRPC_bit_rm::next_support_bit(const QVar& i, const int a, const QVar& j, const int v, const int p) {
		//由于若传入的v已越界
		if (v > j.capacity - 1 || v == Limits::INDEX_OVERFLOW)
			return Limits::INDEX_OVERFLOW;

		const auto c = neibor_matrix[i.id][j.id][0];
		const int idx_cia = get_QConVal_index(*c, i, a);
		const auto index = GetBitIdx(v);
		const u64 b = ((bitSup_[idx_cia][index.x] & j.bitDom(p)[index.x]) >> index.y);
		
		if (b)
			return v + FirstOne(b);

		for (int u = index.x + 1; u < j.num_bit; ++u) {
			const u64 mask = bitSup_[idx_cia][u] & j.bitDom(p)[u];
			if (mask)
				return GetValue(u, FirstOne(mask));
		}

		return Limits::INDEX_OVERFLOW;
	}


}
