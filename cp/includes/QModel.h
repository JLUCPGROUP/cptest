#pragma once
#include <climits>
#include <vector>
#include "XBuilder.h"

namespace cp {

	inline bool Existed(vector<int>& tuple) {
		return tuple[0] != INT_MAX;
	}

	inline void Exclude(vector<int>& tuple) {
		tuple[0] = INT_MAX;
	}

	class QVar {
	public:
		QVar(HVar* v);
		~QVar();
		void enable_backtracking(const int size);
		void disable_backtracking();
		void remove_value(const int a, const int p) const;
		void reduce_to(const int a, const int p) const;
		int size(const int p) const;
		inline int next(const int a, const int p) const;
		void next_value(int& a, const int p) const;
		//int prev(const int a, const int p) const;
		inline bool have(const int a, const int p) const;
		inline int head(const int p) const;
		//inline bool assigned(const int p) const { return assigned_[p]; }
		//inline void assign(const bool a, const int p) { assigned_[p] = a; }
		//int tail(const int p) const;
		bool faild(const int p) const { return size(p) == 0; };
		u64* bitDom(const int p) const { return bit_doms_[p]; }
		void show(const int p) const;
		//inline void back_to(const int src, const int dest);
		void delete_level(const int p) const;
		void copy_level(const int src, const int dest) const;
		//inline int new_level(int src);
		void new_level(const int src, const int dest) const;
		//int max_value(const int p);
		const int id;
		const int capacity;
		const int limit;
		const int num_bit;
		const vector<int> vals;
		const int size_tmp;
	private:
		bool backtrackable_ = false;
		int* size_;
		u64* bit_tmp_;
		u64** bit_doms_;
		int num_bd_;
	};

	class QVal {
	public:
		QVar * v;
		int a;
		bool aop = true;
		QVal() : v(nullptr), a(-2) {};
		QVal(QVar* v, const int a, const bool aop = true) :v(v), a(a), aop(aop) {};

		const QVal& operator=(const QVal& rhs);
		void flip();
		QVal next() const;
		bool is_null_node() const;
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
		~assignments_stack() {};
		void initial(const HModel& m);
		void push(const QVal& v_a);
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
		//bool assigned(const QVar* v) const;
		bool assigned(const QVar& v) const;
		bool solution(vector<int>& sol) const;
		friend ostream& operator<< (std::ostream &os, assignments_stack &I);
		friend ostream& operator<< (std::ostream &os, assignments_stack* I);
		vector<int> v_;
	protected:
		//int* v_;
		//QVal* qvals_;
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

	class vars_heap {
	public:
		vars_heap() {};
		virtual ~vars_heap();
		void push(QVar &  v, const int p);
		QVar* pop(const int p);
		void initial(const int size);
		void del() const;
		void insert(QVar& v, const int p);
		bool empty() const { return !cur_size_; };
		QVar* remove_at(const int location, const int p);
		void clear();
	private:
		static inline bool compare(QVar const & a, QVar const & b, const int p);
		void filter_up(const int start, const int p) const;
		void filter_down(const int start, const int finish, const int p) const;
		int* position_;
		QVar**  vs_;
		int max_size_;
		int cur_size_ = 0;

	};

	class QTab {
	public:
		QTab(HTab* t, vector<QVar*>& scope);
		bool sat(vector<int> &t) const;
		void get_first_valid_tuple(QVal& v_a, vector<int>& t, const int p);
		void get_next_valid_tuple(QVal& v_a, vector<int>& t, const int p);
		void get_first_valid_tuple(const QVar& v, const int a, vector<int>& t, const int p) const;
		void get_next_valid_tuple(const QVar& v, const int a, vector<int>& t, const int p) const;
		//int index(QVar* v) const;
		int index(QVar const &  v) const;
		bool is_valid(vector<int>& t, const int p) const;
		const int id;
		const int arity;
		vector<QVar*> scope;
		vector<vector<int>> tuples;
		~QTab() {};
	};

	class arc {
	public:
		QTab * c;
		QVar * v;
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
		int get_var_index()const { return c->index(*v); }
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
		u64 revisions = 0;
	};

	struct SearchStatistics {
		u64 num_sol = 0;
		u64 num_positives = 0;
		u64 num_negatives = 0;
		u64 nodes = 0;
		u64 initial_propagate_time = 0;
		u64 build_time = 0;
		u64 search_time = 0;
		u64 total_time = 0;
		u64 solve_time = 0;
		bool pass;
		bool time_out = false;
		bool sat_initial = true;
		u64 num_revisions = 0;
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