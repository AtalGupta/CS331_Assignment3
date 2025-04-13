#!/usr/bin/env python
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Controller, OVSController
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel, info
from mininet.node import OVSSwitch
import time

class CustomTopo(Topo):
    """Custom topology with 4 switches and 8 hosts"""
    
    def build(self):
        # Add switches
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        
        # Add hosts
        h1 = self.addHost('h1', ip='10.0.0.2/24')
        h2 = self.addHost('h2', ip='10.0.0.3/24')
        h3 = self.addHost('h3', ip='10.0.0.4/24')
        h4 = self.addHost('h4', ip='10.0.0.5/24')
        h5 = self.addHost('h5', ip='10.0.0.6/24')
        h6 = self.addHost('h6', ip='10.0.0.7/24')
        h7 = self.addHost('h7', ip='10.0.0.8/24')
        h8 = self.addHost('h8', ip='10.0.0.9/24')
        
        # Add links between switches with 7ms latency
        self.addLink(s1, s2, delay='7ms')
        self.addLink(s2, s3, delay='7ms')
        self.addLink(s3, s4, delay='7ms')
        self.addLink(s4, s1, delay='7ms')
        self.addLink(s1, s3, delay='7ms')
        
        # Add links between hosts and switches with 5ms latency
        self.addLink(h1, s1, delay='5ms')
        self.addLink(h2, s1, delay='5ms')
        self.addLink(h3, s2, delay='5ms')
        self.addLink(h4, s2, delay='5ms')
        self.addLink(h5, s3, delay='5ms')
        self.addLink(h6, s3, delay='5ms')
        self.addLink(h7, s4, delay='5ms')
        self.addLink(h8, s4, delay='5ms')

def run_ping_tests(net, test_number):
    """Run the specific ping tests"""
    info("\n*** Running test set {} ***\n".format(test_number))
    
    # Test ping from h3 to h1
    info("\n*** Pinging h1 from h3 (Test {}):\n".format(test_number))
    h3 = net.get('h3')
    h1 = net.get('h1')
    info("Pinging {} from {}\n".format(h1.IP(), h3.name))
    result = h3.cmd('ping -c 4 {}'.format(h1.IP()))
    info(result)
    
    # Test ping from h5 to h7
    info("\n*** Pinging h7 from h5 (Test {}):\n".format(test_number))
    h5 = net.get('h5')
    h7 = net.get('h7')
    info("Pinging {} from {}\n".format(h7.IP(), h5.name))
    result = h5.cmd('ping -c 4 {}'.format(h7.IP()))
    info(result)
    
    # Test ping from h8 to h2
    info("\n*** Pinging h2 from h8 (Test {}):\n".format(test_number))
    h8 = net.get('h8')
    h2 = net.get('h2')
    info("Pinging {} from {}\n".format(h2.IP(), h8.name))
    result = h8.cmd('ping -c 4 {}'.format(h2.IP()))
    info(result)

def run_tests():
    """Create network and run ping tests"""
    topo = CustomTopo()
    
    # Use OVSController which works better for traditional switching
    net = Mininet(topo=topo, link=TCLink, switch=OVSSwitch, controller=OVSController)
    
    net.start()
    
    # Let's make sure all switches are properly connected to the controller
    for s in net.switches:
        info('*** Setting up switch {} \n'.format(s.name))
        # Enable STP on the switches
        s.cmd('ovs-vsctl set bridge {} stp_enable=true'.format(s.name))
    
    # Allow time for STP to converge (important for a network with loops)
    info('*** Waiting for STP to converge\n')
    time.sleep(10)  # Longer wait time for STP to fully converge
    
    # Configure default routes for all hosts (just to be sure)
    for host in net.hosts:
        host.cmd('route add default dev {}-eth0'.format(host.name))
    
    # Create a complete virtual ARP table on each host
    info('*** Setting up ARP tables\n')
    for host in net.hosts:
        for target in net.hosts:
            if host != target:
                host.cmd('arp -s {} {}'.format(target.IP(), target.MAC()))
    
    # Additional step: make sure IP forwarding is enabled
    for host in net.hosts:
        host.cmd('echo 1 > /proc/sys/net/ipv4/ip_forward')
    
    info("Network topology has been created and configured\n")
    
    # Let's verify connectivity with a full pingAll test first
    info("\n*** Testing full network connectivity\n")
    net.pingAll()
    
    # Run each test 3 times with 30-second intervals
    for test_num in range(1, 4):
        if test_num > 1:
            info("\n*** Waiting 30 seconds before the next test set ***\n")
            time.sleep(30)
        
        run_ping_tests(net, test_num)
    
    # Interactive CLI for manual testing
    CLI(net)
    
    # Cleanup
    net.stop()

if __name__ == '__main__':
    # Clean up any Mininet remnants
    import os
    os.system('sudo mn -c')
    
    # Tell mininet to print useful information
    setLogLevel('info')
    run_tests()
