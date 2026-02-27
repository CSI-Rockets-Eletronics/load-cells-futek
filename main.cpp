#include <signal.h>
#include <sys/poll.h>
#include <chrono>
#include <string>
#include <iomanip>
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
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0]
                  << " <serial1> <ticks_per_pound1> <serial2> <ticks_per_pound2>" << std::endl;
        exit(1);
    }

    char* serial1 = argv[1];
    double tpp1 = std::stod(argv[2]);
    char* serial2 = argv[3];
    double tpp2 = std::stod(argv[4]);

    registerSignalHandlers();

    IDA100 lc1(serial1);
    IDA100 lc2(serial2);

    // print header
    std::cerr << std::left
              << std::setw(20) << "Timestamp (us)"
              << std::setw(20) << "LoadCell1 (lbs)"
              << std::setw(20) << "LoadCell2 (lbs)"
              << std::endl;

    uint64_t lastLogTime = 0;
    while (exit_signo == -1) {
        uint64_t timestamp = micros();
        double lbs1 = lc1.read() / tpp1;
        double lbs2 = lc2.read() / tpp2;

        std::cout << "rec1: " << timestamp << " " << lbs1 << std::endl;
        std::cout << "rec2: " << timestamp << " " << lbs2 << std::endl;

        if (timestamp - lastLogTime > LOG_INTERVAL_MS * 1000) {
            std::cerr << std::left << std::fixed << std::setprecision(2)
                      << std::setw(20) << timestamp
                      << std::setw(20) << lbs1
                      << std::setw(20) << lbs2
                      << std::endl;
            lastLogTime = timestamp;
        }
    }

    std::cerr << "Received signal " << exit_signo << ", exiting..." << std::endl;
}