# Deadlock-Detection-and-Prevention-Using-ML

STEPS TO RUN IN LINUX DISTRO  : 
Download deadlock_prevention.hpp, deadlock_trainer.cpp, main.cpp, deadlock_prevention.cpp and train_main.cpp
step 1: g++ -std=c++17 -Wall -O3 train_main.cpp deadlock_prevention.cpp -o trainer
step 2: g++ -std=c++17 -Wall -O3 main.cpp deadlock_prevention.cpp -o deadlock_test
step 3: ./trainer
step 4:  ./deadlock_test
