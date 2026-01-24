#include <signal.h>
#include <sys/poll.h>
#include <chrono>
#include <iostream>
#include <string>
#include "ida100.h"

uint64_t LOG_INTERVAL_MS = 1000;


int exit_signo = -1;

void exitHandler(int signo) { exit_signo = signo; }

void registerSignalHandlers() {
    struct sigaction sg;
    sg.sa_handler = exitHandler;
    sigemptyset(&sg.sa_mask);
    sg.sa_flags = 0;
    sigaction(SIGINT, &sg, nullptr);
    sigaction(SIGTERM, &sg, nullptr);
}

uint64_t micros() {
    return std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0]
                  << " <serial number> " << std::endl;
        exit(1);
    }
    
    char* serialNumber = argv[1];
    registerSignalHandlers();
    
    IDA100 loadCell(serialNumber);
    
    // in microseconds
    uint64_t lastLogTime = 0;
    
    while (exit_signo == -1) {
        uint64_t currentTime = micros();
               
        double reading = loadCell.read(); 
    
        std::cout << "rec: " << currentTime << " " << reading << std::endl;
        std::cout.flush();  
        
    }
    
    std::cerr << "Received signal " << exit_signo << ", exiting..." << std::endl;

    return 0;
}