 ICMP Echo Request 허용
netsh advfirewall firewall add rule name="Allow ICMP Echo Request" protocol=icmpv4 dir=in action=allow

 ICMP Echo Reply 허용
netsh advfirewall firewall add rule name="Allow ICMP Echo Reply" protocol=icmpv4 dir=out action=allow

ICMP Time Exceeded 허용
netsh advfirewall firewall add rule name="Allow ICMP Time Exceeded" protocol=icmpv4 dir=in action=allow

저거 3개 했는데도 안되면 
모든 ICMP 패킷 허용
netsh advfirewall firewall add rule name="Allow all ICMP" protocol=icmpv4 dir=in action=allow
