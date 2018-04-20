#pragma once
#include <climits>
#include <vector>
#include "XBuilder.h"
#include <vector>
#include "HModel.h"
using namespace std;
namespace cp {
	typedef unsigned long long  u64;

#define SWAP(a, b){ \
  a = a ^ b;        \
  b = a ^ b;        \
  a = a ^ b;}    

	class UVar;
	class UTab;
	class UVal;
	class UTuple;
	class UTupleSet;
	struct SearchStatistics;
	struct SearchScheme;


	class UTuple {
	public:
		UTuple(const vector<int> &t);

	protected:
		int *t_;
		int arity_;
		int max_arity_;
	};

	struct SearchScheme {
		Heuristic::Var vrh;
		Heuristic::Var vlh;
		Heuristic::DecisionScheme ds;

		string vrh_str;
		string vlh_str;
		string ds_str;
	};

	class UTupleSet {
	public:
		bool binary_search(const UTuple &t);

	private:
		int **ts_;
		int arity_;
		int max_arity_;
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
	};


	class UVar {
	public:
		UVar(const HVar& v);
		void remove_value(const int a);
		//bind
		void reduece_to(const int a);
		//void size();
		//int next();
		bool have(const int a);
		//int head();
		//bool faild();
		//u64 *bitDom();
		//void show();
		void clear_marks();
		void restrict();
		void mark(const int a);
		inline void swap(const int i, const int j);
		const int id;
		const int capacity;
		const int limit;
		const int num_bit;
		const vector<int> values;

	protected:
		int level = 0;
		vector<int> map_;
		vector<int> dom_;
		int size_;
		int mark_;
	};
}
