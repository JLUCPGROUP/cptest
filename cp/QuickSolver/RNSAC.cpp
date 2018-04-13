#include "BacktrackingSearch.h"
namespace cp {

	RNSQ::RNSQ(const HModel& h, const bool backtrackable) :BacktrackingSearch(h, backtrackable) {
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
					if (x->id > y->id && !neibor_matrix[x->id][y->id].empty()) {
						N[v->id].push_back(neibor_matrix[x->id][y->id][0]);
					}
				}
			}
		}

		var_mark_.resize(num_vars, -1);
	}

	RNSQ::~RNSQ() {
		// free bitSup
		const int n = num_tabs * max_dom_size*max_arity;
		for (int i = 0; i < n; ++i)
			delete[] bitSup_[i];
		delete[] bitSup_;
	}

	PropagationState RNSQ::propagate(vector<QVar*>& x_evt, const int p) {
		q_.clear();

		int res = false;

		//initial q_
		if (p == 0)
			for (auto v : x_evt)
				q_.push(*v, 0);
		else {
			const auto x = x_evt[0];
			for (auto v : neighborhood[x->id])
				if (!I.assigned(*v))
					q_.push(*v, p);
		}


		while (!q_.empty()) {
			QVar* x = q_.pop(p);
			bool deletion = false;
			int a = x->head(p);
			while (a != Limits::INDEX_OVERFLOW) {
				//creat a new sub network
				copy_level(p, tmp_);
				//reduce x to a 
				x->reduce_to(a, tmp_);
				res = condition_fc(*x, a, tmp_);
				if (!res) {
					x->remove_value(a, p);
					deletion = true;
				}
				//else if (res == 1) {
				//	//neibor_ac(*x, tmp_);
				//}

				if (x->faild(p)) {
					ps_.state = false;
					return ps_;
				}

				a = x->next(a, p);
			}

			if (deletion) {
				for (auto v : neighborhood[x->id])
					if (!I.assigned(*v))
						q_.push(*v, p);
			}
			//for (int a = x->head(p); a != Limits::INDEX_OVERFLOW; x->next_value(a, p)) {
			//	//creat a new sub network
			//	copy_level(p, tmp_);
			//	//reduce x to a 
			//	x->reduce_to(a, tmp_);
			//	res = condition_fc(*x, a, tmp_);
			//	if (!res) {
			//		x->remove_value(a, p);
			//		deletion = true;
			//	}
			//	if (x->faild(p)) {
			//		ps_.state = false;
			//		return ps_;
			//	}
			//	if (deletion) {
			//		for (auto v : neighborhood[x->id])
			//			if (!I.assigned(*v))
			//				q_.push(*v, p);
			//	}
			//}
		}

		ps_.state = true;
		return ps_;
	}

	//PropagationState RNSQ::propagate(vector<QVar*>& x_evt, const int p) {
	//	q_.clear();
	//	bool deletion = true;
	//	//initial q_
	//	if (p == 0)
	//		for (auto v : x_evt)
	//			q_.push(*v, 0);
	//	else
	//		for (auto v : neighborhood[x_evt[0]->id])
	//			if (!I.assigned(*v))
	//				q_.push(*v, p);
	//	while (!q_.empty()) {
	//		QVar* x = q_.pop(p);
	//		deletion = false;
	//		const int removals = revise_NSAC(*x, *x_evt[0], p);
	//		if (removals > 0) {
	//			if (x->faild(p)) {
	//				ps_.state = false;
	//				return ps_;
	//			}
	//			for (auto v : neighborhood[x->id]) {
	//				if (!I.assigned(*v)) {
	//					q_.push(*v, p);
	//				}
	//			}
	//		}
	//	}
	//	ps_.state = true;
	//	return ps_;
	//}

	int RNSQ::revise_NSAC(QVar& v, QVar& x, const int p) {
		int num = 0;
		for (auto a = v.head(p); a != Limits::INDEX_OVERFLOW; v.next_value(a, p)) {
			copy_level(p, tmp_);
			v.reduce_to(a, tmp_);
			const bool res = full_NSAC(v, x, tmp_);
			if (!res) {
				v.remove_value(a, p);
				++num;
			}
		}
		return num;
	}

	bool RNSQ::full_NSAC(QVar& v, QVar& x, const int p) {
		bool res;
		q_nei_.clear();
		for (auto c : subscription[v.id]) {
			QVar* y = c->scope[0]->id == v.id ? c->scope[1] : c->scope[0];
			res = revise(*c, *y, p);
			if (res) {
				q_nei_.push(*y, p);
				if (y->faild(p)) {
					++con_weight[c->id];
					return false;
				}
			}
		}
		//cout << "--" << endl;
		while (!q_nei_.empty()) {
			QVar* y = q_nei_.pop(p);
			for (auto c : subscription[y->id]) {
				QVar* z = c->scope[0]->id == y->id ? c->scope[1] : c->scope[0];

				if (!is_neibor(v, *z) || (v.id == z->id)) {
					continue;
				}
				res = revise(*c, *z, p);
				if (res) {
					if (z->faild(p)) {
						++con_weight[c->id];
						return false;
					}
				}
			}
		}
		return true;
	}

	inline bool RNSQ::is_neibor(QVar& x, QVar& y) {
		return !neibor_matrix[x.id][y.id].empty();
	}

	int RNSQ::condition_fc(QVar & v, const int a, const int p) {
		//q_nei_.clear();
		int res = 2;
		var_mark_.assign(var_mark_.size(), 0);
		//propagate on neibor variable
		for (auto x : neighborhood[v.id]) {
			//if (!I.assigned(*x)) {
			const auto c = neibor_matrix[x->id][v.id][0];
			if (revise(*c, *x, p)) {
				var_mark_[x->id] = 1;
				//q_nei_.push(*x, p);
				if (x->faild(p)) {
					return 0;
				}
				else if (x->size(p) == 1) {
					res = 1;
				}
			}
			//}
		}

		//propagate on neibor
		for (auto c : N[v.id]) {
			QVar* x = c->scope[0];
			QVar* y = c->scope[1];

			if (var_mark_[x->id] == 1 && !I.assigned(*y)) {
			//if (!I.assigned(*y)) {

				revise(*c, *y, p);
				if (y->faild(p)) {
					return 0;
				}
				else if (y->size(p) == 1) {
					res = 1;
				}
				//var_mark_[x->id] = 0;
			//}
			}

			if (var_mark_[y->id] == 1 && !I.assigned(*x)) {
			//if (!I.assigned(*x)) {

				revise(*c, *x, p);
				if (x->faild(p)) {
					return 0;
				}
				else if (x->size(p) == 1) {
					res = 1;
				}
				//var_mark_[y->id] = 0;
			//}
			}
		}


		return res;
		//
		//for (auto)
	}

	//int RNSQ::neibor_ac(const QVar& v, int p) {
	//	q_nei_.clear();

	//	for (auto x : neighborhood[v.id])
	//		if (!I.assigned(*x))
	//			q_nei_.push(*x, p);

	//	while (!q_nei_.empty()) {
	//		QVar* x = q_nei_.pop(p);
	//		for ()
	//	}
	//}

	//inline bool RNSQ::revise(const QTab& c, const QVar& v, const int level) {
	//	++ss_.num_revisions;
	//	const int num_elements = v.size(level);
	//	int a = v.head(level);
	//	while (a != Limits::INDEX_OVERFLOW) {
	//		if (!seek_support(c, v, a, level)) {
	//			v.remove_value(a, level);
	//			//cout << "(" << v.id << ", " << a << ")" << endl;
	//			++ps_.num_delete;
	//		}
	//		a = v.next(a, level);
	//	}
	//	return num_elements != v.size(level);
	//}

	//inline bool RNSQ::revise(const QTab& c, const QVar& x, const QVar& v, const int p) {
	//	QVar* z = c.scope[0]->id == v.id ? c.scope[1] : c.scope[0];
	//	++ss_.num_revisions;
	//	const int num_elements = v.size(p);
	//	for (int a = v.head(p); a != Limits::INDEX_OVERFLOW; v.next_value(a, p)) {
	//		if (seek_support(c,  v, a, p)) {
	//			v.remove_value(a, p);
	//			//cout << "(" << v.id << ", " << a << ")" << endl;
	//			++ps_.num_delete;
	//		}
	//	}
	//	return num_elements != v.size(p);
	//}

	//inline bool RNSQ::seek_support(const QTab& c, const QVar& x, const QVar& v, const int a, const int p) const {
	//	cout << x.id << " " << v.id << endl;
	//	const int idx = get_QConVal_index(c, v, a);
	//	for (int i = 0; i < x.num_bit; ++i)
	//		if (bitSup_[idx][i] & x.bitDom(p)[i])
	//			return true;
	//	return false;
	//}

	//inline bool RNSQ::seek_support(const QTab& c, const QVar& v, const int a, const int p) const {
	//	const int idx = get_QConVal_index(c, v, a);
	//	for (QVar* y : c.scope)
	//		if (y->id != v.id) {
	//			cout << y->id << " " << v.id << endl;
	//			for (int i = 0; i < y->num_bit; ++i)
	//				if (bitSup_[idx][i] & y->bitDom(p)[i])
	//					return true;
	//		}
	//	return false;
	//}

	inline bool RNSQ::revise(const QTab& c, const QVar& v, const int level) {
		int v_idx;
		QVar* x;
		if (c.scope[0]->id == v.id) {
			v_idx = 0;
			x = c.scope[1];
		}
		else {
			v_idx = 1;
			x = c.scope[0];
		}
		++ss_.num_revisions;
		const int num_elements = v.size(level);

		int a = v.head(level);
		while (a != Limits::INDEX_OVERFLOW) {
			if (!seek_support(*x, c, v_idx, a, level)) {
				v.remove_value(a, level);
				++ps_.num_delete;
			}
			a = v.next(a, level);
		}
		return num_elements != v.size(level);
	}

	inline bool RNSQ::seek_support(const QVar& y, const QTab& c, const int v_idx, const int a, const int p) const {
		const int idx = c.id * max_arity * max_dom_size + v_idx * max_dom_size + a;
		for (int i = 0; i < y.num_bit; ++i)
			if (bitSup_[idx][i] & y.bitDom(p)[i])
				return true;
		return false;
	}

	inline bool RNSQ::seek_support(const QVar& y, const QTab& c, const QVar& v, const int a, const int p) const {
		const int idx = c.id * max_arity * max_dom_size + c.index(v) * max_dom_size + a;
		for (int i = 0; i < y.num_bit; ++i)
			if (bitSup_[idx][i] & y.bitDom(p)[i])
				return true;
		return false;
	}
}
