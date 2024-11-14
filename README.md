# Steps to Run in Linux Distro

## Files to Download
- deadlock_prevention.hpp
- deadlock_trainer.cpp
- main.cpp
- deadlock_prevention.cpp
- train_main.cpp

## Compilation and Execution Steps

1. Compile the Training Program:  
   `g++ -std=c++17 -Wall -O3 train_main.cpp deadlock_prevention.cpp -o trainer`

2. Compile the Deadlock Test Program:  
   `g++ -std=c++17 -Wall -O3 main.cpp deadlock_prevention.cpp -o deadlock_test`

3. Run the Trainer:  
   `./trainer`

4. Run the Deadlock Test:  
   `./deadlock_test`
