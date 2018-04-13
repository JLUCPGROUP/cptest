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

	class UTuple {
	public:
		UTuple(const vector<int> &t);

	protected:
		int *t_;
		int arity_;
		int max_arity_;
	};

	class UTupleSet {
	public:
		bool binary_search(const UTuple &t);

	private:
		int **ts_;
		int arity_;
		int max_arity_;
	};

	class UVar {
	public:
		UVar(HVar *v);
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
