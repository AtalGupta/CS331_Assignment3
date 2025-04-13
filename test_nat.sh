#!/bin/bash

OUTPUT_DIR="./test_results"
mkdir -p "$OUTPUT_DIR"

cleanup() {
    echo "Cleaning up..."
    sudo mn -c > /dev/null 2>&1
    sudo pkill -f tcpdump
    sudo pkill -f iperf3
}
trap cleanup EXIT

run_test() {
    test_name=$1
    cmd=$2
    log_file="$OUTPUT_DIR/${test_name}.log"
    
    echo "===== Running $test_name =====" | tee "$log_file"
    eval "$cmd" 2>&1 | tee -a "$log_file"
    echo -e "\nTest $test_name completed\n" | tee -a "$log_file"
}

collect_metrics() {
    # Collect NAT rules and connections
    run_test "nat_rules" "sudo python3 nat_topology.py h9 iptables -t nat -L -vn"
    run_test "conntrack" "sudo python3 nat_topology.py h9 conntrack -L"
    
    # Collect interface configurations
    for host in h1 h2 h5 h8 h9; do
        run_test "${host}_config" "sudo python3 nat_topology.py $host ifconfig; $host route -n"
    done
}

run_ping_test() {
    src=$1; dst=$2; dst_ip=$3; test_name=$4
    for i in {1..3}; do
        log_file="$OUTPUT_DIR/${test_name}_run${i}.log"
        echo "===== Test Run $i =====" | tee "$log_file"
        
        # Run ping with timestamp
        (echo "PING TEST: $src -> $dst ($dst_ip)"; \
         sudo python3 nat_topology.py $src ping -c 4 -W 1 $dst_ip) | tee -a "$log_file"
        
        # Calculate theoretical delay
        echo -e "\nTheoretical Delay Calculation:" | tee -a "$log_file"
        echo "Path: $src -> h9 -> $dst (each way)" | tee -a "$log_file"
        echo "Total RTT: 2*(5ms + 5ms + 7ms*2 + 5ms) = 56ms" | tee -a "$log_file"
        
        # Extract metrics
        grep -E 'packets transmitted|rtt min' "$log_file" | tee -a "$log_file"
        echo -e "\n--------------------------------\n" | tee -a "$log_file"
    done
}

run_iperf_test() {
    server=$1; client=$2; s_ip=$3; test_name=$4
    for i in {1..3}; do
        log_file="$OUTPUT_DIR/${test_name}_run${i}.log"
        echo "===== Test Run $i =====" | tee "$log_file"
        
        # Start server
        sudo python3 nat_topology.py $server iperf3 -s -p 5201 -D | tee -a "$log_file"
        sleep 2
        
        # Run client
        (echo "IPERF TEST: $client -> $server ($s_ip)"; \
         sudo python3 nat_topology.py $client iperf3 -c $s_ip -p 5201 -t 10 -J) | tee -a "$log_file"
        
        # Kill server
        sudo python3 nat_topology.py $server pkill -f iperf3 | tee -a "$log_file"
        
        # Extract metrics
        jq '.end.sum_sent.bits_per_second, .end.sum_received.bits_per_second' "$log_file" | \
        awk '{printf "Bandwidth: %.2f Mbps\n", $1/1e6}' | tee -a "$log_file"
        echo -e "\n--------------------------------\n" | tee -a "$log_file"
    done
}

# Q2a Tests: Internal to External
collect_metrics
run_ping_test h1 h5 10.0.0.6 "q2ai_ping"
run_ping_test h2 h3 10.0.0.4 "q2aii_ping"

# Q2b Tests: External to Internal
run_ping_test h8 h1 172.16.10.10 "q2bi_ping"
run_ping_test h6 h2 172.16.10.10 "q2bii_ping"

# Q2c Tests: Iperf
run_iperf_test h1 h6 172.16.10.10 "q2ci_iperf"
run_iperf_test h8 h2 10.1.1.3 "q2cii_iperf"

echo "All tests completed. Results saved to $OUTPUT_DIR"