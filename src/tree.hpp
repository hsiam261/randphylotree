#include <ios>
#include <vector>
#include <fstream>
#include <string>
#include <random>
#include <set>
#include <algorithm>
#include <cassert>
#include <queue>
using namespace std;
#ifndef TREE 
#define TREE 

namespace tree {
	using std::vector;
	using std::fstream;
	using std::string;
	using std::set;
	using std::pair;

	struct edge {
		int u,v;
		double branch_length;

		edge() {}
		edge(int u,int v, double branch_length) : 
			u(u),v(v), branch_length(branch_length) {}
	};

	/**
	 * rooted phylogenetic tree
	 * with number_of_species leaves (indexed 1 to number_of_species)
	 * 0 as root
	 * 2*number_of_species -1 nodes in total (0 to 2*number_of_species-2)
	 * 2*number_of_species-2 edges
	 */ 
	class phylogenetic_tree {
		void gen_newick(fstream & outfile,int root,vector<bool>& visited) {
			visited[root] = true;
			bool is_leaf = true;
			for(auto e:adjList[root]) {
				if(!visited[e.v]) {
					if(is_leaf) {
						outfile << "(";
						is_leaf = false;
					} else {
						outfile << ",";
					}
					
					gen_newick(outfile,e.v,visited);
					outfile << ":" << e.branch_length;
				}
			}

			if(!is_leaf) outfile << ")";
			outfile << root;
		}

		public:
		int n;
		int number_of_species;
		double mu,m,lamda;
		int number_of_sites;

		vector<vector<edge>> adjList;
		vector<vector<int>> kword_counts; //n x number_of_sites


		vector<std::string> node_names;


		phylogenetic_tree(int number_of_species,double lamda,
				double m, double mu, 
				int number_of_sites) : 
			number_of_species(number_of_species),
			n(number_of_species*2-1), 
			adjList(number_of_species*2-1),
			kword_counts(number_of_species*2-1,
					vector<int>(number_of_sites)),
			lamda(lamda), m(m), mu(mu), 
			number_of_sites(number_of_sites),
			node_names(number_of_species*2-1) {
				for(int i=0;i<n;i++) node_names[i] = 
					to_string(i);

		}

		void write_to_file(string filename) {
			fstream outfile;
			outfile.open(filename,std::ios_base::out);

			outfile << number_of_species << "\n";
			outfile << number_of_sites << "\n";
			outfile << lamda << " " << m << " " << mu << "\n";

			
			vector<bool> visited(n,false);
			queue<edge> q;
			visited[0] = true;

			for(auto &e: adjList[0]) {
				q.push(e);
				visited[e.v] = true;
				outfile << e.u << " " << e.v << " " 
				<< e.branch_length << "\n";
			}

			while(!q.empty()) {
				auto e = q.front();
				q.pop();

				for(auto &next: adjList[e.v]) {
					if(!visited[next.v]) {
						q.push(next);
						visited[next.v] = true;
						outfile << next.u << " " 
						<< next.v << " " 
						<< next.branch_length << "\n";
					}
				}
			}

			for(int i=0;i<n;i++) {
				for(int k=0;k<number_of_sites;k++) {
					outfile << kword_counts[i][k] << " "; 
				}
				outfile << "\n";
			}

			outfile.close();
		}

		void dump_topology_newick(std::string filename)	{
			fstream outfile;
			outfile.open(filename,std::ios_base::out);
			
			vector<bool> visited(this->n,false);
			gen_newick(outfile,0,visited);
			outfile << ";";
			outfile.close();
			
		}

		void dump_kword_count(std::string filename) {
			fstream outfile;
			outfile.open(filename,std::ios_base::out);

			outfile << n << "\n";
			outfile << number_of_sites << '\n';

			for(int i=0;i<n;i++) {
				for(int j=0;j<number_of_sites;j++) outfile << 
					kword_counts[i][j] << ' ';
				outfile << '\n';
			}

			outfile.close();
		}


		void dump_names(std::string filename) {
			
		}

	};
	


	/**
	 * takes a tree and a generator 
	 * and generates a random topology
	 * by updating it's adjList
	 * branch_lengths are not generated
	 */ 
	template<class Generator>
	void generate_random_topology(phylogenetic_tree& t,Generator &gen) {
		set<std::pair<int,int>> s;
		if(t.number_of_species>=2) s.insert({1,2});

		for(int i=3;i<=t.number_of_species;i++) {
			//pick a random branch in the (i-1) leaved tree
			//there are 2(i-1)-3 edges (0 to 2*i-6)
			std::uniform_int_distribution<int> dist(0,s.size()-1);
			int idx = dist(gen);

			int j=0;
			auto it=s.begin();
			for(;it!=s.end() && j<idx;it++,j++);
			assert(it!=s.end());	
			auto e = *it;
			s.erase(it);
		
			s.insert({t.number_of_species+i-2,e.first});
			s.insert({t.number_of_species+i-2,e.second});
			s.insert({t.number_of_species+i-2,i});
			
			
		}

		//add root;
		std::uniform_int_distribution<int> dist(0,s.size()-1);
		int idx = dist(gen);
		int j=0;
		auto it=s.begin();
		for(;it!=s.end() && j<idx;it++,j++);
		assert(it!=s.end());	
		auto e = *it;

		s.erase(it);
		s.insert({0,e.first});
		s.insert({0,e.second});

		vector<int> v(t.number_of_species);
		for(int i=0;i<t.number_of_species;i++) v[i]=(i+1);
		std::shuffle(v.begin(),v.end(),gen);


		vector<pair<int,int>> edges;
		for(auto x:s) edges.push_back(x);

		for(auto &e:edges) {
			if(e.first<=t.number_of_species && e.first>0) {
				e.first = v[e.first-1];
			}

			if(e.second<=t.number_of_species && e.second>0) {
				e.second = v[e.second-1];
			}
		}

		for(auto e:edges) {
			t.adjList[e.first].push_back({e.first,e.second,-1});
			t.adjList[e.second].push_back({e.second,e.first,-1});
		}
	}

	
	template<class Distribution, class Generator>
	void assign_randomized_branch_length(phylogenetic_tree& t,
			Distribution& dist,
			Generator& rng) {
		for(int i=0;i<t.n;i++) {
			for(int j=0;j<t.adjList[i].size();j++) {
				auto &e = t.adjList[i][j];
				if(e.u<e.v) {
					e.branch_length = dist(rng);
					for(int k=0;k<t.adjList[e.v].size();
							k++) {
						auto &e2 = t.adjList[e.v][k];
						auto b = e.branch_length;
						if(e2.v == e.u) {
							e2.branch_length = b;
							break;
						}
					}
				}
			}
		}	
	}
	/**
	 * returns {kmer_count,number of iteration}
	 */  
	template<class Generator>
	pair<int,int> simulate_transition(double lamda,double mu,double m,
			double cutoff_time,
			int initial_state,Generator& rng) {
		double t=0;
		int cnt=0;
		int curr = initial_state;
		while(t<cutoff_time) {
			auto exp = exponential_distribution<>(curr*lamda
					+curr*mu+m);
			double x = exp(rng);
			double next_transition = t+x;
	
			if(next_transition>=cutoff_time) return {curr,cnt};
			t = next_transition;
	
			double p = (curr*lamda+m)/(curr*lamda+curr*mu+m);
			auto bern = bernoulli_distribution(p);
			int toss = bern(rng);
			
			if(toss) curr++;
			else curr--;
			
			cnt++;
		}
		
		assert(false);
	}
	
	template<class Generator>
	void generate_kword_counts(phylogenetic_tree& t,
			int initial_state,int site_no,Generator& rng) {
		vector<bool> visited(t.n,false);
		visited[0] = true;
		t.kword_counts[0][site_no] = initial_state;
		
		queue<edge> q;
		for(auto& e:t.adjList[0]) {
			q.push(e);
			visited[e.v]=true;
		}

		while (!q.empty()) {
			auto e = q.front();
			q.pop();
			int next_count = simulate_transition(t.lamda, t.mu, 
					t.m, e.branch_length, 
					t.kword_counts[e.u][site_no], 
					rng).first;
			t.kword_counts[e.v][site_no] = next_count;
			for(auto& next:t.adjList[e.v]) {
				if(!visited[next.v]) {
					q.push(next);
					visited[next.v] = true;
				}
			}
		}

	}

	template<class Generator>
	void generate_kword_counts(phylogenetic_tree& t,
			const vector<int>& initial_states,Generator& rng) {
		for(int i=0;i<t.number_of_sites;i++) {
			generate_kword_counts(t,initial_states[i],i,rng);
		}
	}

}


#endif
