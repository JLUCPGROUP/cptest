#include "QModel.h"

namespace cp {
	///////////////////////////////////////////////////////////////////////
	QVar::QVar(HVar* v) :
		id(v->id),
		capacity(v->vals.size()),
		limit(capacity & MOD_MASK),
		num_bit(ceil(float(capacity) / BITSIZE)),
		vals(v->anti_map),
		size_tmp(v->vals.size()) {
		bit_tmp_ = new u64[num_bit];
		memset(bit_tmp_, ULLONG_MAX, num_bit * sizeof(u64));
		bit_tmp_[num_bit - 1] >>= BITSIZE - limit;
	}

	QVar::~QVar() {
		delete[] bit_tmp_;
		if (backtrackable_)
			disable_backtracking();
	}

	void QVar::disable_backtracking() {
		backtrackable_ = false;
		for (int i = 0; i < num_bd_; ++i)
			delete[] bit_doms_[i];
		delete[] bit_doms_;
		delete[] size_;
	}

	void QVar::enable_backtracking(const int size) {
		backtrackable_ = true;
		num_bd_ = size + 3;
		bit_doms_ = new u64*[num_bd_]();
		for (int i = 0; i < num_bd_; ++i)
			bit_doms_[i] = new u64[num_bit]();

		size_ = new int[num_bd_];
		memset(size_, -1, num_bd_ * sizeof(int));
		size_[0] = size_tmp;

		for (int i = 0; i < num_bit; ++i)
			bit_doms_[0][i] = bit_tmp_[i];
	}

	void QVar::remove_value(const int a, const int p) const {
		const auto index = GetBitIdx(a);
		bit_doms_[p][index.x] &= U64_MASK0[index.y];
		--size_[p];
	}

	void QVar::reduce_to(const int a, const int p) const {
		const auto index = GetBitIdx(a);
		memset(bit_doms_[p], 0, num_bit * sizeof(u64));
		bit_doms_[p][index.x] |= U64_MASK1[index.y];
		size_[p] = 1;
	}

	int QVar::size(const int p) const {
		return size_[p];
	}

	//inline int FirstOne2(const u64 UseMask) {
	//	u64 index = UseMask;
	//	return int(index);
	//}

	int QVar::next(const int a, const int p) const {
		const auto index = GetBitIdx(a);
		if (index.y < 63) {
			if (bit_doms_[p][index.x] >= U64_MASK1[index.y + 1])
				for (int b = index.y + 1; b < BITSIZE; ++b)
					if (bit_doms_[p][index.x] & U64_MASK1[b])
						return GetValue(index.x, b);
		}

		for (int i = index.x + 1; i < num_bit; ++i)
			if (bit_doms_[p][i])
				return GetValue(i, FirstOne(bit_doms_[p][i]));

		return Limits::INDEX_OVERFLOW;
	}

	void QVar::next_value(int& a, const int p) const {
		const auto index = GetBitIdx(a);
		if (index.y < 63) {
			if (bit_doms_[p][index.x] >= U64_MASK1[index.y + 1]) {
				for (int b = index.y + 1; b < BITSIZE; ++b) {
					if (bit_doms_[p][index.x] & U64_MASK1[b]) {
						a = GetValue(index.x, b);
						return;
					}
				}
			}
		}

		for (int i = index.x + 1; i < num_bit; ++i) {
			if (bit_doms_[p][i]) {
				const int b = FirstOne(bit_doms_[p][i]);
				a = GetValue(i, b);
				return;
			}
		}
		a = Limits::INDEX_OVERFLOW;
	}

	//int QVar::next(const int a, const int p) const {
	//	const auto index = GetBitIdx(a);
	//	const u64 b = (bit_doms_[p][index.x] >> index.y) >> 1;
	//	if (b)
	//		return a + FirstOne(b) + 1;

	//	for (int i = index.x + 1; i < num_bit; ++i)
	//		if (bit_doms_[p][i])
	//			return GetValue(i, FirstOne(bit_doms_[p][i]));

	//	return Limits::INDEX_OVERFLOW;
	//}

	//void QVar::next_value(int& a, const int p) const {
	//	const auto index = GetBitIdx(a);
	//	const u64 b = (bit_doms_[p][index.x] >> index.y) >> 1;

	//	if (b) {
	//		a = a + 1 + FirstOne(b);
	//		return;
	//	}

	//	for (int i = index.x + 1; i < num_bit; ++i)
	//		if (bit_doms_[p][i]) {
	//			a = GetValue(i, FirstOne(bit_doms_[p][i]));
	//			return;
	//		}

	//	a = Limits::INDEX_OVERFLOW;
	//}

	bool QVar::have(const int a, const int p) const {
		if (a == Limits::INDEX_OVERFLOW)
			return false;
		const auto index = GetBitIdx(a);
		return bit_doms_[p][index.x] & U64_MASK1[index.y];
	}

	int QVar::head(const int p) const {
		for (size_t i = 0; i < num_bit; ++i)
			if (bit_doms_[p][i])
				return GetValue(i, FirstOne(bit_doms_[p][i]));

		return Limits::INDEX_OVERFLOW;
	}

	void QVar::show(const int p) const {
		cout << "id = " << id << ": ";
		for (int a = 0; a < capacity; ++a)
			if (have(a, p))
				cout << vals[a] << " ";
		cout << endl;
	}

	//void QVar::back_to(const int src, const int dest) {
	//	//for (int i = dest; i < src; ++i)
	//	//	size_[i] = -1;
	//	//assigned_[i] = false;
	//}

	void QVar::delete_level(const int p) const {
		size_[p] = -1;
	}

	void QVar::copy_level(const int src, const int dest) const {
		for (int i = 0; i < num_bit; ++i)
			bit_doms_[dest][i] = bit_doms_[src][i];
		size_[dest] = size_[src];
	}

	void QVar::new_level(const int src, const int dest) const {
		for (int i = 0; i < num_bit; ++i)
			bit_doms_[dest][i] = bit_doms_[src][i];
		size_[dest] = size_[src];
	}

	//int QVar::max_value(const int p)
	//{
	//}

	//////////////////////////////////////////////////////////////////////////////
	const QVal& QVal::operator=(const QVal& rhs) {
		v = rhs.v;
		a = rhs.a;
		aop = rhs.aop;
		return *this;
	}

	void QVal::flip() {
		aop = !aop;
	}

	QVal QVal::next() const {
		return QVal(v, a + 1, true);
	}

	bool QVal::is_null_node() const {
		return a == Limits::INDEX_OVERFLOW;
	}

	bool QVal::operator==(const QVal & rhs) {
		return (this == &rhs) || (v == rhs.v && a == rhs.a && aop == rhs.aop);
	}

	bool QVal::operator!=(const QVal & rhs) {
		return !((this == &rhs) || (v == rhs.v && a == rhs.a && aop == rhs.aop));
	}

	ostream & operator<<(ostream & os, QVal & v_val) {
		const string s = (v_val.aop) ? " = " : " != ";
		os << "(" << v_val.v->id << s << v_val.a << ")";
		return os;
	}
	///////////////////////////////////////////////////////////////////////
	void assignments_stack::initial(const HModel& m) {
		max_size_ = m.vars.size();
		//qvals_ = new QVal[m->vars.size()];
		//v_=new int[m->vars.size()];
		qvals_.reserve(m.vars.size());
		v_.resize(m.vars.size(), -1);
	}

	void assignments_stack::push(const QVal & v_a) {
		qvals_.push_back(v_a);
		v_[v_a.v->id] = v_a.a;
	}

	QVal assignments_stack::pop() {
		auto val = qvals_.back();
		qvals_.pop_back();
		v_[val.v->id] = Limits::INDEX_OVERFLOW;
		return val;
	}

	QVal assignments_stack::top() const { return qvals_.back(); };
	int assignments_stack::size() const { return qvals_.size(); }
	int assignments_stack::capacity() const { return max_size_; }
	bool assignments_stack::full() const { return qvals_.size() == max_size_; }
	bool assignments_stack::empty() const { return  qvals_.empty(); }
	QVal assignments_stack::operator[](const int i) const { return qvals_[i]; };
	QVal assignments_stack::at(const int i) const { return qvals_[i]; }

	void assignments_stack::clear() {
		qvals_.clear();
		v_.assign(v_.size(), Limits::INDEX_OVERFLOW);
	};

	bool assignments_stack::assigned(const int v) const {
		return v_[v] != Limits::INDEX_OVERFLOW;
	}

	//bool assignments_stack::assigned(const QVar* v) const {
	//	return v_[v->id] != Limits::INDEX_OVERFLOW;
	//}

	bool assignments_stack::assigned(const QVar& v) const {
		return v_[v.id] != Limits::INDEX_OVERFLOW;
	}

	bool assignments_stack::solution(vector<int>& sol) const {
		sol.clear();
		if (!full())
			return false;
		sol = v_;
		return true;
	}

	ostream & operator<<(ostream & os, assignments_stack& I) {
		for (int i = 0; i < I.size(); ++i)
			os << I.v_[i] << " ";
		return os;
	}

	ostream & operator<<(ostream & os, assignments_stack* I) {
		for (int i = 0; i < I->size(); ++i)
			os << I->v_[i] << " ";
		return os;
	}
	///////////////////////////////////////////////////////////////////////
	bool vars_cir_que::empty() const {
		return m_front_ == m_rear_;
	}

	void vars_cir_que::initial(const int size) {
		max_size_ = size + 1;
		m_data_.resize(max_size_, nullptr);
		vid_set_.resize(max_size_, false);
		m_front_ = 0;
		m_rear_ = 0;
		size_ = 0;
	}

	bool vars_cir_que::full() const {
		return m_front_ == (m_rear_ + 1) % max_size_;
	}

	void vars_cir_que::push(QVar* v) {
		if (vid_set_[v->id])
			return;
		m_data_[m_rear_] = v;
		m_rear_ = (m_rear_ + 1) % max_size_;
		vid_set_[v->id] = true;
		++size_;
	}

	QVar* vars_cir_que::pop() {
		QVar* tmp = m_data_[m_front_];
		m_front_ = (m_front_ + 1) % max_size_;
		vid_set_[tmp->id] = false;
		--size_;
		return tmp;
	}

	void vars_cir_que::clear() {
		m_front_ = 0;
		m_rear_ = 0;
		size_ = 0;
		vid_set_.assign(vid_set_.size(), false);
	}

	int vars_cir_que::max_size() const {
		return max_size_;
	}

	int vars_cir_que::size() const {
		return size_;
	}
	///////////////////////////////////////////////////////////////////////
	bool vars_pair_cir_que::empty() const {
		return m_front_ == m_rear_;
	}

	void vars_pair_cir_que::initial(const int size) {
		max_size_ = size * size + 1;
		m_data_.resize(max_size_);
		id_set_.resize(size, vector<int>(size, false));
		m_front_ = 0;
		m_rear_ = 0;
		size_ = 0;
		num_vars_ = size;
	}

	bool vars_pair_cir_que::full() const {
		return m_front_ == (m_rear_ + 1) % max_size_;
	}

	void vars_pair_cir_que::push(const variable_pair vv) {
		if (id_set_[vv.x->id][vv.y->id])
			return;
		m_data_[m_rear_] = vv;
		m_rear_ = (m_rear_ + 1) % max_size_;
		id_set_[vv.x->id][vv.y->id] = true;
		++size_;
	}

	variable_pair vars_pair_cir_que::pop() {
		const variable_pair tmp = m_data_[m_front_];
		m_front_ = (m_front_ + 1) % max_size_;
		id_set_[tmp.x->id][tmp.y->id] = false;
		--size_;
		return tmp;
	}

	void vars_pair_cir_que::clear() {
		m_front_ = 0;
		m_rear_ = 0;
		size_ = 0;
		for (auto& v : id_set_) {
			v.assign(num_vars_, false);
		}
	}

	int vars_pair_cir_que::max_size() const {
		return max_size_;
	}

	int vars_pair_cir_que::size() const {
		return size_;
	}

	///////////////////////////////////////////////////////////////////////
	vars_heap::~vars_heap() {
		del();
	}

	void vars_heap::push(QVar& v, const int p) {
		insert(v, p);
	}

	QVar* vars_heap::pop(const int p) {
		return remove_at(0, p);
	}

	void vars_heap::initial(const int size) {
		max_size_ = size;
		vs_ = new QVar*[size];
		position_ = new int[size];
		memset(position_, -1, size * sizeof(int));
	}

	void vars_heap::del() const {
		delete[] vs_;
		delete[] position_;
	}

	void vars_heap::insert(QVar& v, const int p) {
		if (position_[v.id] >= 0)
			filter_up(position_[v.id], p);
		else {
			vs_[cur_size_] = &v;
			filter_up(cur_size_, p);
			cur_size_++;
		}
	}

	QVar* vars_heap::remove_at(const int location, const int p) {
		QVar* v = vs_[location];
		vs_[location] = vs_[cur_size_ - 1];
		cur_size_--;
		filter_down(location, cur_size_ - 1, p);
		position_[v->id] = -1;
		return v;
	}

	void vars_heap::clear() {
		memset(position_, -1, max_size_ * sizeof(int));
		cur_size_ = 0;
	}

	bool vars_heap::compare(const QVar& a, const QVar& b, const int p) {
		return a.size(p) <= b.size(p);
	}

	void vars_heap::filter_up(const int start, const int p) const {
		int j = start;
		int i = (j - 1) / 2;
		QVar* v = vs_[j];
		while (j > 0) {
			if (compare(*(vs_[i]), *v, p)) {
				break;
			}
			else {
				vs_[j] = vs_[i];
				position_[vs_[j]->id] = j;
				j = i;
				i = (i - 1) / 2;
			}
		}
		vs_[j] = v;
		position_[vs_[j]->id] = j;
	}

	void vars_heap::filter_down(const int start, const int finish, const int p) const {
		int i = start;
		int j = 2 * i + 1;
		QVar* v = vs_[i];

		while (j <= finish) {
			if (j < finish && compare(*(vs_[j + 1]), *(vs_[j]), p))
				j++;

			if (compare(*v, *(vs_[j]), p))
				break;
			else {
				vs_[i] = vs_[j];
				position_[vs_[i]->id] = i;
				i = j;
				j = 2 * j + 1;
			}
		}

		vs_[i] = v;
		position_[vs_[i]->id] = i;
	}
	///////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////
	QTab::QTab(HTab * t, vector<QVar*>& scope) :
		id(t->id),
		arity(scope.size()),
		scope(scope),
		tuples(t->tuples) {

	}

	bool QTab::sat(vector<int>& t) const {
		return binary_search(tuples.begin(), tuples.end(), t);
	}

	void QTab::get_first_valid_tuple(QVal & v_a, vector<int>& t, const int p) {
		for (int i = 0; i < arity; ++i)
			if (scope[i] != v_a.v)
				t[i] = scope[i]->head(p);
			else
				t[i] = v_a.a;
	}

	void QTab::get_next_valid_tuple(QVal& v_a, vector<int>& t, const int p) {
		for (int i = arity - 1; i >= 0; --i) {
			if (scope[i] != v_a.v) {
				if (scope[i]->next(t[i], p) == Limits::INDEX_OVERFLOW) {
					t[i] = scope[i]->head(p);
				}
				else {
					t[i] = scope[i]->next(t[i], p);
					return;
				}
			}
		}
		Exclude(t);
	}

	void QTab::get_first_valid_tuple(const QVar& v, const int a, vector<int>& t, const int p) const {
		for (int i = 0; i < arity; ++i)
			if (scope[i]->id != v.id)
				t[i] = scope[i]->head(p);
			else
				t[i] = a;
	}

	void QTab::get_next_valid_tuple(const QVar& v, const int a, vector<int>& t, const int p) const {
		for (int i = arity - 1; i >= 0; --i) {
			if (scope[i]->id != v.id) {
				if (scope[i]->next(t[i], p) == Limits::INDEX_OVERFLOW) {
					t[i] = scope[i]->head(p);
				}
				else {
					t[i] = scope[i]->next(t[i], p);
					return;
				}
			}
		}
		Exclude(t);
	}

	//int QTab::index(QVar* v) const {
	//	for (int i = scope.size() - 1; i >= 0; --i)
	//		if (scope[i] == v)
	//			return i;
	//	return Limits::INDEX_OVERFLOW;
	//}

	int QTab::index(const QVar & v) const {
		for (int i = scope.size() - 1; i >= 0; --i)
			if (scope[i]->id == v.id)
				return i;
		cout << "get var index error!!" << endl;
		return Limits::INDEX_OVERFLOW;
	}

	bool QTab::is_valid(vector<int>& t, const int p) const {
		if (!Existed(t))
			return false;

		for (QVar* v : scope)
			if (!v->have(t[index(*v)], p))
				return false;
		return true;
	}

	const QConVal& QConVal::operator=(const QConVal& rhs) {
		c = rhs.c;
		v = rhs.v;
		a = rhs.a;
		return *this;
	}




}
