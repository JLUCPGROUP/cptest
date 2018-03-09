﻿#include "BitSearch.h"
#include "Timer.h"

namespace cp {
	const BVal& BVal::operator=(const BVal& rhs) {
		v = rhs.v;
		a = rhs.a;
		aop = rhs.aop;
		return *this;
	}

	void BVal::flip() {
		aop = !aop;
	}

	BVal BVal::next() const {
		return BVal(v, a + 1, true);
	}

	bool BVal::operator==(const BVal& rhs) {
		return (this == &rhs) || (v == rhs.v && a == rhs.a && aop == rhs.aop);
	}

	bool BVal::operator!=(const BVal& rhs) {
		return !((this == &rhs) || (v == rhs.v && a == rhs.a && aop == rhs.aop));
	}

	std::ostream& operator<<(std::ostream& os, const BVal& v_val) {
		const string s = (v_val.aop) ? " = " : " != ";
		os << "(" << v_val.v << s << v_val.a << ")";
		return os;
	}
	////////////////////////////////////////////////////////////////
	void bit_assigned_stack::initial(const int num_vars) {
		max_size_ = num_vars;
		vals_.resize(num_vars);
		asnd_.resize(num_vars, -1);
	}

	void bit_assigned_stack::push(const BVal& v_a) {
		//const int pre = top_ - 1;
		////进入的是positive decision 当前栈顶是negative decision
		//if (pre >= 0 && (!vals_[pre].aop) && v_a.aop) {
		//	vals_[pre] = v_a;
		//	asnd_[v_a.v] = true;
		//}
		//else {
		vals_[top_] = v_a;
		asnd_[v_a.v] = v_a.aop ? v_a.a : -1;
		++top_;
		//}
	}

	BVal bit_assigned_stack::pop() {
		--top_;
		asnd_[vals_[top_].v] = -1;
		return vals_[top_];
	};

	BVal bit_assigned_stack::top() const { return vals_[top_]; };
	int bit_assigned_stack::size() const { return top_; }
	int bit_assigned_stack::capacity() const { return max_size_; }
	bool bit_assigned_stack::full() const { return top_ == max_size_; }
	bool bit_assigned_stack::empty() const { return top_ == 0; }
	BVal bit_assigned_stack::operator[](const int i) const { return vals_[i]; };
	BVal bit_assigned_stack::at(const int i) const { return vals_[i]; }
	void bit_assigned_stack::clear() { top_ = 0; };
	bool bit_assigned_stack::assigned(const int v) const { return asnd_[v] != -1; };

	std::ostream& operator<<(std::ostream& os, const bit_assigned_stack& I) {
		for (int i = 0; i < I.size(); ++i)
			os << I[i] << " ";
		return os;
	}

	////////////////////////////////////////////////////////////////


	BitSearch::BitSearch(const HModel& m) :
		top_(0),
		limit(m.max_domain_size() & MOD_MASK),
		num_bit(ceil(float(m.max_domain_size()) / BITSIZE)),
		num_vars(m.vars.size()),
		num_cons(m.tabs.size()),
		max_arity(m.max_arity()),
		max_dom_size(m.max_domain_size()),
		max_bitDom_size(ceil(float(m.max_domain_size()) / BITSIZE)),
		len_stack(m.vars.size() + 1) {
		sac = new SAC1bit(m);
		bit_dom_ = new u64*[num_vars];

		//初始化bitDom
		for (int i = 0; i < num_vars; ++i) {
			bit_dom_[i] = new u64[max_bitDom_size]();
			//memset(bit_dom_[i], ULLONG_MAX, max_bitDom_size * sizeof(u64));
			//bit_dom_[i][max_bitDom_size - 1] >>= BITSIZE - limit;
		}

		//初始化stack
		stack_ = new u64**[len_stack];
		for (int i = 0; i < len_stack; ++i) {
			stack_[i] = new u64*[num_vars];
			for (int j = 0; j < num_vars; j++) {
				stack_[i][j] = new u64[num_bit]();
			}
		}

		////给stack_[0]赋值
		//for (int i = 0; i < num_vars; ++i) {
		//	for (int j = 0; j < max_bitDom_size; j++) {
		//		stack_[0][i][j] = bit_dom_[i][j];
		//	}
		//}

		bit_sub_dom_ = new u64***[num_vars];
		for (int i = 0; i < num_vars; ++i) {
			bit_sub_dom_[i] = new u64**[max_dom_size];
			for (int j = 0; j < max_dom_size; ++j) {
				bit_sub_dom_[i][j] = new u64*[num_vars];
				for (int k = 0; k < num_vars; ++k) {
					bit_sub_dom_[i][j][k] = new u64[max_bitDom_size]();
				}
			}
		}

		constraint_matrix = new int*[num_vars];
		deg = new double[num_vars]();
		for (int i = 0; i < num_vars; ++i) {
			constraint_matrix[i] = new int[num_vars];
			memset(constraint_matrix, -1, num_vars * sizeof(int));
		}

		for (auto x : m.vars)
			for (auto y : m.vars)
				if (x != y)
					if (!sac->neibor_matrix[x->id][y->id].empty()) {
						constraint_matrix[x->id][y->id] = sac->neibor_matrix[x->id][y->id][0]->id;
						++deg[x->id];
					}

		wdeg = new double*[num_vars];
		for (int i = 0; i < num_vars; ++i)
			wdeg[i] = new double[num_vars]();

		I.initial(num_vars);
	}

	bool BitSearch::initial() {
		return false;
	}

	void BitSearch::build_nei_model() const {
		vector<QVar*> x_evt;
		int del;
		//初始化bitDom
		for (int i = 0; i < num_vars; ++i) {
			memset(bit_dom_[i], ULLONG_MAX, max_bitDom_size * sizeof(u64));
			bit_dom_[i][max_bitDom_size - 1] >>= BITSIZE - limit;
		}

		for (auto c : sac->tabs) {
			for (auto t : c->tuples) {
				const BitIndex idx[] = { GetBitIdx(t[0]), GetBitIdx(t[1]) };
				bit_sub_dom_[c->scope[0]->id][t[0]][c->scope[1]->id][idx[1].x] |= U64_MASK1[idx[1].y];
				bit_sub_dom_[c->scope[1]->id][t[1]][c->scope[0]->id][idx[0].x] |= U64_MASK1[idx[0].y];
			}
		}

		for (int i = 0; i < num_vars; ++i) {
			for (int j = 0; j < num_vars; ++j) {
				const int size_i = sac->vars[i]->size(0);
				const int size_j = sac->vars[j]->size(0);
				if (i != j) {
					// 变量i与j不相同
					if (!constraint_matrix[i][j] != -1) {
						//有直接约束关系
						wdeg[i][j] = 1;
						wdeg[j][i] = 1;
					}
					else {
						//无约束关系，都置为1
						for (int a = 0; a < size_i; ++a) {
							for (int b = 0; b < size_j; ++b) {
								const BitIndex idx = GetBitIdx(b);
								bit_sub_dom_[i][a][j][idx.x] |= U64_MASK1[idx.y];
							}
						}
					}
				}
				else {
					// 变量i与j相同，对角线上的值都标记1
					//const int min_size = std::min(sac->vars[i]->size(0), sac->vars[j]->size(0));
					for (int c = 0; c < size_i; ++c) {
						const BitIndex idx = GetBitIdx(c);
						bit_sub_dom_[i][c][j][idx.x] |= U64_MASK1[idx.y];

					}
					//for (int a = 0; a < sac->vars[i]->size(0); ++a) {
					//	for (int b = 0; b < sac->vars[j]->size(0); ++b) {
					//		if (a == b) {
					//			const BitIndex idx = GetBitIdx(b);
					//			bit_sub_dom_[i][a][j][idx.x] |= U64_MASK1[idx.y];
					//		}
					//	}
					//}
				}
			}
		}

	}

	void BitSearch::build_full_model() const {
		vector<QVar*> x_evt;
		int del;
		for (auto x : sac->vars) {
			for (auto a = x->head(1); a != Limits::INDEX_OVERFLOW; x->next_value(a, 1)) {
				const auto idx_a = GetBitIdx(a);
				bit_dom_[x->id][idx_a.x] |= U64_MASK1[idx_a.y];
				sac->copy_level(0, 1);
				x->reduce_to(a, 1);
				x_evt.push_back(x);
				sac->enforce_ac(x_evt, del, 1);
				x_evt.clear();

				for (auto y : sac->vars) {
					for (auto b = y->head(1); b != Limits::INDEX_OVERFLOW; y->next_value(b, 1)) {
						const auto idx_b = GetBitIdx(b);
						bit_sub_dom_[x->id][a][y->id][idx_b.x] |= U64_MASK1[idx_b.y];
					}
				}
			}
		}

		for (int i = 0; i < num_vars; ++i) {
			for (int j = 0; j < num_vars; ++j) {
				if (i != j) {
					wdeg[i][j] = 1;
					wdeg[j][i] = 1;
				}
			}
		}

	}

	bool BitSearch::propagate(const BVal& val) {
		const int pre = top_ - 1;;
		++top_;
		for (size_t i = 0; i < num_vars; ++i) {
			bool failed = false;
			for (int j = 0; j < max_bitDom_size; ++j) {
				stack_[pre + 1][i][j] = stack_[pre][i][j] & bit_sub_dom_[val.v][val.a][i][j];
				failed |= Count(stack_[pre + 1][i][j]);
				if (failed) {
					++wdeg[val.v][i];
					++wdeg[i][val.v];
					return false;
				}
			}
		}

		return true;
	}

	void BitSearch::back_level() {
		--top_;
	}

	bool BitSearch::nonbinary_search(const Heuristic::Var varh, const Heuristic::Val valh, const bool neibor_propagate, const int time_limits) {
		Timer t;
		neibor_prop = neibor_propagate;
		//执行SAC
		const bool sac_res = sac->propagate(sac->vars, 0).state;
		ss.initial_propagate_time = t.elapsed();
		ss.total_time += ss.initial_propagate_time;

		//初始化超时
		if (ss.total_time > time_limits) {
			ss.time_out = true;
			return false;
		}

		//不满足SAC
		if (!sac_res) {
			ss.sat_initial = false;
			return false;
		}

		t.reset();
		//建模
		if (neibor_propagate) {
			//邻域
			build_nei_model();
		}
		else {
			//全域
		}
		ss.build_time = t.elapsed();
		ss.total_time += ss.build_time;

		t.reset();
		bool finished = false;

		BVal val = select_BVal(varh, valh);
		bool consistent = false;
		while ((!I.empty()) || (val != Nodes::NullNode)) {
			if (t.elapsed() > time_limits) {
				//cout << t.elapsed() << endl;
				ss.solve_time = t.elapsed();
				ss.time_out = true;
				return false;
			}

			if (val != Nodes::NullNode) {
				++ss.nodes;
				I.push(val);
				consistent = propagate(val);
			}

			if (consistent) {
				if (I.full()) {
					ss.solve_time = t.elapsed();
					++ss.num_sol;
					cout << I << endl;
					return true;
				}
				else {
					val = select_BVal(varh, valh);
				}
			}

			if (!consistent) {
				back_level();
				val = I.pop();
				next(val);
			}
		}

		ss.solve_time = t.elapsed();
		return false;


	}

	BitSearch::~BitSearch() {
		delete sac;

		for (int i = 0; i < num_vars; ++i)
			delete[] bit_dom_[i];
		delete[] bit_dom_;


		for (int i = 0; i < max_bitDom_size; ++i) {
			for (int j = 0; j < num_vars; ++j) {
				delete[] stack_[i][j];
			}
			delete[] stack_[i];
		}
		delete[] stack_;

		for (int i = 0; i < num_vars; ++i) {
			for (int j = 0; j < max_dom_size; ++j) {
				for (int k = 0; k < num_vars; ++k) {
					delete[] bit_sub_dom_[i][j][k];
				}
				delete[] bit_sub_dom_[i][j];
			}
			delete[] bit_sub_dom_[i];
		}
		delete[] bit_sub_dom_;

		constraint_matrix = new int*[num_vars];
		deg = new double[num_vars]();
		for (int i = 0; i < num_vars; ++i) {
			delete[] constraint_matrix[i];
		}
		delete[] constraint_matrix;

		delete[] deg;

		for (int i = 0; i < num_vars; ++i)
			delete[] wdeg[i];
		delete[] wdeg;
	}

	inline BVal BitSearch::select_BVal(const Heuristic::Var varh, const Heuristic::Val valh) const {
		const int v = select_BVar(varh);
		const int a = select_value(v, valh);
		BVal val(v, a);
		return val;
	}

	inline int BitSearch::select_BVar(const Heuristic::Var varh) const {
		double min_size = DBL_MAX;
		double max_size = DBL_MIN;
		int var = -1;
		switch (varh) {
		case Heuristic::VRH_LEX:
			return I.size();
		case Heuristic::VRH_DOM_MIN: {
			for (int i = 0; i < num_vars; ++i) {
				if (!I.assigned(i)) {
					double cur_size = 0;
					for (int j = 0; j < num_bit; j++) {
						cur_size += Count(stack_[top_ - 1][i][j]);
					}
					if (cur_size < min_size) {
						min_size = cur_size;
						var = i;
					}
				}
			}
			return var;
		}
		case Heuristic::VRH_VWDEG: break;
		case Heuristic::VRH_DOM_DEG_MIN: break;
		case Heuristic::VRH_DOM_DDEG_MIN: break;
		case Heuristic::VRH_DOM_WDEG_MIN: break;
		default:;
		}
		return var;
	}

	inline int BitSearch::select_value(const int v, const Heuristic::Val valh) const {
		switch (valh) {
		case Heuristic::VLH_MIN: {
			for (int i = 0; i < num_bit; ++i) {
				if (stack_[top_ - 1][v][i]) {
					return FirstOne(stack_[top_ - 1][v][i]) + num_bit * BITSIZE;
				}
			}
			return -1;
		}
		case Heuristic::VLH_MIN_DOM: break;
		case Heuristic::VLH_MIN_INC: break;
		case Heuristic::VLH_MAX_INC: break;
		case Heuristic::VLH_VWDEG: break;
		default:;
		}
		return -1;
	}

	inline bool BitSearch::next(BVal& v_a) const {
		const auto index = GetBitIdx(v_a.a);
		const u64 b = (stack_[top_][v_a.v][index.x] >> index.y) >> 1;

		if (b) {
			v_a.a = v_a.a + 1 + FirstOne(b);
			return true;
		}

		for (int i = index.x + 1; i < num_bit; ++i) {
			if (stack_[top_][v_a.v][i]) {
				v_a.a = GetValue(i, FirstOne(stack_[top_][v_a.v][i]));
				return true;
			}
		}

		v_a.a = Limits::INDEX_OVERFLOW;
		return false;
	}
}
