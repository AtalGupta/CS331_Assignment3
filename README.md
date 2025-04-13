#CS 331 Assignment-3

## Q1

This project sets up a custom network topology using Mininet with 4 switches and 8 hosts, each connected with specific latencies. It includes automated ping tests to evaluate connectivity and delay between various nodes.

---

### 🔧 Setup Instructions

Follow the steps below to install Mininet and run the simulation script.

### 1. Clone the Mininet Repository
```bash
git clone https://github.com/mininet/mininet
cd mininet
git checkout -b 2.3.0
./util/install.sh -a
```
### Make the script executable
```bash
chmod +x q1_a.py
```
### Run the script using sudo
```bash
sudo ./q1_a.py
```
## Q2
For running 
```bash
sudo python3 nat_topology.py
```

## Q3
Command to run is 

```bash
gcc distance_vector.c node0.c node1.c node2.c node3.c 
sudo ./dvrouting
```

This will give the output that I have given in the output.txt file which is printing the updated  distance table for the network 
