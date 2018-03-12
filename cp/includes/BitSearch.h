#pragma once
#include <climits>
#include <vector>
#include "XBuilder.h"
#include "BacktrackingSearch.h"

namespace cp {

	//class dynamic_bitset
	//{
	//public:
	//	dynamic_bitset();
	//	dynamic_bitset(const int n);
	//	~dynamic_bitset();
	//	u64 * data_;
	//	int num_bit;
	//	int limit;
	//};

	enum SearchMethod {
		SM_MAC,
		SM_DFS,
		SM_BAB,
		SM_BIDFS
	};

	class BVal {
	public:
		int v;
		int a;
		bool aop = true;
		BVal() : v(-2), a(-2) {};
		BVal(const int v, const int a, const bool aop = true) :v(v), a(a), aop(aop) {};
		const BVal& operator=(const BVal& rhs);
		void flip();
		BVal next() const;
		bool operator==(const BVal& rhs);
		bool operator!=(const BVal& rhs);
		friend std::ostream& operator<< (std::ostream &os, const BVal& v_val);
		virtual ~BVal() {};
	};

	namespace Nodes {
		const BVal RootNode = BVal(-1, 0);
		const BVal Deadend = BVal(-2, -1);
		const BVal NullNode = BVal(-2, -2);
		const BVal OutLastBroNode = BVal(-2, -3);
		const BVal OutLastNode = BVal(-2, -4);
	}

	class bit_assigned_stack {
	public:
		bit_assigned_stack() {};
		~bit_assigned_stack() {};

		void initial(const int num_vars);
		void push(const BVal& v_a);
		BVal pop();
		BVal top() const;
		int size() const;
		int capacity() const;
		bool full() const;
		bool empty() const;
		int operator[](const int i) const;
		BVal at(const int i) const;
		void clear();
		bool assigned(const int v) const;
		friend std::ostream& operator<< (std::ostream &os, const bit_assigned_stack &I);
		//friend std::ostream& operator<< (std::ostream &os, bit_assigned_stack* I);
		//vector<int> solution();
	protected:
		vector<BVal> vals_;
		vector<int> asnd_;
		int top_ = 0;
		int max_size_;
	};

	class BitSearch {
	public:
		BitSearch(const HModel& m);
		bool initial();
		void build_nei_model();
		void build_full_model();
		bool propagate(const BVal& val);
		void back_level();
		bool nonbinary_search(const Heuristic::Var varh, const Heuristic::Val valh, const bool neibor_propagate, const int time_limits);
		virtual ~BitSearch();
		BVal select_BVal(const Heuristic::Var varh, const Heuristic::Val valh) const;
		int select_BVar(const Heuristic::Var varh) const;
		int select_value(const int v, const Heuristic::Val valh) const;
		bool next(BVal& v_a) const;
		inline int size(const int v, const int p) const;
		inline int head(const int v, const int p) const;
		SearchStatistics statistics() const;
		int top_;
		const int limit;
		const int num_vars;
		const int num_cons;
		const int max_arity;
		const int max_dom_size;
		const int max_bitDom_size;
		const int len_stack;
		bool neibor_prop;
		int** constraint_matrix;
		//wdeg启发式两种算法计算方法不相同
		double** wdeg;
		//double** ddeg;
		//deg启发式保留初始网络的信息
		double* deg;

		SAC* sac;
		u64** bit_dom_;
		u64*** stack_;
		u64**** bit_sub_dom_;
		bit_assigned_stack I;
		SearchStatistics ss;


	};
}
