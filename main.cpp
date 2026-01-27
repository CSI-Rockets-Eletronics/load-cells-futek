#include <signal.h>
#include <sys/poll.h>

#include <chrono>
#include <string>

#include <iomanip>
#include <sstream> 

#include "ida100.h"

// interval for logging reading to stderr
uint64_t LOG_INTERVAL_MS = 1000;

// -1 if no signal received
int exit_signo = -1;

std::string getEasternTime(){
	using namespace std::chrono;

	auto now = system_clock::now();
	auto now_time_t = system_clock::to_time_t(now);
	auto microseconds = duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000000;

	std::tm* local_tm = std::localtime(&now_time_t);

	std::ostringstream oss;
	oss << std::put_time(local_tm, "%H:%M:%S");
	oss << "." << std::setfill('0') << std::setw(6) << microseconds.count();

	return oss.str();
}

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

void checkForCalibration(IDA100& loadCell) {
    struct pollfd fds;
    fds.fd = 0;  // stdin
    fds.events = POLLIN;

    if (poll(&fds, 1, 0)) {
        std::string message;
        std::cin >> message;

        if (message == "calibrate") {
            loadCell.calibrate();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <serial number> <ticks per pound>" << std::endl;
        exit(1);
    }

    char* serialNumber = argv[1];
    double ticksPerPound = std::stod(argv[2]);

    registerSignalHandlers();

    IDA100 loadCell(serialNumber);

    // in microseconds
    uint64_t lastLogTime = 0;

    while (exit_signo == -1) {
        checkForCalibration(loadCell);

        uint64_t timestamp = micros();
	std::string easternTime = getEasternTime();
        double lbs = loadCell.read() / ticksPerPound;

        //std::cout << "rec: " << timestamp << " " << lbs << std::endl;
        std::cerr << "rec: " << easternTime << " " << lbs << std::endl; 

        if (timestamp - lastLogTime > LOG_INTERVAL_MS * 1000) {
            std::cerr << "rec: " << easternTime << " " << lbs << std::endl;
           // std::cerr << "rec: " << timestamp << " " << lbs << std::endl;
            lastLogTime = timestamp;
        }
    }

    std::cerr << "Received signal " << exit_signo << ", exiting..."
              << std::endl;
}
