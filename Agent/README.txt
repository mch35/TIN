1. Instalacja bibliotek

Dla każdej biblioteki:
http://www.netfilter.org/projects/libnfnetlink/downloads.html
http://www.netfilter.org/projects/libmnl/downloads.html
http://www.netfilter.org/projects/libnetfilter_queue/downloads.html

    tar -xvf libnfnetlink-1.0.0.tar.bz2
    cd libnfnetlink-1.0.0/
    ./configure
    make
    sudo make install
	sudo ldconfig 
	
2. Ustawienie iptables
	sudo iptables -A OUTPUT -p tcp -j NFQUEUE --queue-num 0

3. Agent musi zostać uruchomiony z prawami roota.

4. Usuwanie

Żeby zobaczyć na których pozycjach są nasze reguły:
	sudo iptables -L OUTPUT --line-numbers
	
Później dla każdej pozycji:
	 iptables -D OUTPUT (nr pozycji)