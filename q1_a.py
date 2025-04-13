#!/usr/bin/env python
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import OVSController
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel

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

def run_tests():
    """Create network and run ping tests"""
    topo = CustomTopo()
    net = Mininet(topo=topo, link=TCLink, controller=OVSController)
    net.start()
    net.pingAll()
    
    print("Network topology has been created")
    
    # Test ping from h1 to h3
    print("\nPinging h1 from h3:")
    h3 = net.get('h3')
    result = h3.cmd('ping -c 4 10.0.0.2')
    print(result)
    
    # Test ping from h7 to h5
    print("\nPinging h7 from h5:")
    h5 = net.get('h5')
    result = h5.cmd('ping -c 4 10.0.0.8')
    print(result)
    
    # Test ping from h2 to h8
    print("\nPinging h2 from h8:")
    h8 = net.get('h8')
    result = h8.cmd('ping -c 4 10.0.0.3')
    print(result)
    
    # Interactive CLI for manual testing
    CLI(net)
    
    # Cleanup
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run_tests()