#include "deadlock_prevention.hpp"
#include <iostream>

double SimpleNeuralNetwork::predict(const std::vector<double>& input) {
    // Forward propagation
    std::vector<double> hidden(bias1.size());
    
    // Hidden layer
    for(size_t i = 0; i < hidden.size(); i++) {
        hidden[i] = bias1[i];
        for(size_t j = 0; j < input.size(); j++) {
            hidden[i] += input[j] * weights1[j][i];
        }
        hidden[i] = sigmoid(hidden[i]);
    }
    
    // Output layer
    double output = bias2[0];
    for(size_t i = 0; i < hidden.size(); i++) {
        output += hidden[i] * weights2[i][0];
    }
    
    return sigmoid(output);
}

void SimpleNeuralNetwork::train(const std::vector<std::vector<double>>& X, const std::vector<double>& y) {
    std::cout << "Starting neural network training with " << X.size() << " examples\n";
    // Simple stochastic gradient descent
    double learning_rate = 0.1;
    
    for(size_t sample = 0; sample < X.size(); sample++) {
        const auto& input = X[sample];
        double target = y[sample];
        
        // Forward propagation
        std::vector<double> hidden(bias1.size());
        
        // Hidden layer
        for(size_t i = 0; i < hidden.size(); i++) {
            hidden[i] = bias1[i];
            for(size_t j = 0; j < input.size(); j++) {
                hidden[i] += input[j] * weights1[j][i];
            }
            hidden[i] = sigmoid(hidden[i]);
        }
        
        // Output layer
        double output = bias2[0];
        for(size_t i = 0; i < hidden.size(); i++) {
            output += hidden[i] * weights2[i][0];
        }
        output = sigmoid(output);
        
        // Backpropagation
        double output_error = output - target;
        double output_delta = output_error * output * (1 - output);
        
        // Update output layer
        bias2[0] -= learning_rate * output_delta;
        for(size_t i = 0; i < hidden.size(); i++) {
            weights2[i][0] -= learning_rate * output_delta * hidden[i];
        }
        
        // Update hidden layer
        for(size_t i = 0; i < hidden.size(); i++) {
            double hidden_error = weights2[i][0] * output_delta;
            double hidden_delta = hidden_error * hidden[i] * (1 - hidden[i]);
            
            bias1[i] -= learning_rate * hidden_delta;
            for(size_t j = 0; j < input.size(); j++) {
                weights1[j][i] -= learning_rate * hidden_delta * input[j];
            }
        }
    }
}

MLAugmentedDeadlockPrevention::MLAugmentedDeadlockPrevention(int num_res, int num_proc)
    : num_resources(num_res), 
      num_processes(num_proc),
      risk_model(num_res * num_proc + num_res, 10) // Input size = resources*processes + available resources, hidden size = 10
{
    available.resize(num_resources, 0);
    allocated.resize(num_processes, std::vector<int>(num_resources, 0));
    max_need.resize(num_processes, std::vector<int>(num_resources, 0));
}

void MLAugmentedDeadlockPrevention::allocate_resources(int process_id, const std::vector<int>& resources) {
    for(int i = 0; i < num_resources; i++) {
        available[i] -= resources[i];
        allocated[process_id][i] += resources[i];
    }
}

void MLAugmentedDeadlockPrevention::release_resources(int process_id, const std::vector<int>& resources) {
    for(int i = 0; i < num_resources; i++) {
        available[i] += resources[i];
        allocated[process_id][i] -= resources[i];
    }
}

bool MLAugmentedDeadlockPrevention::ml_augmented_bankers_check(int process_id, const std::vector<int>& requested_resources) {
    // First check if the request is safe according to traditional Banker's algorithm
    bool traditional_safe = is_safe_state(process_id, requested_resources);
    
    // Get ML prediction
    double risk = predict_deadlock_risk(process_id, requested_resources);
    
    // Combine both decisions (you can adjust the threshold)
    return traditional_safe && (risk < 0.5);
}

void MLAugmentedDeadlockPrevention::update_rag(int process_id, int resource_id) {
    rag[process_id].insert(resource_id);
}

std::vector<std::vector<int>> MLAugmentedDeadlockPrevention::detect_cycles() {
    std::vector<std::vector<int>> cycles;
    std::vector<bool> visited(num_processes, false);
    std::vector<int> path;
    
    // Simple DFS to detect cycles
    std::function<void(int)> dfs = [&](int node) {
        visited[node] = true;
        path.push_back(node);
        
        for(int next : rag[node]) {
            if(!visited[next]) {
                dfs(next);
            } else {
                // Found a cycle
                std::vector<int> cycle;
                auto it = std::find(path.begin(), path.end(), next);
                while(it != path.end()) {
                    cycle.push_back(*it);
                    ++it;
                }
                if(!cycle.empty()) {
                    cycles.push_back(cycle);
                }
            }
        }
        
        path.pop_back();
        visited[node] = false;
    };
    
    for(int i = 0; i < num_processes; i++) {
        if(!visited[i]) {
            dfs(i);
        }
    }
    
    return cycles;
}

bool MLAugmentedDeadlockPrevention::ml_augmented_wait_die(int requesting_process, int holding_process,
                                                         const std::unordered_map<int, double>& timestamp) {
    // Traditional Wait-Die logic
    bool should_wait = timestamp.at(requesting_process) < timestamp.at(holding_process);
    
    // Get ML prediction for deadlock risk
    std::vector<int> dummy_request(num_resources, 1); // Simplified for example
    double risk = predict_deadlock_risk(requesting_process, dummy_request);
    
    // Combine both decisions (you can adjust the threshold)
    return should_wait && (risk < 0.7);
}

double MLAugmentedDeadlockPrevention::predict_deadlock_risk(int process_id, const std::vector<int>& requested_resources) {
    // Create feature vector
    std::vector<double> features;
    
    // Add current allocation state
    for(const auto& proc_alloc : allocated) {
        features.insert(features.end(), proc_alloc.begin(), proc_alloc.end());
    }
    
    // Add available resources
    features.insert(features.end(), available.begin(), available.end());
    
    // Make prediction
    double prediction = risk_model.predict(features);
    std::cout << "Deadlock risk prediction for process " << process_id << ": " << prediction << std::endl;
    return prediction;
}

void MLAugmentedDeadlockPrevention::add_training_example(const std::vector<double>& features, bool led_to_deadlock) {
    history.push_back({features, led_to_deadlock});
}

void MLAugmentedDeadlockPrevention::train_risk_model() {
    if(history.empty()) return;
    
    std::vector<std::vector<double>> X;
    std::vector<double> y;
    
    for(const auto& example : history) {
        X.push_back(example.features);
        y.push_back(example.led_to_deadlock ? 1.0 : 0.0);
    }
    
    risk_model.train(X, y);
}

void MLAugmentedDeadlockPrevention::save_model(const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if(file.is_open()) {
        // Save model parameters
        // This is a placeholder - implement actual model saving logic
        file.close();
    }
}

bool MLAugmentedDeadlockPrevention::is_safe_state(int process_id, const std::vector<int>& requested) {
    std::vector<int> work = available;
    std::vector<std::vector<int>> temp_allocated = allocated;
    
    // Simulate allocation
    for(int i = 0; i < num_resources; i++) {
        if(requested[i] > work[i]) return false;
        work[i] -= requested[i];
        temp_allocated[process_id][i] += requested[i];
    }
    
    return can_complete(process_id, work, temp_allocated);
}

bool MLAugmentedDeadlockPrevention::can_complete(int process_id, std::vector<int>& work, 
                                                const std::vector<std::vector<int>>& allocated) {
    std::vector<bool> finished(num_processes, false);
    int count = 0;
    
    while(count < num_processes) {
        bool found = false;
        
        for(int i = 0; i < num_processes; i++) {
            if(!finished[i]) {
                bool can_allocate = true;
                for(int j = 0; j < num_resources; j++) {
                    if(max_need[i][j] - allocated[i][j] > work[j]) {
                        can_allocate = false;
                        break;
                    }
                }
                
                if(can_allocate) {
                    for(int j = 0; j < num_resources; j++) {
                        work[j] += allocated[i][j];
                    }
                    finished[i] = true;
                    count++;
                    found = true;
                }
            }
        }
        
        if(!found) break;
    }
    
    return count == num_processes;
}

void MLAugmentedDeadlockPrevention::load_model(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if(file.is_open()) {
        // Implement model loading logic here
        file.close();
    }
} 