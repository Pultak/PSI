Router R2

Přiřazování Ip addresy k rozhraní gigabitEthernet 1/0


config term
interface gigabitEthernet 1/0
ip address 10.10.2.2 255.255.255.224
no shutdown
end



Nastavení DHCP 
config term
ip dhcp pool home
network 10.10.2.0 255.255.255.224
default-router 10.10.2.2
dns-server 8.8.8.8 4.4.4.4
end

Nastavení druhého rozhraní směřující k R1

config term
interface gigabitEthernet 0/0
ip address 192.168.1.2 255.255.255.252
no shutdown
end


Nastavení RIP

config term 
router rip 
version 2
network 10.10.2.0
network 192.168.1.0
end


Uložení všech změn do konfigurace na disku

copy running-config startup-config




Router R1

Nastavení rozhraní k R2

config term
interface gigabitEthernet 1/0
ip address 192.168.1.1 255.255.255.252
no shutdown
end

Nastavení protokolu RIP

config term
router rip
version 2
network 192.168.1.0
end


Získání IP adresy od poskytovatele internetového připojení

config term
interface gigabitEthernet 0/0
ip address dhcp
no shutdown
end


Nastavení NAT

config term
access-list 100 permit ip 192.168.1.0 0.0.0.3 any
access-list 100 permit ip 10.10.2.0 0.0.0.31 any
interface gigabitEthernet 1/0
ip nat inside
interface gigabitEthernet 0/0
ip nat outside
ip nat inside source list 100 interface GigabitEthernet0/0 overload
end


Uložení nastavení 

copy running-config startup-config




