from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import Host
from mininet.log import setLogLevel, info
from mininet.link import TCLink
from mininet.clean import cleanup
import time
import os

class LinuxRouter(Host):
    def config(self, **params):
        super(LinuxRouter, self).config(**params)
        self.cmd('sysctl net.ipv4.ip_forward=1')
        self.cmd('iptables -t nat -F')
        self.cmd('iptables -F')
        self.cmd('iptables -t nat -A POSTROUTING -o h9-eth0 -j MASQUERADE')
        self.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')
        self.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth0 -j ACCEPT')

class NatTopo(Topo):
    def build(self):
        # Create switches with unique ports
        s1 = self.addSwitch('s1')
        s2 = self.addSwitch('s2')
        s3 = self.addSwitch('s3')
        s4 = self.addSwitch('s4')
        
        # Create NAT router
        h9 = self.addHost('h9', cls=LinuxRouter, ip='172.16.10.10/24')
        
        # Internal hosts
        h1 = self.addHost('h1', ip='10.1.1.2/24', defaultRoute='via 10.1.1.1')
        h2 = self.addHost('h2', ip='10.1.1.3/24', defaultRoute='via 10.1.1.1')
        
        # External hosts
        for i in range(3,9):
            self.addHost(f'h{i}', ip=f'172.16.10.{i}/24')

        # Links with explicit port numbers
        self.addLink(h9, s1, port1=1, intfName1='h9-eth0', delay='5ms')
        self.addLink(h9, h1, port1=2, intfName1='h9-eth1', params1={'ip': '10.1.1.1/24'}, delay='5ms')
        self.addLink(h9, h2, port1=3, intfName1='h9-eth2', params1={'ip': '10.1.1.1/24'}, delay='5ms')
        
        # Switch connections with unique ports
        self.addLink(s1, s2, port1=4, port2=4, delay='7ms')
        self.addLink(s2, s3, port1=5, port2=5, delay='7ms')
        self.addLink(s3, s4, port1=6, port2=6, delay='7ms')
        self.addLink(s4, s1, port1=7, port2=7, delay='7ms')
        self.addLink(s1, s3, port1=8, port2=8, delay='7ms')
        
        # Host connections
        self.addLink(s2, 'h3', port1=9, delay='5ms')
        self.addLink(s2, 'h4', port1=10, delay='5ms')
        self.addLink(s3, 'h5', port1=11, delay='5ms')
        self.addLink(s3, 'h6', port1=12, delay='5ms')
        self.addLink(s4, 'h7', port1=13, delay='5ms')
        self.addLink(s4, 'h8', port1=14, delay='5ms')

def run_tests():
    # Full cleanup before starting
    os.system('sudo mn -c >/dev/null 2>&1')
    os.system('sudo ip -all netns delete >/dev/null 2>&1')
    cleanup()
    
    # Start network
    net = Mininet(topo=NatTopo(), link=TCLink, cleanup=True)
    net.start()
    
    # Configure routes
    for i in range(3,9):
        net.get(f'h{i}').cmd('ip route add 10.1.1.0/24 via 172.16.10.10')
    
    # Test functions
    def test(cmd, host, desc):
        print(f'\n=== {desc} ===')
        print(net.get(host).cmd(cmd))
    
    # Q2a Tests
    test('ping -c 4 172.16.10.6', 'h1', 'Q2ai: h1 -> h5 Ping Test')
    test('ping -c 4 172.16.10.4', 'h2', 'Q2aii: h2 -> h3 Ping Test')
    
    # Q2b Tests
    test('ping -c 4 172.16.10.10', 'h8', 'Q2bi: h8 -> h1 Ping Test')
    test('ping -c 4 172.16.10.10', 'h6', 'Q2bii: h6 -> h2 Ping Test')
    
    # Q2c Tests
    print('\n=== Q2ci: h1->h6 Iperf Test ===')
    net.get('h1').cmd('iperf3 -s -D')
    time.sleep(2)
    print(net.get('h6').cmd('iperf3 -c 10.1.1.2 -t 5 -i 1'))
    
    print('\n=== Q2cii: h8->h2 Iperf Test ===')
    net.get('h8').cmd('iperf3 -s -D')
    time.sleep(2)
    print(net.get('h2').cmd('iperf3 -c 172.16.10.9 -t 5 -i 1'))
    
    # Verification
    print('\n=== NAT Verification ===')
    print(net.get('h9').cmd('iptables -t nat -L -vn'))
    print('\n=== Routing Tables ===')
    for h in ['h1', 'h2', 'h5', 'h8', 'h9']:
        print(f'--- {h} ---')
        print(net.get(h).cmd('route -n'))
    
    net.stop()

if __name__ == '__main__':
    setLogLevel('info')
    run_tests()