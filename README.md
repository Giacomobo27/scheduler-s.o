# CPU Scheduling Simulator

This project is a simulation tool designed to demonstrate CPU scheduling techniques in a multiprocessor environment. It is a scheduling simulator that has these following features:

1. Support for multiple (configurable) CPUs.
2. Implementation of preemptive Shortest Job First (SJF) scheduling using a quantum prediction formula:
   \[
   q(t+1) = a \times q_{\text{current}} + (1 - a) \times q(t)
   \]

---

## Features

- **Multiple CPU Support**: The simulator can manage and schedule tasks across multiple CPUs simultaneously, allowing exploration of parallel processing techniques.
- **Preemptive Shortest Job First (SJF)**: Implements SJF scheduling in a preemptive manner, ensuring optimal CPU utilization and fairness among processes.
- **Quantum Prediction**: Employs an advanced formula to dynamically predict the CPU quantum for processes, adapting based on historical data.

---

## Quantum Prediction Formula

The quantum prediction is computed using an exponential smoothing formula:
\[
q(t+1) = a \times q_{\text{current}} + (1 - a) \times q(t)
\]

- **`a`**: Decay coefficient (configurable) that determines the weight of recent versus historical quantum values.
- **`q(t)`**: Previous quantum prediction.
- **`q(t+1)`**: Updated quantum prediction for the next CPU burst.

---

## How It Works

1. **Process Initialization**:
   - Processes are initialized with either random or user-defined CPU bursts and I/O patterns.

2. **Scheduling**:
   - The scheduler assigns processes to CPUs using the Shortest Job First (SJF) algorithm with quantum prediction.

3. **Preemption**:
   - A process is preempted if a new process with a shorter predicted burst time arrives, ensuring optimal CPU utilization.

4. **Results**:
   - The simulation calculates and displays the results of the allocation of the rescources.
