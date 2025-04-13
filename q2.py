#!/usr/bin/python

from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Node
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.link import TCLink
import time

class LinuxRouter(Node):
    """A Node with IP forwarding enabled."""

    def config(self, **params):
        super(LinuxRouter, self).config(**params)
        # Enable forwarding on the router
        self.cmd('sysctl net.ipv4.ip_forward=1')

    def terminate(self):
        self.cmd('sysctl net.ipv4.ip_forward=0')
        super(LinuxRouter, self).terminate()

class NetworkTopo(Topo):
    def build(self, **_opts):
        # Add switches
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        
        # Add hosts
        h1 = self.addHost('h1', ip='10.1.1.2/24', defaultRoute='via 10.1.1.1')
        h2 = self.addHost('h2', ip='10.1.1.3/24', defaultRoute='via 10.1.1.1')
        h3 = self.addHost('h3', ip='10.0.0.4/24')
        h4 = self.addHost('h4', ip='10.0.0.5/24')
        h5 = self.addHost('h5', ip='10.0.0.6/24')
        h6 = self.addHost('h6', ip='10.0.0.7/24')
        h7 = self.addHost('h7', ip='10.0.0.8/24')
        h8 = self.addHost('h8', ip='10.0.0.9/24')
        
        # Add NAT host (h9)
        h9 = self.addHost('h9', ip='10.1.1.1/24')
        
        # Add links between switches with 7ms latency
        self.addLink(s1, s2, cls=TCLink, delay='7ms')
        self.addLink(s2, s3, cls=TCLink, delay='7ms')
        self.addLink(s3, s4, cls=TCLink, delay='7ms')
        self.addLink(s4, s1, cls=TCLink, delay='7ms')
        self.addLink(s1, s3, cls=TCLink, delay='7ms')
        
        # Connect hosts to switches with 5ms latency
        self.addLink(h3, s2, cls=TCLink, delay='5ms')
        self.addLink(h4, s2, cls=TCLink, delay='5ms')
        self.addLink(h5, s3, cls=TCLink, delay='5ms')
        self.addLink(h6, s3, cls=TCLink, delay='5ms')
        self.addLink(h7, s4, cls=TCLink, delay='5ms')
        self.addLink(h8, s4, cls=TCLink, delay='5ms')
        
        # Connect h9 to s1 with 5ms latency
        self.addLink(h9, s1, cls=TCLink, delay='5ms')
        
        # Connect h1 and h2 to h9 with 5ms latency
        self.addLink(h1, h9, cls=TCLink, delay='5ms')
        self.addLink(h2, h9, cls=TCLink, delay='5ms')

def configure_network():
    """Configure the network with NAT"""
    net = Mininet(topo=NetworkTopo(), link=TCLink)
    net.start()
    
    # Configure NAT on h9
    h9 = net.get('h9')
    
    # Add second interface on h9 with public IP
    h9.cmd('ip addr add 172.16.10.10/24 dev h9-eth1')
    
    # Enable NAT on h9
    h9.cmd('iptables -t nat -A POSTROUTING -o h9-eth1 -j MASQUERADE')
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth1 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    
    # Configure port forwarding for internal servers (if needed)
    # h9.cmd('iptables -t nat -A PREROUTING -i h9-eth1 -p tcp --dport 5201 -j DNAT --to-destination 10.1.1.2:5201')
    
    # Add routes on other hosts to reach the private network
    for host in ['h3', 'h4', 'h5', 'h6', 'h7', 'h8']:
        h = net.get(host)
        h.cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
    
    # Additional routes for s1-s4 to find the private network
    for switch in ['s1', 's2', 's3', 's4']:
        s = net.get(switch)
        s.cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
    
    # Configure h1 and h2 to use h9 as gateway
    h1 = net.get('h1')
    h2 = net.get('h2')
    h1.cmd('ip route add default via 10.1.1.1')
    h2.cmd('ip route add default via 10.1.1.1')
    
    # Return the network object
    return net

if __name__ == '__main__':
    setLogLevel('info')
    
    # Configure and start the network
    net = configure_network()
    
    # Display network information
    info('*** Network configuration:\n')
    for host in net.hosts:
        info(f'{host.name}: {host.IP()}\n')
        host.cmd('ifconfig')
    
    # Start CLI
    CLI(net)
    
    # Clean up
    net.stop()