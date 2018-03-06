#pragma once
#include "QModel.h"
namespace cp {
	class BacktrackingSearch {
	public:
		BacktrackingSearch(const HModel& h);
		virtual ~BacktrackingSearch();
		inline int new_level();
		inline int back_level();
		inline void copy_level(const int src, const int dest);
		//void clear_level(const int p);
		void insert(QVar& v, const int p);
		SearchStatistics statistics() const;
		virtual PropagationState propagate(vector<QVar*>& x_evt, const int level) = 0;

		virtual SearchStatistics solve(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits);
		QVal select_QVal(const Heuristic::Var varh, const Heuristic::Val valh, const int p);
		QVar* select_QVar(const Heuristic::Var varh, const int p);
		int select_value(const QVar& v, const Heuristic::Val valh, const int p);
		/*	inline bool entail(const QTab& c);*/
		vector<QVar*> vars;
		vector<QTab*> tabs;
		vector<u64> con_stamp;
		vector<u64> var_stamp;
		vector<double> con_weight;
		u64 time = 0;
		//unordered_map<QVar*, vector<QTab*>> subscription;
		//unordered_map<QVar*, vector<QVar* >> neighborhood;
		vector<vector<QTab*>> subscription;
		vector<vector<QVar*>> neighborhood;
		vector<vector<vector<QTab*>>> neibor_matrix;
		//vector<vector<QVar*>> neibor_vars;
		//  由于所有变量的域长度不一定相同 所以这里的c-value值不一定真实存在
		int get_QConVal_index(const QConVal& c_val)const;
		int get_QConVal_index(const QTab& c, const QVar& v, const int a) const;
		int get_QConVal_index(const QTab& c, const int v_idx, const int a) const;
	protected:
		vector<int> tmp_tuple_;
		vars_heap q_;
		vector<QVar*> get_scope(const HTab& t);
		vector<QVar*> get_neighbor(const QVar& v);
		const int max_arity;
		const int max_dom_size;
		const int max_bitDom_size;
		const int num_vars;
		const int num_tabs;
		const int num_cva;
		PropagationState ps_;
		SearchStatistics ss_;
		assignments_stack I_;
		int top_ = 0;
		int tmp_;
	};

	//class MAC3 :public BacktrackingSearch {
	//public:
	//	MAC3(HModel *h);
	//	~MAC3() {};
	//	PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
	//	bool revise(QTab* c, QVar* v, const int level);
	//	bool seek_support(QTab* c, QVar* v, int a, const int level);
	//};
	class MAC3 :public BacktrackingSearch {
	public:
		MAC3(const HModel& h);
		virtual ~MAC3();
		PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
		inline bool revise(const QTab& c, const QVar& v, const int level);
		inline bool seek_support(const QTab& c, const QVar& v, const int a, const int p);
	};

	class MAC3bit :public BacktrackingSearch {
	public:
		MAC3bit(const HModel& h);
		virtual ~MAC3bit();
		inline PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
		inline bool revise(const QTab& c, const QVar& v, const int level);
		inline bool seek_support(const QTab& c, const QVar& v, const int a, const int p) const;
	protected:
		u64 * * bitSup_;
	};

	class MAC3rm :public BacktrackingSearch {
	public:
		MAC3rm(const HModel& h);
		virtual ~MAC3rm();
		inline PropagationState propagate(vector<QVar*>& x_evt, const int level) override;
		inline bool revise(const QTab& c, const QVar& v, const int level);
		inline bool seek_support(const QTab& c, const QVar& v, const int a, const int p);
	protected:
		vector<vector<int>> res_;
		//int** res_;
		//u64 num_res_ = 0;
	};

	class lMaxRPC_bit_rm :public BacktrackingSearch {
	public:
		lMaxRPC_bit_rm(const HModel& h);
		virtual ~lMaxRPC_bit_rm();
		PropagationState propagate(vector<QVar*>& x_evt, const int p) override;
		bool have_pc_support(const QVar& i, const int a, const QVar& j, const int p);
		bool have_pc_wit(const QVar& i, const int a, const QVar& j, int b, const QVar& k, const int p);
		inline int next_support_bit(const QVar& i, const int a, const QVar& j, const int v, const int p);
		//int first_support_bit(const QVar& i, const int a, const QVar& j, const int v, const int p);
	protected:

		vector<vector<vector<QVar*>>> common_neibor_;
		vector<int> last_pc;
		vector<int> last_ac;
		u64** bitSup_;
	};
}
