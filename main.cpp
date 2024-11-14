#include "deadlock_prevention.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

void print_state(const MLAugmentedDeadlockPrevention& prevention) {
    std::cout << "\nCurrent System State:\n";
    std::cout << "Available Resources: ";
    for(int r : prevention.get_available()) {
        std::cout << r << " ";
    }
    std::cout << "\n\nAllocated Resources:\n";
    const auto& allocated = prevention.get_allocated();
    for(size_t i = 0; i < allocated.size(); i++) {
        std::cout << "Process " << i << ": ";
        for(int r : allocated[i]) {
            std::cout << r << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    std::cout << "Initializing deadlock prevention system...\n";
    // Initialize the system with 3 resources and 5 processes
    MLAugmentedDeadlockPrevention prevention(3, 5);
    
    // Set initial available resources
    std::vector<int> initial_resources = {10, 5, 7};
    prevention.set_available(initial_resources);
    
    // Set maximum needs for processes
    std::vector<std::vector<int>> max_needs = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    prevention.set_max_need(max_needs);
    
    // Initialize timestamps for Wait-Die scheme
    std::unordered_map<int, double> timestamps;
    for(int i = 0; i < 5; i++) {
        timestamps[i] = i * 1.0;  // Simple increasing timestamps
    }
    
    print_state(prevention);
    
    // Test 1: Banker's Algorithm with ML
    std::cout << "\n=== Test 1: ML-augmented Banker's Algorithm ===\n";
    int process_id = 0;
    std::vector<int> requested_resources = {1, 0, 2};
    bool is_safe = prevention.ml_augmented_bankers_check(process_id, requested_resources);
    
    std::cout << "Requesting resources for process " << process_id << ": ";
    for(int r : requested_resources) {
        std::cout << r << " ";
    }
    std::cout << "\n";
    
    if(is_safe) {
        std::cout << "Request is safe to grant - allocating resources\n";
        prevention.allocate_resources(process_id, requested_resources);
    } else {
        std::cout << "Request denied - would lead to unsafe state\n";
    }
    
    print_state(prevention);
    
    // Test 2: Wait-Die scheme with ML
    std::cout << "\n=== Test 2: ML-augmented Wait-Die scheme ===\n";
    int requesting_process = 1;
    int holding_process = 0;
    bool should_wait = prevention.ml_augmented_wait_die(requesting_process, holding_process, timestamps);
    std::cout << "Process " << requesting_process << " should " 
              << (should_wait ? "wait" : "be aborted") << "\n";
    
    // Test 3: Resource Allocation Graph
    std::cout << "\n=== Test 3: Resource Allocation Graph ===\n";
    std::cout << "Adding edges to RAG: 0->1, 1->2, 2->0\n";
    prevention.update_rag(0, 1);
    prevention.update_rag(1, 2);
    prevention.update_rag(2, 0);
    
    auto cycles = prevention.detect_cycles();
    if(!cycles.empty()) {
        std::cout << "Detected cycles in RAG:\n";
        for(const auto& cycle : cycles) {
            for(int node : cycle) {
                std::cout << node << " -> ";
            }
            std::cout << cycle[0] << "\n";
        }
    }
    
    // Test 4: Training the ML model
    std::cout << "\n=== Test 4: ML Model Training ===\n";
    std::cout << "Training features: ";
    std::vector<double> sample_features = {5, 3, 4, 2, 1, 1, 1, 0, 2};
    for(double f : sample_features) {
        std::cout << f << " ";
    }
    std::cout << "\nLabel: false (no deadlock)\n";
    
    prevention.add_training_example(sample_features, false);
    prevention.train_risk_model();
    std::cout << "Training completed\n";
    std::cout << "Saving model to 'learned_policy.dat'\n";
    
    prevention.save_model("learned_policy.dat");
    
    return 0;
} 