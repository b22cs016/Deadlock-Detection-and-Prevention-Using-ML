#ifndef DEADLOCK_PREVENTION_HPP
#define DEADLOCK_PREVENTION_HPP

#include <vector>
#include <unordered_map>
#include <set>
#include <random>
#include <cmath>
#include <functional>
#include <thread>
#include <chrono>
#include <fstream>
#include <algorithm>

class SimpleNeuralNetwork {
private:
    std::vector<std::vector<double>> weights1;
    std::vector<std::vector<double>> weights2;
    std::vector<double> bias1;
    std::vector<double> bias2;
    
    double sigmoid(double x) {
        return 1.0 / (1.0 + exp(-x));
    }

public:
    SimpleNeuralNetwork(int input_size, int hidden_size) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<> d(0, 1);

        // Initialize weights and biases
        weights1 = std::vector<std::vector<double>>(input_size, std::vector<double>(hidden_size));
        weights2 = std::vector<std::vector<double>>(hidden_size, std::vector<double>(1));
        bias1 = std::vector<double>(hidden_size);
        bias2 = std::vector<double>(1);

        // Random initialization
        for(int i = 0; i < input_size; i++) {
            for(int j = 0; j < hidden_size; j++) {
                weights1[i][j] = d(gen) * 0.1;
            }
        }
        for(int i = 0; i < hidden_size; i++) {
            weights2[i][0] = d(gen) * 0.1;
            bias1[i] = d(gen) * 0.1;
        }
        bias2[0] = d(gen) * 0.1;
    }

    double predict(const std::vector<double>& input);
    void train(const std::vector<std::vector<double>>& X, const std::vector<double>& y);
};

class MLAugmentedDeadlockPrevention {
private:
    int num_resources;
    int num_processes;
    std::vector<int> available;
    std::vector<std::vector<int>> allocated;
    std::vector<std::vector<int>> max_need;
    
    SimpleNeuralNetwork risk_model;
    
    // Resource Allocation Graph
    std::unordered_map<int, std::set<int>> rag;
    
    // Training history
    struct TrainingExample {
        std::vector<double> features;
        bool led_to_deadlock;
    };
    std::vector<TrainingExample> history;

    bool is_safe_state(int process_id, const std::vector<int>& requested);
    bool can_complete(int process_id, std::vector<int>& work, const std::vector<std::vector<int>>& allocated);

public:
    MLAugmentedDeadlockPrevention(int num_res, int num_proc);
    
    // Getter methods
    const std::vector<int>& get_available() const { return available; }
    const std::vector<std::vector<int>>& get_allocated() const { return allocated; }
    const std::vector<std::vector<int>>& get_max_need() const { return max_need; }
    
    // Setter methods
    void set_available(const std::vector<int>& resources) { available = resources; }
    void set_max_need(const std::vector<std::vector<int>>& need) { max_need = need; }
    
    // Resource management methods
    void allocate_resources(int process_id, const std::vector<int>& resources);
    void release_resources(int process_id, const std::vector<int>& resources);
    
    double predict_deadlock_risk(int process_id, const std::vector<int>& requested_resources);
    bool ml_augmented_bankers_check(int process_id, const std::vector<int>& requested_resources);
    void update_rag(int process_id, int resource_id);
    std::vector<std::vector<int>> detect_cycles();
    bool ml_augmented_wait_die(int requesting_process, int holding_process, 
                              const std::unordered_map<int, double>& timestamp);
    void train_risk_model();
    void add_training_example(const std::vector<double>& features, bool led_to_deadlock);
    void save_model(const std::string& filename);
    void load_model(const std::string& filename);
};

// Add these before the DeadlockDetector class
enum class Action {
    ALLOCATE,
    WAIT,
    RELEASE
};

struct State {
    std::vector<int> available_resources;
    std::vector<std::vector<int>> allocated_resources;
    // Add any other state information you need
};

class QlearningAgent {
public:
    Action get_best_action(const State& state);
    void update_q_values(const State& state, Action action, double reward, const State& next_state);
    void reset();
    void save_model(const std::string& filename);
private:
    // Add Q-learning implementation details
};

class DeadlockDetector {
public:
    // ... existing code ...

    Action get_best_action(const State& state) {
        return rl_agent.get_best_action(state);
    }
    
    void update_q_values(const State& state, Action action, double reward, const State& next_state) {
        rl_agent.update_q_values(state, action, reward, next_state);
    }
    
    void reset() {
        rl_agent.reset();
    }

    // Add public method to save model
    void save_model(const std::string& filename) {
        rl_agent.save_model(filename);
    }

private:
    QlearningAgent rl_agent;
    // ... other private members ...
};

inline std::string action_to_string(Action action) {
    switch(action) {
        case Action::ALLOCATE: return "ALLOCATE";
        case Action::WAIT: return "WAIT";
        case Action::RELEASE: return "RELEASE";
        default: return "UNKNOWN";
    }
}

#endif 