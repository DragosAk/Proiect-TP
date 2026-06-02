Structura:

* assets: Resursele grafice (texturi, sprite-uri, fundaluri).
* generators: Scripturi Python generatoare de asset-uri.
* wavs: Efectelor sonore.
* main.c: Scriptul principal pentru a initializa fereastra Raylib si a rula jocul.
* farm.c: Logica principala a jocului, gestionand miscarea, inventarul, magazinul și randarea hartii.
* farm.h: Fisier de antet (header) continand definitiile pentru structuri, constante si starile jocului.
* Makefile: Script de compilare (build) pentru a asambla fișierele C.
* menu.mp3: Fișierul audio comprimat pentru coloana sonoră a meniului principal.

Cerinte:

* GCC
* Raylib
* Make

Setup:

* git clone https://github.com/DragosAk/Farming-Simulator
* make
* ./farming_simulator
