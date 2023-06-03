#include <string>
#include <random>
#include <chrono>
#include <memory>
#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "tree.hpp"
using namespace std;


using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

seed_seq seq{(uint64_t) chrono::duration_cast<chrono::nanoseconds>
	(chrono::high_resolution_clock::now().time_since_epoch()).count(),
		(uint64_t) __builtin_ia32_rdtsc(),
		(uint64_t) (uintptr_t) make_unique<char>().get()
};

mt19937 rng(seq);


struct MyDistribution {
	std::string type;
	double lower;
	double upper;
	double mean;
	double standardDeviation;
};

struct Arguments {
	int seed;
	MyDistribution branchLengthDistribution;
	MyDistribution lambda;
	MyDistribution m;
	MyDistribution mu;
	int numberOfSpecies;
	int numberOfSites;
	int numSamples;
};

bool parseYamlFile(const std::string& filename, Arguments& args) {
	try {
		YAML::Node config = YAML::LoadFile(filename);

		// Read seed
		if (config["seed"]) {
			args.seed = config["seed"].as<int>();
			rng = mt19937(args.seed);
		}

		// Read branch-length-distribution
		if (config["branch-length-distribution"]) {
			YAML::Node blDistNode = config["branch-length-distribution"];
			if (blDistNode["uniform"]) {
				YAML::Node uniformNode = blDistNode["uniform"];
				args.branchLengthDistribution.type = "uniform";
				args.branchLengthDistribution.lower = uniformNode["lower"].as<double>();
				args.branchLengthDistribution.upper = uniformNode["upper"].as<double>();
			} else if (blDistNode["normal"]) {
				YAML::Node normalNode = blDistNode["normal"];
				args.branchLengthDistribution.type = "normal";
				args.branchLengthDistribution.mean = normalNode["mean"].as<double>();
				args.branchLengthDistribution.standardDeviation = normalNode["standard-deviation"].as<double>();
			} else {
				std::cerr << "Invalid branch-length-distribution specified." << std::endl;
				return false;
			}
		} else {
			std::cerr << "branch-length-distribution not specified." << std::endl;
			return false;
		}

		// Read lambda
		if (config["lambda"]) {
			YAML::Node lambdaNode = config["lambda"];
			if (lambdaNode.IsScalar()) {
				args.lambda.type = "scalar";
				args.lambda.mean = lambdaNode.as<double>();
			} else if (lambdaNode["uniform"]) {
				YAML::Node uniformNode = lambdaNode["uniform"];
				args.lambda.type = "uniform";
				args.lambda.lower = uniformNode["lower"].as<double>();
				args.lambda.upper = uniformNode["upper"].as<double>();
			} else if (lambdaNode["normal"]) {
				YAML::Node normalNode = lambdaNode["normal"];
				args.lambda.type = "normal";
				args.lambda.mean = normalNode["mean"].as<double>();
				args.lambda.standardDeviation = normalNode["standard-deviation"].as<double>();
			} else {
				std::cerr << "Invalid lambda distribution specified." << std::endl;
				return false;
			}
		} else {
			std::cerr << "lambda not specified." << std::endl;
			return false;
		}

		// Read m
		if (config["m"]) {
			YAML::Node mNode = config["m"];
			if (mNode.IsScalar()) {
				args.m.type = "scalar";
				args.m.mean = mNode.as<double>();
			} else if (mNode["uniform"]) {
				YAML::Node uniformNode = mNode["uniform"];
				args.m.type = "uniform";
				args.m.lower = uniformNode["lower"].as<double>();
				args.m.upper = uniformNode["upper"].as<double>();
			} else if (mNode["normal"]) {
				YAML::Node normalNode = mNode["normal"];
				args.m.type = "normal";
				args.m.mean = normalNode["mean"].as<double>();
				args.m.standardDeviation = normalNode["standard-deviation"].as<double>();
			} else {
				std::cerr << "Invalid m distribution specified." << std::endl;
				return false;
			}
		} else {
			std::cerr << "m not specified." << std::endl;
			return false;
		}

		// Read mu
		if (config["mu"]) {
			YAML::Node muNode = config["mu"];
			if (muNode.IsScalar()) {
				args.mu.type = "scalar";
				args.mu.mean = muNode.as<double>();
			} else if (muNode["uniform"]) {
				YAML::Node uniformNode = muNode["uniform"];
				args.mu.type = "uniform";
				args.mu.lower = uniformNode["lower"].as<double>();
				args.mu.upper = uniformNode["upper"].as<double>();
			} else if (muNode["normal"]) {
				YAML::Node normalNode = muNode["normal"];
				args.mu.type = "normal";
				args.mu.mean = normalNode["mean"].as<double>();
				args.mu.standardDeviation = normalNode["standard-deviation"].as<double>();
			} else {
				std::cerr << "Invalid mu distribution specified." << std::endl;
				return false;
			}
		} else {
			std::cerr << "mu not specified." << std::endl;
			return false;
		}

		// Read number-of-species and number-of-sites
		args.numberOfSpecies = config["number-of-species"].as<int>();
		args.numberOfSites = config["number-of-sites"].as<int>();
		
		args.numSamples = -1;
		if (config["number-of-samples"]) {
			args.numSamples = config["number-of-samples"].as<int>(); 
		}

		return true;
	} catch (const std::exception& e) {
		std::cerr << "Failed to parse YAML file: " << e.what() << std::endl;
		return false;
	}
}


void generate_tree(const Arguments& args, string output_file) {
	double lamda;
	if(args.lambda.type == "scalar") {
		lamda = args.lambda.mean;
	} else if(args.lambda.type == "normal") {
		lamda = normal_distribution<double>(args.lambda.mean, args.lambda.standardDeviation)(rng);
	} else {
		lamda = uniform_real_distribution<double>(args.lambda.lower, args.lambda.upper)(rng);
	} 


	double mu;
	if(args.mu.type == "scalar") {
		mu = args.mu.mean;
	} else if(args.mu.type == "normal") {
		mu = normal_distribution<double>(args.mu.mean, args.mu.standardDeviation)(rng);
	} else {
		mu = uniform_real_distribution<double>(args.mu.lower, args.mu.upper)(rng);
	}

	mu = max(lamda, mu);

	double m;
	if(args.m.type == "scalar") {
		m = args.m.mean;
	} else if(args.m.type == "normal") {
		m = normal_distribution<double>(args.m.mean, args.m.standardDeviation)(rng);
	} else {
		m = uniform_real_distribution<double>(args.m.lower, args.m.upper)(rng);
	}

	int NUMBER_OF_SPECIES = args.numberOfSpecies;
	int NUMBER_OF_SITES = args.numberOfSites;

	cout << "NUMBER_OF_SPECIES: " << NUMBER_OF_SPECIES << endl;
	cout << "NUMBER_OF_SITES:" << NUMBER_OF_SITES << endl;
	cout << "lambda: " << lamda << endl;
	cout << "mu: " << mu << endl;
	cout << "m: " << m << endl;

	tree::phylogenetic_tree t(NUMBER_OF_SPECIES,lamda,m,mu, NUMBER_OF_SITES);

	cout << "Generating topology" << endl;
	tree::generate_random_topology(t, rng);
	
	if(lamda != -1 || m != -1 || mu != -1) {
		cout << "Generating kword-counts" << endl;
		vector<int> initial_state(NUMBER_OF_SITES);
		for(int i=0;i<initial_state.size();i++) {
			initial_state[i] = tree::simulate_transition(lamda,mu,m,
				1000,500,rng).first;
		}
		tree::generate_kword_counts(t,initial_state,rng);
	}

	cout << "Generating branch lengths" << endl;
	if(args.branchLengthDistribution.type == "uniform") {
		auto gen = uniform_real_distribution<double>(args.branchLengthDistribution.lower, args.branchLengthDistribution.upper);
		tree::assign_randomized_branch_length(t,gen, rng);
	} else if(args.branchLengthDistribution.type == "normal") {
		auto gen = normal_distribution<double>(args.branchLengthDistribution.mean, args.branchLengthDistribution.standardDeviation);
		tree::assign_randomized_branch_length(t,gen, rng);
	}

	cout << "Writing to File" << endl;
	t.write_to_file(output_file);
	cout << "Done" << endl;
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "Usage: ./program <yaml-file> <output-file>" << std::endl;
		return 1;
	}

	Arguments args;

	if (!parseYamlFile(string(argv[1]), args)) {
		return 1;
	}

	string output_file = string(argv[2]);
	if(args.numSamples == -1) {
		generate_tree(args, output_file);
	} else {
		for(int i=0; i<args.numSamples; i++) {
			auto out = output_file + '-' + to_string(i);
			generate_tree(args, out);
		}
	}

	return 0;
}

