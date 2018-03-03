#pragma once
#include <climits>
#include <vector>
#include "XBuilder.h"

namespace cp {
typedef unsigned long long u64;
typedef unsigned u32;

const u32 U32_UMASK1[32] = {
	0x80000000, 0x40000000, 0x20000000, 0x10000000,
	0x08000000, 0x04000000, 0x02000000, 0x01000000,
	0x00800000, 0x00400000, 0x00200000, 0x00100000,
	0x00080000, 0x00040000, 0x00020000, 0x00010000,
	0x00008000, 0x00004000, 0x00002000, 0x00001000,
	0x00000800, 0x00000400, 0x00000200, 0x00000100,
	0x00000080, 0x00000040, 0x00000020, 0x00000010,
	0x00000008, 0x00000004, 0x00000002, 0x00000001
};

const u32 U32_UMASK0[32] = {
	0x7FFFFFFF, 0xBFFFFFFF, 0xDFFFFFFF, 0xEFFFFFFF,
	0xF7FFFFFF, 0xFBFFFFFF, 0xFDFFFFFF, 0xFEFFFFFF,
	0xFF7FFFFF, 0xFFBFFFFF, 0xFFDFFFFF, 0xFFEFFFFF,
	0xFFF7FFFF, 0xFFFBFFFF, 0xFFFDFFFF, 0xFFFEFFFF,
	0xFFFF7FFF, 0xFFFFBFFF, 0xFFFFDFFF, 0xFFFFEFFF,
	0xFFFFF7FF, 0xFFFFFBFF, 0xFFFFFDFF, 0xFFFFFEFF,
	0xFFFFFF7F, 0xFFFFFFBF, 0xFFFFFFDF, 0xFFFFFFEF,
	0xFFFFFFF7, 0xFFFFFFFB, 0xFFFFFFFD, 0xFFFFFFFE
};

//const u64 U64_MASK1[64] = {
//	0x8000000000000000, 0x4000000000000000, 0x2000000000000000, 0x1000000000000000,
//	0x0800000000000000, 0x0400000000000000, 0x0200000000000000, 0x0100000000000000,
//	0x0080000000000000, 0x0040000000000000, 0x0020000000000000, 0x0010000000000000,
//	0x0008000000000000, 0x0004000000000000, 0x0002000000000000, 0x0001000000000000,
//	0x0000800000000000, 0x0000400000000000, 0x0000200000000000, 0x0000100000000000,
//	0x0000080000000000, 0x0000040000000000, 0x0000020000000000, 0x0000010000000000,
//	0x0000008000000000, 0x0000004000000000, 0x0000002000000000, 0x0000001000000000,
//	0x0000000800000000, 0x0000000400000000, 0x0000000200000000, 0x0000000100000000,
//	0x0000000080000000, 0x0000000040000000, 0x0000000020000000, 0x0000000010000000,
//	0x0000000008000000, 0x0000000004000000, 0x0000000002000000, 0x0000000001000000,
//	0x0000000000800000, 0x0000000000400000, 0x0000000000200000, 0x0000000000100000,
//	0x0000000000080000, 0x0000000000040000, 0x0000000000020000, 0x0000000000010000,
//	0x0000000000008000, 0x0000000000004000, 0x0000000000002000, 0x0000000000001000,
//	0x0000000000000800, 0x0000000000000400, 0x0000000000000200, 0x0000000000000100,
//	0x0000000000000080, 0x0000000000000040, 0x0000000000000020, 0x0000000000000010,
//	0x0000000000000008, 0x0000000000000004, 0x0000000000000002, 0x0000000000000001,
//};
//
//const u64 U64_MASK0[64] = {
//	0x7FFFFFFFFFFFFFFF, 0xBFFFFFFFFFFFFFFF, 0xDFFFFFFFFFFFFFFF, 0xEFFFFFFFFFFFFFFF,
//	0xF7FFFFFFFFFFFFFF, 0xFBFFFFFFFFFFFFFF, 0xFDFFFFFFFFFFFFFF, 0xFEFFFFFFFFFFFFFF,
//	0xFF7FFFFFFFFFFFFF, 0xFFBFFFFFFFFFFFFF, 0xFFDFFFFFFFFFFFFF, 0xFFEFFFFFFFFFFFFF,
//	0xFFF7FFFFFFFFFFFF, 0xFFFBFFFFFFFFFFFF, 0xFFFDFFFFFFFFFFFF, 0xFFFEFFFFFFFFFFFF,
//	0xFFFF7FFFFFFFFFFF, 0xFFFFBFFFFFFFFFFF, 0xFFFFDFFFFFFFFFFF, 0xFFFFEFFFFFFFFFFF,
//	0xFFFFF7FFFFFFFFFF, 0xFFFFFBFFFFFFFFFF, 0xFFFFFDFFFFFFFFFF, 0xFFFFFEFFFFFFFFFF,
//	0xFFFFFF7FFFFFFFFF, 0xFFFFFFBFFFFFFFFF, 0xFFFFFFDFFFFFFFFF, 0xFFFFFFEFFFFFFFFF,
//	0xFFFFFFF7FFFFFFFF, 0xFFFFFFFBFFFFFFFF, 0xFFFFFFFDFFFFFFFF, 0xFFFFFFFEFFFFFFFF,
//	0xFFFFFFFF7FFFFFFF, 0xFFFFFFFFBFFFFFFF, 0xFFFFFFFFDFFFFFFF, 0xFFFFFFFFEFFFFFFF,
//	0xFFFFFFFFF7FFFFFF, 0xFFFFFFFFFBFFFFFF, 0xFFFFFFFFFDFFFFFF, 0xFFFFFFFFFEFFFFFF,
//	0xFFFFFFFFFF7FFFFF, 0xFFFFFFFFFFBFFFFF, 0xFFFFFFFFFFDFFFFF, 0xFFFFFFFFFFEFFFFF,
//	0xFFFFFFFFFFF7FFFF, 0xFFFFFFFFFFFBFFFF, 0xFFFFFFFFFFFDFFFF, 0xFFFFFFFFFFFEFFFF,
//	0xFFFFFFFFFFFF7FFF, 0xFFFFFFFFFFFFBFFF, 0xFFFFFFFFFFFFDFFF, 0xFFFFFFFFFFFFEFFF,
//	0xFFFFFFFFFFFFF7FF, 0xFFFFFFFFFFFFFBFF, 0xFFFFFFFFFFFFFDFF, 0xFFFFFFFFFFFFFEFF,
//	0xFFFFFFFFFFFFFF7F, 0xFFFFFFFFFFFFFFBF, 0xFFFFFFFFFFFFFFDF, 0xFFFFFFFFFFFFFFEF,
//	0xFFFFFFFFFFFFFFF7, 0xFFFFFFFFFFFFFFFB, 0xFFFFFFFFFFFFFFFD, 0xFFFFFFFFFFFFFFFE,
//};
const u64 U64_MASK1[64] = {
	0x0000000000000001,0x0000000000000002,0x0000000000000004,0x0000000000000008,
	0x0000000000000010,0x0000000000000020,0x0000000000000040,0x0000000000000080,
	0x0000000000000100,0x0000000000000200,0x0000000000000400,0x0000000000000800,
	0x0000000000001000,0x0000000000002000,0x0000000000004000,0x0000000000008000,
	0x0000000000010000,0x0000000000020000,0x0000000000040000,0x0000000000080000,
	0x0000000000100000,0x0000000000200000,0x0000000000400000,0x0000000000800000,
	0x0000000001000000,0x0000000002000000,0x0000000004000000,0x0000000008000000,
	0x0000000010000000,0x0000000020000000,0x0000000040000000,0x0000000080000000,
	0x0000000100000000,0x0000000200000000,0x0000000400000000,0x0000000800000000,
	0x0000001000000000,0x0000002000000000,0x0000004000000000,0x0000008000000000,
	0x0000010000000000,0x0000020000000000,0x0000040000000000,0x0000080000000000,
	0x0000100000000000,0x0000200000000000,0x0000400000000000,0x0000800000000000,
	0x0001000000000000,0x0002000000000000,0x0004000000000000,0x0008000000000000,
	0x0010000000000000,0x0020000000000000,0x0040000000000000,0x0080000000000000,
	0x0100000000000000,0x0200000000000000,0x0400000000000000,0x0800000000000000,
	0x1000000000000000,0x2000000000000000,0x4000000000000000,0x8000000000000000,
};

const u64 U64_MASK0[64] = {
	0xFFFFFFFFFFFFFFFE,0xFFFFFFFFFFFFFFFD,0xFFFFFFFFFFFFFFFB,0xFFFFFFFFFFFFFFF7,
	0xFFFFFFFFFFFFFFEF,0xFFFFFFFFFFFFFFDF,0xFFFFFFFFFFFFFFBF,0xFFFFFFFFFFFFFF7F,
	0xFFFFFFFFFFFFFEFF,0xFFFFFFFFFFFFFDFF,0xFFFFFFFFFFFFFBFF,0xFFFFFFFFFFFFF7FF,
	0xFFFFFFFFFFFFEFFF,0xFFFFFFFFFFFFDFFF,0xFFFFFFFFFFFFBFFF,0xFFFFFFFFFFFF7FFF,
	0xFFFFFFFFFFFEFFFF,0xFFFFFFFFFFFDFFFF,0xFFFFFFFFFFFBFFFF,0xFFFFFFFFFFF7FFFF,
	0xFFFFFFFFFFEFFFFF,0xFFFFFFFFFFDFFFFF,0xFFFFFFFFFFBFFFFF,0xFFFFFFFFFF7FFFFF,
	0xFFFFFFFFFEFFFFFF,0xFFFFFFFFFDFFFFFF,0xFFFFFFFFFBFFFFFF,0xFFFFFFFFF7FFFFFF,
	0xFFFFFFFFEFFFFFFF,0xFFFFFFFFDFFFFFFF,0xFFFFFFFFBFFFFFFF,0xFFFFFFFF7FFFFFFF,
	0xFFFFFFFEFFFFFFFF,0xFFFFFFFDFFFFFFFF,0xFFFFFFFBFFFFFFFF,0xFFFFFFF7FFFFFFFF,
	0xFFFFFFEFFFFFFFFF,0xFFFFFFDFFFFFFFFF,0xFFFFFFBFFFFFFFFF,0xFFFFFF7FFFFFFFFF,
	0xFFFFFEFFFFFFFFFF,0xFFFFFDFFFFFFFFFF,0xFFFFFBFFFFFFFFFF,0xFFFFF7FFFFFFFFFF,
	0xFFFFEFFFFFFFFFFF,0xFFFFDFFFFFFFFFFF,0xFFFFBFFFFFFFFFFF,0xFFFF7FFFFFFFFFFF,
	0xFFFEFFFFFFFFFFFF,0xFFFDFFFFFFFFFFFF,0xFFFBFFFFFFFFFFFF,0xFFF7FFFFFFFFFFFF,
	0xFFEFFFFFFFFFFFFF,0xFFDFFFFFFFFFFFFF,0xFFBFFFFFFFFFFFFF,0xFF7FFFFFFFFFFFFF,
	0xFEFFFFFFFFFFFFFF,0xFDFFFFFFFFFFFFFF,0xFBFFFFFFFFFFFFFF,0xF7FFFFFFFFFFFFFF,
	0xEFFFFFFFFFFFFFFF,0xDFFFFFFFFFFFFFFF,0xBFFFFFFFFFFFFFFF,0x7FFFFFFFFFFFFFFF,
};

namespace Limits {
/**
* \brief 取值范围
*/
const int MIN_INTVAR_ID = 0x7fff7000;
const int MAX_INTVAR_ID = INT_MAX - 1;
const int MAX_OPT = INT_MIN & 0xffff7000 - 1;
const int MIN_OPT = INT_MIN + 1;
const int UNSIGNED_VAL = INT_MIN & 0xffff7000;
const int MIN_VAL = UNSIGNED_VAL + 1;
const int MAX_VAL = MIN_INTVAR_ID - 1;
const int INDEX_OVERFLOW = -1;
const int PRESENT = -1;
const int ABSENT = 0;
}

//64位
const int BITSIZE = 64;
const int DIV_BIT = 6;
const int MOD_MASK = 0x3f;
//32位
//const int BITSIZE = 32;
//const int DIV_BIT = 5;
//const int MOD_MASK = 0x1f;

namespace Heuristic {
enum Var {
	VRH_LEX,
	VRH_DOM_MIN,
	VRH_VWDEG,
	VRH_DOM_DEG_MIN,
	VRH_DOM_WDEG_MIN
};

enum Val {
	VLH_MIN,
	VLH_MIN_DOM,
	VLH_MIN_INC,
	VLH_MAX_INC,
	VLH_VWDEG
};

enum DecisionScheme {
	DS_BI,
	DS_NB
};
};

inline bool Existed(vector<int>& tuple) {
	return tuple[0] != INT_MAX;
}

inline void Exclude(vector<int>& tuple) {
	tuple[0] = INT_MAX;
}

inline tuple<int, int> GetBitIdx(const int idx) {
	tuple<int, int> a;
	get<0>(a) = idx >> DIV_BIT;
	get<1>(a) = idx & MOD_MASK;
	return a;
}
inline int GetValue(const int i, const int j) {
	return  (i << DIV_BIT) + j;
}

typedef vector<u64> bitSetVector;
inline int FirstOne(const u32 UseMask) {
	register u32 index = UseMask;
	//将第一个为1位的低位都置1，其它位都置0
	index = (index - 1)  &  ~index;
	//得到有多少为1的位
	index = index & 0x55555555 + (index >> 1) & 0x55555555;
	index = index & 0x33333333 + (index >> 2) & 0x33333333;
	index = index & 0x0F0F0F0F + (index >> 4) & 0x0F0F0F0F;
	index = index & 0xFF + (index & 0xFF00 >> 8) + (index & 0xFF0000 >> 16) + (index & 0xFF000000 >> 24);
	//得到位数,如果为32则表示全0
	return index;
}

inline int FirstOne(const u64 UseMask) {
	u64 index = UseMask;
	//将第一个为1位的低位都置1，其它位都置0
	index = (index - 1)  &  ~index;
	//得到有多少为1的位
	index = (index & 0x5555555555555555) + ((index >> 1) & 0x5555555555555555);
	index = (index & 0x3333333333333333) + ((index >> 2) & 0x3333333333333333);
	index = (index & 0x0F0F0F0F0F0F0F0F) + ((index >> 4) & 0x0F0F0F0F0F0F0F0F);
	index = (index & 0x00FF00FF00FF00FF) + ((index >> 8) & 0x00FF00FF00FF00FF);
	index = (index & 0x0000ffff0000ffff) + ((index >> 16) & 0x0000ffff0000ffff);
	index = (index & 0xFFFFFFFF) + ((index & 0xFFFFFFFF00000000) >> 32);
	//得到位数,如果为32则表示全0
	return int(index);
}

class QVar {
public:
	QVar(HVar* v);
	void runtime(const int size);
	void remove_value(const int a, const int p);
	void reduce_to(const int a, const int p);
	int size(const int p) const;
	inline int next(const int a, const int p) const;
	void next_value(int& a, const int p);
	//int prev(const int a, const int p) const;
	inline bool have(const int a, const int p) const;
	inline int head(const int p) const;
	//inline bool assigned(const int p) const { return assigned_[p]; }
	//inline void assign(const bool a, const int p) { assigned_[p] = a; }
	//int tail(const int p) const;
	bool faild(const int p) const { return size(p) == 0; };
	bitSetVector& bitDom(const int p) { return bit_doms_[p]; }
	void show(const int p);
	inline void back_to(const int src, const int dest);
	void delete_level(const int p);
	void copy_level(const int src, const int dest);
	//inline int new_level(int src);
	void new_level(const int src, const int dest);

	const int id;
	const int capacity;
	const int limit;
	const int num_bit;
	const vector<int> vals;
	//static int tmp;
private:
	//int top_;
	//vector<bool> assigned_;
	vector<int> size_;
	bitSetVector bit_tmp_;
	vector<bitSetVector> bit_doms_;
};
//class QVar {
//public:
//	QVar(HVar* v);
//	void runtime(const int size);
//	void remove_value(const int a, const int p);
//	void reduce_to(const int a, const int p);
//	int size(const int p) const;
//	inline int next(const int a, const int p) const;
//	inline void next_value(int& a, const int p);
//	//int prev(const int a, const int p) const;
//	inline bool have(const int a, const int p) const;
//	inline int head(const int p) const;
//	//inline bool assigned(const int p) const { return assigned_[p]; }
//	//inline void assign(const bool a, const int p) { assigned_[p] = a; }
//	//int tail(const int p) const;
//	bool faild(const int p) const { return size(p) == 0; };
//	bitSetVector& bitDom(const int p) { return bit_doms_[p]; }
//	void show(const int p);
//	inline void back_to(const int src, const int dest);
//	void delete_level(const int p);
//	void copy_level(const int src, const int dest);
//	//inline int new_level(int src);
//	void new_level(const int src, const int dest);
//
//	const int id;
//	const int capacity;
//	const int limit;
//	const int num_bit;
//	const vector<int> vals;
//	//static int tmp;
//private:
//	//int top_;
//	//vector<bool> assigned_;
//	int* size_;
//	u64* bit_tmp_;
//	u64** bit_doms_;
//};

class QVal {
public:
	QVar* v;
	int a;
	bool aop = true;
	QVal() : v(nullptr), a(-2) {};
	QVal(QVar* v, const int a, const bool aop = true) :v(v), a(a), aop(aop) {};

	const QVal& operator=(const QVal& rhs);
	void flip();
	QVal next() const;
	bool operator==(const QVal& rhs);
	bool operator!=(const QVal& rhs);
	friend std::ostream& operator<< (std::ostream &os, QVal &v_val);
	~QVal() {};
};

namespace SearchNode {
const QVal RootNode = QVal(nullptr, 0);
const QVal Deadend = QVal(nullptr, -1);
const QVal NullNode = QVal(nullptr, -2);
const QVal OutLastBroNode = QVal(nullptr, -3);
const QVal OutLastNode = QVal(nullptr, -4);
};

class assignments_stack {
public:
	assignments_stack() {};
	void initial(HModel* m);
	~assignments_stack() {};
	void push(QVal& v_a);
	QVal pop();
	QVal top() const;
	int size() const;
	int capacity() const;
	bool full() const;
	bool empty() const;
	QVal operator[](const int i) const;
	QVal at(const int i) const;
	void clear();
	bool assigned(const int v) const;
	bool assigned(const QVar* v) const;
	bool solution(vector<int>& sol) const;
	friend ostream& operator<< (std::ostream &os, assignments_stack &I);
	friend ostream& operator<< (std::ostream &os, assignments_stack* I);
protected:
	vector<int> v_;
	vector<QVal> qvals_;
	int max_size_;
};

class vars_cir_que {
public:
	vars_cir_que() {}
	virtual ~vars_cir_que() {};

	bool empty() const;
	void initial(const int size);
	bool full() const;
	void push(QVar* v);
	QVar* pop();
	void clear();
	int max_size() const;
	int size() const;

private:
	vector<QVar*> m_data_;
	vector<bool> vid_set_;
	size_t max_size_;
	int m_front_;
	int m_rear_;
	int size_;
};

class QTab {
public:
	QTab(HTab* t, vector<QVar*> scope);
	bool sat(vector<int> &t) const;
	void get_first_valid_tuple(QVal& v_a, vector<int>& t, const int p);
	void get_next_valid_tuple(QVal& v_a, vector<int>& t, const int p);
	void get_first_valid_tuple(QVar* v, const int a, vector<int>& t, const int p);
	void get_next_valid_tuple(QVar* v, const int a, vector<int>& t, const int p);
	int index(QVar* v) const;
	bool is_valid(vector<int>& t, const int p);
	const int id;
	const int arity;
	vector<QVar*> scope;
	vector<vector<int>> tuples;
	~QTab() {};
};

class arc {
public:
	QTab* c;
	QVar* v;
	arc() :c(nullptr), v(nullptr) {};
	arc(QTab* c, QVar* v) :c(c), v(v) {}
	virtual ~arc() {}
};

class QConVal {
public:
	QConVal() :c(nullptr), v(nullptr), a(Limits::INDEX_OVERFLOW) {}
	QConVal(QTab* c, QVar *v, const  int a) : c(c), v(v), a(a) {}
	QConVal(QTab* c, QVal& va) :c(c), v(va.v), a(va.a) {}
	QConVal(arc& rc, const int a) :c(rc.c), v(rc.v), a(a) {}

	virtual ~QConVal() {}
	arc get_arc() const { return arc(c, v); }
	QVal get_vvalue() const { return QVal(v, a); }
	int get_var_index()const { return c->index(v); }
	const QConVal& operator=(const QConVal& rhs);

	friend std::ostream& operator<< (std::ostream &os, QConVal &cval) {
		os << "(" << cval.c->id << ", " << cval.v->id << ", " << cval.a << ")";
		return os;
	}

	QTab* c;
	QVar* v;
	int a;
};

struct PropagationState {
	//是否执行失败
	bool state;
	bool seek_support_fail = true;
	bool revise_fail = true;
	//删除变量值数
	int num_delete = 0;
	//造成失败的约束
	QTab* tab = nullptr;
	//造成失败的变量
	QVar* var = nullptr;
	QVal v_a_fail;
	int level = 0;
};

struct SearchStatistics {
	uint64_t num_sol = 0;
	uint64_t num_positive = 0;
	uint64_t num_negative = 0;
	uint64_t nodes = 0;
	uint64_t build_time = 0;
	uint64_t solve_time = 0;
	bool time_out = false;
	bool pass;
	//int n_deep = 0;
};
struct SearchScheme {
	Heuristic::Var vrh;
	Heuristic::Var vlh;
	Heuristic::DecisionScheme ds;

	string vrh_str;
	string vlh_str;
	string ds_str;
};
}