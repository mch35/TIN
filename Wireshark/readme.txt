Instalacja Wiresharka z systemowego repozytorium Ubuntu / Debiana
sudo apt-get install wireshark

Wireshark domyślnie instaluje się z prawami do przechwytywania pakietów wyłącznie dla roota. Aby to zmienić, należy wykonać polecenie: 
sudo dpkg-reconfigure wireshark-common
a następnie wyrazić zgodę na przechwytywanie pakietów przez zwykłych użytkowników.

Następnie należy dodać użytkownika do grupy wireshark: 
sudo adduser nazwa_użytkownika wireshark 
i ponownie uruchomić system.

Skrypt wireshark.lua należy umieścić w katalogu /home/nazwa_użytkownika/.wireshark/plugins
Wireshark przy uruchomieniu automatycznie go odczyta.

Aby rozpocząć przechwytywanie pakietów, po uruchomieniu programu należy wybrać interfejs, i nacisnąć przycisk START.