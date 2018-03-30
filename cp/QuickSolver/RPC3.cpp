#include "BacktrackingSearch.h"
#include <set>

namespace cp {
	RPC3::RPC3(const HModel& h, const bool backtrackable) :BacktrackingSearch(h, backtrackable) {
		rel_.resize(num_vars, vector<vector<vector<int>>>(num_vars, vector<vector<int>>(max_dom_size, vector<int>(max_dom_size, false))));
		common_neibor_.resize(num_vars, vector<vector<QVar*>>(num_vars));
		//set<QVar*> vars_map;
		vector<bool> vars_in(num_vars, false);
		for (auto x : vars) {
			for (auto y : vars) {
				if (x != y) {
					for (auto z : vars) {
						if (!neibor_matrix[x->id][z->id].empty() && !neibor_matrix[y->id][z->id].empty()) {
							//vars_map.insert(z);
							if (!vars_in[z->id]) {
								vars_in[z->id] = true;
								common_neibor_[x->id][y->id].push_back(z);
							}
						}
					}
				}
				vars_in.assign(vars_in.size(), false);
			}
		}
		//initial bitSup
		//const int n = num_tabs * max_dom_size * max_arity;
		//bitSup_ = new u64*[n];
		//for (size_t i = 0; i < n; i++)
		//	bitSup_[i] = new u64[max_bitDom_size]();

		for (QTab* c : tabs) {
			const int x = c->scope[0]->id;
			const int y = c->scope[1]->id;
			for (auto& t : c->tuples) {
				rel_[x][y][t[0]][t[1]] = true;
				rel_[y][x][t[1]][t[0]] = true;
			}
		}
		/////
		//q_nei_.initial(num_vars);
		//N.resize(num_vars);
		//生成邻域子网矩阵
		//for (auto v : vars) {
		//	for (auto x : neighborhood[v->id]) {
		//		for (auto y : neighborhood[v->id]) {
		//			//只计算三角形，避免重复加入约束
		//			if (x->id > y->id && !neibor_matrix[x->id][y->id].empty()) {
		//				N[v->id].push_back(neibor_matrix[x->id][y->id][0]);
		//			}
		//		}
		//	}
		//}

		//var_mark_.resize(num_vars, -1);

		con_que_.initial(num_vars);
		for (auto x : vars) {
			for (auto y : vars) {
				if (!neibor_matrix[x->id][y->id].empty()) {
					con_que_.push(variable_pair{ x,y });
				}
			}
		}
		//con_que_.clear_and_push(tabs);
		r_1_.resize(num_vars, vector<vector<int>>(max_dom_size, vector<int>(num_vars, Limits::INDEX_OVERFLOW)));
		r_2_.resize(num_vars, vector<vector<int>>(max_dom_size, vector<int>(num_vars, Limits::INDEX_OVERFLOW)));
	}

	PropagationState RPC3::propagate(vector<QVar*>& x_evt, const int p) {

		//初始化con_que_
		if (x_evt.size() == 1) {
			con_que_.clear();
			//con_que_.clear_and_push(subscription[x_evt[0]->id]);
			const auto x = x_evt[0];
			for (auto y : neighborhood[x->id]) {
				con_que_.push(variable_pair{ y,x });
				con_que_.push(variable_pair{ x,y });
			}
		}
		int R;

		while (!con_que_.empty()) {
			const auto vp = con_que_.pop();
			cout << "pop:" << vp.x->id << "," << vp.y->id << endl;
			bool deletion = false;
			for (auto a = vp.x->head(p); a != Limits::INDEX_OVERFLOW; vp.x->next_value(a, p)) {
				cout << "test: (" << vp.x->id << "," << a << ")" << endl;
				const bool valid[] = {
					vp.y->have(r_1_[vp.x->id][a][vp.y->id], p),
					vp.y->have(r_2_[vp.x->id][a][vp.y->id], p) };

				//both r valid
				if (valid[0] && valid[1]) {
					continue;
				}
				else {
					if (valid[0] && !valid[1]) {
						R = r_1_[vp.x->id][a][vp.y->id];
					}
					else if (!valid[0] && valid[1]) {
						R = r_2_[vp.x->id][a][vp.y->id];
					}
					else {
						R = Limits::INDEX_OVERFLOW;
					}

					cout << "find_two_support(" << vp.x->id << "," << a << "," << vp.y->id << "," << R << ")" << endl;
					if (!find_two_support(*vp.x, a, *vp.y, R, p)) {
						vp.x->remove_value(a, p);
						cout << "remove P:" << p << ":" << vp.x->id << "," << a << endl;
						deletion = true;
					}
				}

				if (vp.x->faild(p)) {
					ps_.state = false;
					return ps_;
				}

				if (deletion) {
					for (const auto z : neighborhood[vp.x->id]) {
						con_que_.push(variable_pair{ z,vp.x });
						//for (const auto w : neighborhood[z->id]) {
						//	auto c = neibor_matrix[w->id][vp.x->id];
						//	if (w != vp.x&&c.empty()) {
						//		con_que_.push(variable_pair{ w,z });
						//	}
						//}
					}
				}
			}
		}
		ps_.state = true;
		return ps_;
	}
	bool RPC3::is_consistent(const QVar& x, const int a, const QVar& y, const int b) {
		cout << "      consistent check:(" << x.id << "," << a << ")~(" << y.id << "," << b << ") =>" << rel_[x.id][y.id][a][b] << endl;
		return rel_[x.id][y.id][a][b];
	}

	//bool RPC3::is_consistent(const QTab& c, const QVar& x, const int a, const QVar& y, const int b) {
	//	int t[2];
	//	t[c.index(x)] = a;
	//	t[c.index(y)] = b;
	//	return rel_[c.id][t[0]][t[1]];
	//}

	int RPC3::find_two_support(const QVar& x, const int a, const QVar& y, const int r, const int p) {
		bool one_support = (r != Limits::INDEX_OVERFLOW);
		//const auto c_xy = neibor_matrix[x.id][y.id][0];

		auto b = Limits::INDEX_OVERFLOW;
		for (auto last_b = y.head(p); last_b != Limits::INDEX_OVERFLOW; y.next_value(last_b, p)) {
			b = last_b;
			cout << "test: (" << x.id << "," << a << ")-(" << y.id << "," << last_b << ")" << endl;

			//L4
			if (is_consistent(x, a, y, last_b)) {
				cout << "consistent" << endl;
				if (!one_support) {
					//L5 若一个支持都没有，放在r1
					cout << "1 support" << endl;
					one_support = true;
					r_1_[x.id][a][y.id] = last_b;
				}
				else {
					//有一个支持放在r2
					cout << "2 support" << endl;
					if (last_b != r) {
						r_2_[x.id][a][y.id] = last_b;
						return true;
					}
				}

			}
		}
		cout << "---------" << endl;
		//若单个支持 则要为(ai,aj)检查pc
		//singleton
		if (!one_support) {
			//无支持
			cout << "no support" << endl;
			return false;
		}
		else {
			//单个支持
			b = r_1_[x.id][a][y.id];

			cout << "1 support, seek pc" << endl;
			for (auto z : common_neibor_[x.id][y.id]) {
				cout << "pcw: " << z->id << endl;
				const bool valid[] = {
					z->have(r_1_[x.id][a][z->id],p),
					z->have(r_2_[x.id][a][z->id],p),
					z->have(r_1_[y.id][b][z->id], p),
					z->have(r_2_[y.id][b][z->id], p) };
				//R[x,a,z]有一个有效，并且该有效R与y,b相容
				if (valid[0] || valid[1]) {
					if (valid[0]) {
						cout << "    valid[0]: (" << z->id << "," << r_1_[x.id][a][z->id] << endl;
						//并且该有效R与y,b相容
						if (is_consistent(y, b, *z, r_1_[x.id][a][z->id])) {
							cout << "   consistent" << endl;
							continue;
						}
						else {
							cout << "   inconsistent" << endl;
						}
					}

					if (valid[1]) {
						cout << "    valid[1]: (" << z->id << "," << r_2_[x.id][a][z->id] << endl;
						//并且该有效R与y,b相容
						if (is_consistent(y, b, *z, r_2_[x.id][a][z->id])) {
							cout << "   consistent" << endl;
							continue;
						}
						else {
							cout << "   inconsistent" << endl;
						}


					}
					//如果两个都valid那么上一个
				}
				///////////////////////////////////

				if (valid[2] || valid[3]) {
					cout << "    valid[2]: (" << z->id << "," << r_1_[y.id][b][z->id] << endl;
					if (valid[2]) {
						//并且该有效R与x,a相容
						if (is_consistent(x, a, *z, r_1_[y.id][b][z->id])) {
							cout << "   consistent" << endl;
							continue;
						}
						else {
							cout << "   inconsistent" << endl;
						}

					}
					if (valid[3]) {
						//并且该有效R与x,a相容
						cout << "    valid[3]: (" << z->id << "," << r_2_[y.id][b][z->id] << endl;
						if (is_consistent(x, a, *z, r_2_[y.id][b][z->id])) {
							cout << "    consistent" << endl;

							continue;
						}
						else {
							cout << "    inconsistent" << endl;
						}

					}
					//如果两个都valid那么上一个
				}

				bool pc_witness = false;
				cout << "non valid r, seek new pcw" << endl;
				for (auto c = z->head(p); c != Limits::INDEX_OVERFLOW; z->next_value(c, p)) {
					cout << "test: (" << x.id << "," << a << ")-(" << z->id << "," << c << ")-(" << y.id << ", " << b << ")" << endl;
					if (is_consistent(x, a, *z, c) && is_consistent(y, b, *z, c)) {
						cout << "    consistent" << endl;
						pc_witness = true;
						break;
					}
				}

				if (!pc_witness) {
					cout << "    non consistent need delete" << endl;
					return false;
				}

			}
		}

		return true;
	}
}
