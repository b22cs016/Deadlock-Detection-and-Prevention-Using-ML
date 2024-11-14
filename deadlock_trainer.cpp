#include "deadlock_prevention.hpp"
#include <chrono>
#include <signal.h>
#include <random>
#include <fstream>
#include <iostream>

volatile sig_atomic_t g_running = 1;

void signal_handler(int) {
    g_running = 0;
    std::cout << "\nReceived stop signal. Finishing current batch and saving model...\n";
}

class DeadlockTrainer {
private:
    MLAugmentedDeadlockPrevention& prevention;
    std::mt19937 rng;
    const unsigned long CHECKPOINT_INTERVAL = 10000; // Save every 10k scenarios
    
    // Generate random resource request
    std::vector<int> generate_random_request(int max_resources) {
        std::vector<int> request(prevention.get_available().size());
        for(size_t i = 0; i < request.size(); i++) {
            request[i] = rng() % (max_resources + 1);
        }
        return request;
    }

    // Simulate deadlock scenario
    bool simulate_scenario() {
        int num_processes = prevention.get_allocated().size();
        bool deadlock_detected = false;
        
        // Try allocation for multiple processes
        for(int i = 0; i < num_processes; i++) {
            int process_id = rng() % num_processes;
            
            // Generate random resource request
            auto request = generate_random_request(5);
            
            // Try allocation
            bool was_safe = prevention.ml_augmented_bankers_check(process_id, request);
            if(was_safe) {
                prevention.allocate_resources(process_id, request);
                
                // Print allocation info
                std::cout << "Process " << process_id << " allocated resources: ";
                for(int r : request) {
                    std::cout << r << " ";
                }
                std::cout << "\n";
            }
            
            // Randomly release some resources
            if(rng() % 2 == 0) {
                auto release = generate_random_request(3);
                prevention.release_resources(process_id, release);
                
                std::cout << "Process " << process_id << " released resources: ";
                for(int r : release) {
                    std::cout << r << " ";
                }
                std::cout << "\n";
            }
        }
        
        // Check for deadlock using RAG
        auto cycles = prevention.detect_cycles();
        deadlock_detected = !cycles.empty();
        
        if(deadlock_detected) {
            std::cout << "Deadlock detected between processes: ";
            for(const auto& cycle : cycles) {
                for(int node : cycle) {
                    std::cout << node << " ";
                }
            }
            std::cout << "\n";
        }
        
        return deadlock_detected;
    }

    void save_checkpoint(unsigned long scenarios_count) {
        std::string checkpoint_file = "model_checkpoint_" + 
                                    std::to_string(scenarios_count) + ".dat";
        prevention.save_model(checkpoint_file);
        std::cout << "Checkpoint saved to " << checkpoint_file << std::endl;
    }

public:
    DeadlockTrainer(MLAugmentedDeadlockPrevention& prev) 
        : prevention(prev), rng(std::random_device{}()) {}

    void train_continuously() {
        std::cout << "Starting continuous training. Press Ctrl+C to stop and save model.\n"
                  << "Checkpoints will be saved every " << CHECKPOINT_INTERVAL 
                  << " scenarios.\n";
        
        unsigned long scenarios_count = 0;
        auto start_time = std::chrono::steady_clock::now();
        
        while(g_running) {
            // Generate training scenario
            std::vector<double> features;
            for(const auto& alloc : prevention.get_allocated()) {
                features.insert(features.end(), alloc.begin(), alloc.end());
            }
            features.insert(features.end(), 
                          prevention.get_available().begin(), 
                          prevention.get_available().end());
            
            bool led_to_deadlock = simulate_scenario();
            prevention.add_training_example(features, led_to_deadlock);
            
            // Add checkpoint saving
            if(scenarios_count % CHECKPOINT_INTERVAL == 0) {
                save_checkpoint(scenarios_count);
            }

            // Existing periodic training
            if(scenarios_count % 1000 == 0) {
                prevention.train_risk_model();
                
                // Print current system state
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
                
                auto current_time = std::chrono::steady_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::minutes>
                              (current_time - start_time);
                
                std::cout << "Trained on " << scenarios_count << " scenarios. "
                         << "Running time: " << duration.count() << " minutes\n"
                         << "Current deadlock detection accuracy: " 
                         << calculate_accuracy() << "%\n";
            }
        }
        
        // Final training and save
        prevention.train_risk_model();
        prevention.save_model("final_model.dat");
        
        auto end_time = std::chrono::steady_clock::now();
        auto total_duration = std::chrono::duration_cast<std::chrono::minutes>
                            (end_time - start_time);
        
        std::cout << "\nTraining completed:\n"
                  << "Total scenarios: " << scenarios_count << "\n"
                  << "Total time: " << total_duration.count() << " minutes\n"
                  << "Model saved to 'final_model.dat'\n";
    }

    double calculate_accuracy() {
        int correct_predictions = 0;
        const int TEST_CASES = 100;
        
        // Get number of processes from the prevention object
        int num_processes = prevention.get_allocated().size();
        
        for(int i = 0; i < TEST_CASES; i++) {
            // Generate a realistic test case
            int process_id = rng() % num_processes;  // Using rng instead of rand()
            auto test_request = generate_random_request(5);
            
            // Use ml_augmented_bankers_check instead of private is_safe_state
            bool bankers_safe = prevention.ml_augmented_bankers_check(process_id, test_request);
            
            // Get ML prediction
            double predicted_risk = prevention.predict_deadlock_risk(process_id, test_request);
            
            // Compare ML prediction with Banker's algorithm
            bool ml_safe = predicted_risk < 0.5;
            if(ml_safe == bankers_safe) {
                correct_predictions++;
            }
            
            // Log disagreements for analysis
            if(ml_safe != bankers_safe) {
                std::cout << "ML disagreed with Banker's Algorithm:\n"
                         << "Process: " << process_id << "\n"
                         << "Request: ";
                for(int r : test_request) std::cout << r << " ";
                std::cout << "\nML risk: " << predicted_risk 
                         << "\nBanker's: " << (bankers_safe ? "safe" : "unsafe")
                         << "\n\n";
            }
        }
        
        return (correct_predictions * 100.0) / TEST_CASES;
    }
}; 