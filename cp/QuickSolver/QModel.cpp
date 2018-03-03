#include "QModel.h"

namespace cp {
///////////////////////////////////////////////////////////////////////
QVar::QVar(HVar* v) :
	id(v->id),
	capacity(v->vals.size()),
	limit(capacity & MOD_MASK),
	num_bit(ceil(float(capacity) / BITSIZE)),
	vals(v->vals) {
	bit_tmp_.resize(num_bit, ULLONG_MAX);
	if (limit != BITSIZE)
		bit_tmp_.back() >>= BITSIZE - limit;
}

void QVar::runtime(const int size) {
	bit_doms_.resize(size + 3, bit_tmp_);
	//assigned_.resize(size + 3, false);
	size_.resize(size + 3, -1);
	size_[0] = capacity;
	//tmp = size + 1;
}

void QVar::remove_value(const int a, const int p) {
	const auto index = GetBitIdx(a);
	bit_doms_[p][get<0>(index)] &= U64_MASK0[get<1>(index)];
	--size_[p];
}

void QVar::reduce_to(const int a, const int p) {
	const auto index = GetBitIdx(a);
	for (auto& v : bit_doms_[p])
		v = 0;
	bit_doms_[p][get<0>(index)] |= U64_MASK1[get<1>(index)];
	size_[p] = 1;
}

int QVar::size(const int p) const {
	return size_[p];
}

int QVar::next(const int a, const int p) const {
	const auto index = GetBitIdx(a);
	const u64 b = (bit_doms_[p][get<0>(index)] >> get<1>(index)) >> 1;
	if (b)
		return a + FirstOne(b) + 1;

	for (size_t i = get<0>(index) + 1; i < num_bit; ++i)
		if (bit_doms_[p][i])
			return GetValue(i, FirstOne(bit_doms_[p][i]));

	return Limits::INDEX_OVERFLOW;
}

void QVar::next_value(int& a, const int p) {
	auto index = GetBitIdx(a++);
	const u64 b = (bit_doms_[p][get<0>(index)] >> get<1>(index)) >> 1;

	if (b) {
		a = a + FirstOne(b);
		return;
	}

	for (size_t i = get<0>(index) + 1; i < num_bit; ++i)
		if (bit_doms_[p][i]) {
			a = GetValue(i, FirstOne(bit_doms_[p][i]));
			return;
		}

	a = Limits::INDEX_OVERFLOW;
}

bool QVar::have(const int a, const int p) const {
	if (a == Limits::INDEX_OVERFLOW)
		return false;
	const auto index = GetBitIdx(a);
	return bit_doms_[p][get<0>(index)] & U64_MASK1[get<1>(index)];
}

int QVar::head(const int p) const {
	for (size_t i = 0; i < num_bit; ++i)
		if (bit_doms_[p][i])
			return GetValue(i, FirstOne(bit_doms_[p][i]));

	return Limits::INDEX_OVERFLOW;
}

void QVar::show(const int p) {
	cout << "id = " << id << ": ";
	for (auto a : vals)
		if (have(a, p))
			cout << a << " ";
	//cout << "[" << assigned_[p] << "]";
	cout << endl;
}

void QVar::back_to(const int src, const int dest) {
	//for (int i = dest; i < src; ++i)
	//	size_[i] = -1;
	//assigned_[i] = false;
}

void QVar::delete_level(const int p) {
	//assigned_[p] = false;
	size_[p] = -1;
}

void QVar::copy_level(const int src, const int dest) {
	//bit_doms_[dest].assign(bit_doms_[src].begin(), bit_doms_[src].end());
	for (int i = 0; i < num_bit; ++i)
		bit_doms_[dest][i] = bit_doms_[src][i];
	//assigned_[dest] = assigned_[src];
}

void QVar::new_level(const int src, const int dest) {
	//bit_doms_[dest].assign(bit_doms_[src].begin(), bit_doms_[src].end());
	for (int i = 0; i < num_bit; ++i)
		bit_doms_[dest][i] = bit_doms_[src][i];
	//assigned_[dest] = assigned_[src];
	size_[dest] = size_[src];
}
//////////////////////////////////////////////////////////////////////////
//QVar::QVar(HVar* v) :
//	id(v->id),
//	capacity(v->vals.size()),
//	limit(capacity & MOD_MASK),
//	num_bit(ceil(float(capacity) / BITSIZE)),
//	vals(v->vals) {
//	bit_tmp_.resize(num_bit, ULLONG_MAX);
//	if (limit != BITSIZE)
//		bit_tmp_.back() >>= BITSIZE - limit;
//}
//
//void QVar::runtime(const int size) {
//	bit_doms_.resize(size + 3, bit_tmp_);
//	//assigned_.resize(size + 3, false);
//	size_.resize(size + 3, -1);
//	size_[0] = capacity;
//	//tmp = size + 1;
//}
//
//void QVar::remove_value(const int a, const int p) {
//	const auto index = GetBitIdx(a);
//	bit_doms_[p][get<0>(index)] &= U64_MASK0[get<1>(index)];
//	--size_[p];
//}
//
//void QVar::reduce_to(const int a, const int p) {
//	const auto index = GetBitIdx(a);
//	for (auto& v : bit_doms_[p])
//		v = 0;
//	bit_doms_[p][get<0>(index)] |= U64_MASK1[get<1>(index)];
//	size_[p] = 1;
//}
//
//int QVar::size(const int p) const {
//	return size_[p];
//}
//
//int QVar::next(const int a, const int p) const {
//	const auto index = GetBitIdx(a);
//	const u64 b = (bit_doms_[p][get<0>(index)] >> get<1>(index)) >> 1;
//	if (b)
//		return a + FirstOne(b) + 1;
//
//	for (size_t i = get<0>(index) + 1; i < num_bit; ++i)
//		if (bit_doms_[p][i])
//			return GetValue(i, FirstOne(bit_doms_[p][i]));
//
//	return Limits::INDEX_OVERFLOW;
//}
//
//void QVar::next_value(int& a, const int p) {
//	auto index = GetBitIdx(a++);
//	const u64 b = (bit_doms_[p][get<0>(index)] >> get<1>(index)) >> 1;
//
//	if (b) {
//		a = a + FirstOne(b) + 1;
//		return;
//	}
//
//	for (size_t i = get<0>(index) + 1; i < num_bit; ++i)
//		if (bit_doms_[p][i]) {
//			a = GetValue(i, FirstOne(bit_doms_[p][i]));
//			return;
//		}
//
//	a = Limits::INDEX_OVERFLOW;
//}
//
//bool QVar::have(const int a, const int p) const {
//	if (a == Limits::INDEX_OVERFLOW)
//		return false;
//	const auto index = GetBitIdx(a);
//	return bit_doms_[p][get<0>(index)] & U64_MASK1[get<1>(index)];
//}
//
//int QVar::head(const int p) const {
//	for (size_t i = 0; i < num_bit; ++i)
//		if (bit_doms_[p][i])
//			return GetValue(i, FirstOne(bit_doms_[p][i]));
//
//	return Limits::INDEX_OVERFLOW;
//}
//
//void QVar::show(const int p) {
//	cout << "id = " << id << ": ";
//	for (auto a : vals)
//		if (have(a, p))
//			cout << a << " ";
//	//cout << "[" << assigned_[p] << "]";
//	cout << endl;
//}
//
//void QVar::back_to(const int src, const int dest) {
//	//for (int i = dest; i < src; ++i)
//	//	size_[i] = -1;
//	//assigned_[i] = false;
//}
//
//void QVar::delete_level(const int p) {
//	//assigned_[p] = false;
//	size_[p] = -1;
//}
//
//void QVar::copy_level(const int src, const int dest) {
//	//bit_doms_[dest].assign(bit_doms_[src].begin(), bit_doms_[src].end());
//	for (int i = 0; i < num_bit; ++i)
//		bit_doms_[dest][i] = bit_doms_[src][i];
//	//assigned_[dest] = assigned_[src];
//}
//
//void QVar::new_level(const int src, const int dest) {
//	//bit_doms_[dest].assign(bit_doms_[src].begin(), bit_doms_[src].end());
//	for (int i = 0; i < num_bit; ++i)
//		bit_doms_[dest][i] = bit_doms_[src][i];
//	//assigned_[dest] = assigned_[src];
//	size_[dest] = size_[src];
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
void assignments_stack::initial(HModel * m) {
	max_size_ = m->vars.size();
	qvals_.reserve(m->vars.size());
	v_.resize(m->vars.size(), -1);
}

void assignments_stack::push(QVal & v_a) {
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

bool assignments_stack::assigned(const QVar* v) const {
	return v_[v->id] != Limits::INDEX_OVERFLOW;
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
QTab::QTab(HTab * t, vector<QVar*> scope) :
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

void QTab::get_first_valid_tuple(QVar* v, const int a, vector<int>& t, const int p) {
	for (int i = 0; i < arity; ++i)
		if (scope[i] != v)
			t[i] = scope[i]->head(p);
		else
			t[i] = a;
}

void QTab::get_next_valid_tuple(QVar* v, const int a, vector<int>& t, const int p) {
	for (int i = arity - 1; i >= 0; --i) {
		if (scope[i] != v) {
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

int QTab::index(QVar* v) const {
	for (int i = scope.size() - 1; i >= 0; --i)
		if (scope[i] == v)
			return i;
	return Limits::INDEX_OVERFLOW;
}

bool QTab::is_valid(vector<int>& t, const int p) {
	if (!Existed(t))
		return false;

	for (QVar* v : scope)
		if (!v->have(t[index(v)], p))
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