#!/usr/bin/env python3
"""
Simple script to log data from two FUTEK load cells to a CSV file.
Runs the compiled C++ main program for each load cell and captures output.
"""

import subprocess
import csv
import sys
import time
from datetime import datetime
import threading

# Load cell configurations (serial number, ticks per pound)
LOAD_CELLS = [
    {"name": "LoadCell1", "serial": "1076702", "ticks_per_lb": 772},
    {"name": "LoadCell2", "serial": "652964", "ticks_per_lb": 3616},
]

# CSV output file
CSV_FILE = f"loadcell_data_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"


class LoadCellLogger:
    def __init__(self, name, serial, ticks_per_lb, csv_writer, csv_lock):
        self.name = name
        self.serial = serial
        self.ticks_per_lb = ticks_per_lb
        self.csv_writer = csv_writer
        self.csv_lock = csv_lock
        self.process = None
        self.running = False

    def start(self):
        """Start the C++ main program for this load cell"""
        try:
            self.process = subprocess.Popen(
                ["./main", self.serial, str(self.ticks_per_lb)],
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                text=True,
                bufsize=1,
            )
            self.running = True
            print(f"Started {self.name} (serial: {self.serial})", file=sys.stderr)
        except FileNotFoundError:
            print(
                "Error: './main' executable not found. Run 'make' first.",
                file=sys.stderr,
            )
            sys.exit(1)
        except Exception as e:
            print(f"Error starting {self.name}: {e}", file=sys.stderr)
            sys.exit(1)

    def read_and_log(self):
        """Read output from the process and log to CSV"""
        try:
            for line in self.process.stdout:
                line = line.strip()
                if not line.startswith("rec:"):
                    continue

                # Parse: "rec: <timestamp> <lbs>"
                parts = line.split()
                if len(parts) != 3:
                    continue

                timestamp_us = int(parts[1])
                lbs = float(parts[2])

                # Convert timestamp to readable format
                timestamp_s = timestamp_us / 1_000_000
                dt = datetime.fromtimestamp(timestamp_s)

                # Write to CSV (thread-safe)
                with self.csv_lock:
                    self.csv_writer.writerow(
                        {
                            "timestamp_us": timestamp_us,
                            "datetime": dt.strftime("%Y-%m-%d %H:%M:%S.%f"),
                            "load_cell": self.name,
                            "force_lbs": lbs,
                        }
                    )
                    
                print(f"{self.name}: {dt.strftime('%H:%M:%S.%f')[:-3]} - {lbs:.3f} lbs")

        except Exception as e:
            print(f"Error reading from {self.name}: {e}", file=sys.stderr)
        finally:
            self.running = False

    def stop(self):
        """Stop the process"""
        if self.process:
            self.process.terminate()
            self.process.wait(timeout=5)
            print(f"Stopped {self.name}", file=sys.stderr)


def main():
    print(f"Logging load cell data to: {CSV_FILE}", file=sys.stderr)
    print("Press Ctrl+C to stop logging", file=sys.stderr)

    # Open CSV file
    csv_file = open(CSV_FILE, "w", newline="")
    csv_writer = csv.DictWriter(
        csv_file,
        fieldnames=["timestamp_us", "datetime", "load_cell", "force_lbs"],
    )
    csv_writer.writeheader()
    csv_lock = threading.Lock()

    # Create loggers for each load cell
    loggers = []
    threads = []

    for lc_config in LOAD_CELLS:
        logger = LoadCellLogger(
            lc_config["name"],
            lc_config["serial"],
            lc_config["ticks_per_lb"],
            csv_writer,
            csv_lock,
        )
        loggers.append(logger)

    try:
        # Start all load cells
        for logger in loggers:
            logger.start()

        # Start reading threads
        for logger in loggers:
            thread = threading.Thread(target=logger.read_and_log, daemon=True)
            thread.start()
            threads.append(thread)

        # Wait for threads (or Ctrl+C)
        for thread in threads:
            thread.join()

    except KeyboardInterrupt:
        print("\nReceived Ctrl+C, stopping...", file=sys.stderr)

    finally:
        # Stop all loggers
        for logger in loggers:
            logger.stop()

        # Close CSV file
        csv_file.close()
        print(f"Data saved to: {CSV_FILE}", file=sys.stderr)


if __name__ == "__main__":
    main()
