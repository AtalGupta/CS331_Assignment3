#!/bin/bash

# NAT Testing Script
echo "Starting NAT configuration tests..."

# Output directory
OUTPUT_DIR="/tmp/q2_results"
mkdir -p "$OUTPUT_DIR"

# Function to run and log ping tests
run_ping_test() {
    local source=$1
    local dest=$2
    local dest_ip=$3
    local test_name=$4
    local log_file="${OUTPUT_DIR}/ping_${source}_to_${dest}.log"
    local pcap_file="${OUTPUT_DIR}/ping_${source}_to_${dest}.pcap"
    
    echo "Running ping test: $test_name ($source to $dest)"
    echo "Test: $test_name ($source to $dest)" > "$log_file"
    
    # Run 3 tests with 30 second intervals
    for i in {1..3}; do
        echo "Test run $i" >> "$log_file"
        sudo mn -c > /dev/null 2>&1  # Clean up previous mininet
        
        # Start packet capture
        echo "Starting tcpdump on h9..."
        sudo python3 q2.py <<EOF &
h9 tcpdump -i h9-eth1 -n icmp -w $pcap_file
EOF
        sleep 2  # Wait for tcpdump to start
        
        # Run the test
        sudo python3 q2.py <<EOF
h9 iptables -t nat -L -v -n >> $log_file
${source} ping -c 4 ${dest_ip} >> $log_file 2>&1
h9 iptables -t nat -L -v -n >> $log_file
h9 pkill tcpdump
exit
EOF
        
        echo "Waiting 30 seconds before next test..."
        sleep 30
    done
    
    echo "Completed ping test: $source to $dest"
}

# Function to run iperf tests
run_iperf_test() {
    local server=$1
    local server_ip=$2
    local client=$3
    local client_ip=$4
    local test_name=$5
    local log_file="${OUTPUT_DIR}/iperf_${server}_to_${client}.log"
    local pcap_file="${OUTPUT_DIR}/iperf_${server}_to_${client}.pcap"
    
    echo "Running iperf test: $test_name ($server as server, $client as client)"
    echo "Test: $test_name ($server as server, $client as client)" > "$log_file"
    
    # Run 3 tests
    for i in {1..3}; do
        echo "Test run $i" >> "$log_file"
        sudo mn -c > /dev/null 2>&1
        
        # Start packet capture
        sudo python3 q2.py <<EOF &
h9 tcpdump -i h9-eth1 -n tcp -w $pcap_file
EOF
        sleep 2
        
        # Run the test
        sudo python3 q2.py <<EOF
${server} iperf3 -s -p 5201 >> $log_file 2>&1 &
sleep 5
h9 iptables -t nat -L -v -n >> $log_file
${client} iperf3 -c ${server_ip} -p 5201 -t 120 >> $log_file 2>&1
h9 iptables -t nat -L -v -n >> $log_file
${server} pkill iperf3
h9 pkill tcpdump
exit
EOF
        
        echo "Waiting 30 seconds before next test..."
        sleep 30
    done
    
    echo "Completed iperf test: $server to $client"
}

# Run all the required tests
echo "Running ping tests from internal to external hosts"
run_ping_test "h1" "h5" "10.0.0.6" "Internal to External Test 1"
run_ping_test "h2" "h3" "10.0.0.4" "Internal to External Test 2"

echo "Running ping tests from external to internal hosts"
run_ping_test "h8" "h1" "10.1.1.2" "External to Internal Test 1"
run_ping_test "h6" "h2" "10.1.1.3" "External to Internal Test 2"

echo "Running iperf tests"
run_iperf_test "h1" "10.1.1.2" "h6" "10.0.0.7" "Iperf Test 1"
run_iperf_test "h8" "10.0.0.9" "h2" "10.1.1.3" "Iperf Test 2"

echo "Copying NAT rules..."
cp /tmp/nat_rules.txt "$OUTPUT_DIR/nat_rules.txt"

echo "All tests completed."