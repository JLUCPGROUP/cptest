#include "UModel.h"

namespace cp {
	class BacktrackingSearch {
	public:
		BacktrackingSearch(const HModel& h, const bool backtrackable = true);
		virtual ~BacktrackingSearch();
		inline int new_level();
		inline int back_level();
		void copy_level(const int src, const int dest);
		//void clear_level(const int p);
		void insert(UVar& v, const int p);
		SearchStatistics statistics() const;
		virtual bool propagate(vector<UVar*>& x_evt, const int level) = 0;

		virtual SearchStatistics binary_search(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits);
		virtual SearchStatistics nonbinary_search(const Heuristic::Var varh, const Heuristic::Val valh, const int time_limits);

		QVal select_QVal(const Heuristic::Var varh, const Heuristic::Val valh, const int p);
		UVar* select_UVar(const Heuristic::Var varh, const int p);
		int select_value(const UVar& v, const Heuristic::Val valh, const int p);
		/*	inline bool entail(const QTab& c);*/
		vector<UVar*> vars;
		vector<QTab*> tabs;
		vector<u64> con_stamp;
		vector<u64> var_stamp;
		vector<double> con_weight;
		vector<int> var_deg_que;
		vector<int> deg;
		u64 time = 0;
		//unordered_map<UVar*, vector<QTab*>> subscription;
		//unordered_map<UVar*, vector<UVar* >> neighborhood;
		vector<vector<QTab*>> subscription;
		vector<vector<UVar*>> neighborhood;
		vector<vector<vector<QTab*>>> neibor_matrix;
		//vector<vector<UVar*>> neibor_vars;
		//  由于所有变量的域长度不一定相同 所以这里的c-value值不一定真实存在
		u64 get_QConVal_index(const QConVal& c_val)const;
		u64 get_QConVal_index(const QTab& c, const UVar& v, const int a) const;
		u64 get_QConVal_index(const QTab& c, const int v_idx, const int a) const;
		void enable_backtracking();
		void disable_backtracking();
		void show(const int p);
		vector<vector<int>> solutions;
		string get_solution_str();
		bool solution_check();

		string sol_str;
		vector<int> sol_std;
	protected:
		vector<int> tmp_tuple_;
		vars_heap q_;
		vector<UVar*> get_scope(const HTab& t);
		vector<UVar*> get_neighbor(const UVar& v);
		static inline void next_val(QVal& val, const int p);
		void get_solution();
		const int max_arity;
		const int max_dom_size;
		const int max_bitDom_size;
		const int num_vars;
		const int num_tabs;
		const int num_cva;
		PropagationState ps_;
		SearchStatistics ss_;
		assignments_stack I;
		int top_ = 0;
		int tmp_;
		bool backtrackable_ = false;
		vector<int> solution_;
		string name;
	};
}