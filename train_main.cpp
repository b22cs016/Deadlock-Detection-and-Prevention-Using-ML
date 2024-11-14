#include "deadlock_prevention.hpp"
#include "deadlock_trainer.cpp"
#include <signal.h>

int main() {
    // Register signal handler for Ctrl+C
    signal(SIGINT, signal_handler);
    
    // Initialize prevention system
    MLAugmentedDeadlockPrevention prevention(3, 5);// 3 resources, 5 processes 
    
    // Set initial state
    std::vector<int> initial_resources = {10, 5, 7};// 10 units of resource 0, 5 units of resource 1, 7 units of resource 2
    prevention.set_available(initial_resources);
    
    std::vector<std::vector<int>> max_needs = {
        {7, 5, 3},
        {3, 2, 2},
        {9, 0, 2},
        {2, 2, 2},
        {4, 3, 3}
    };
    prevention.set_max_need(max_needs);
    
    // Create and run trainer
    DeadlockTrainer trainer(prevention);
    trainer.train_continuously();
    
    return 0;
} 