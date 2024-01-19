Simple Filesystem

Biblioteca este impartia in 3 parti:
	- disk driver
	- filesystem
	- api filesystem

La asta se adauga un simplu utilitar in linie de comanda care evidentiaza functionalitatile de baza a filesystem-ului.

Disk driver:
	- are rolul de a interfata filesystem-ul cu disk-ul;
	- nu face verificari, are rolul doar de scriere si citire;

Filesystem:
	- implementeaza baza intregului filesystem;
	- permite operatii de baza: read, write, format, mount, unmount, debug, create si remove;
	- se ocupa de sincronizarea accesului la acelasi disk;


Api filesystem:
	- interfateaza o aplicatie cu filesystem-ul;
	- se ocupa de gestionarea utilizatorilor;

Utilitar:
	- prezinta operatii de baza ale filesystem-ului;
	- pentru a verifica care sunt operatiile, folositi comanda 'help'.

Scenariu de testare:
	Se ruleaza utilizeaza shell-ul pus la dispozitie folosind comanda './bin/main' din folder-ul 'PROIECT_PSO'. In terminal se introduce litera 'n' si se adauga un nou utilizator la alegere. Utilizatorul poate vedea comenzile disponibile comanda 'help'. Utilizatorul va crea un nou grup folosind comanda groupadd cu un nume la alegere. Folosind comenzile 'showusers' si 'showgroups' se vor vedea utilizatorii si grupurile create. 
	
	Utilizatorul va folosi comanda 'touch' pentru a crea un fisier. Va crea un nou fisier in folder-ul PROIECT_PSO si va adauga date in el. Il va copia in filesystem folosind comanda 'copyin ./<nume fisier linux> <nume fisier din filesystem>'. Folosind comanda cat pe fisierul nou copiat, se pot vizualiza datele adaugate. Stat va afisa informatiile privind utilizatorul, grupul si permisiunile fisierului. Folosind comanda 'ls' se vor vedea fisiere disponibile. Folosind comanda chmod se vor schimba permisiunile fisierului: 'chmod <nume fisier> 0600'. Astfel, va avea permisiuni de citire si scriere doar utilizatorul curent. Se va cere parola de sudo care este 'seful'. Se va folosi comanda 'return' pentru a schimba utilizatorul. Se va adauga din nou un utilizator tastand 'n' si prelevand informatiile necesare. Se va folosi comanda 'ls' pentru a verifica existenta fisierului folosit anterior. Incercand comanda cat pe fisierul creat anterior, vom primi o eroare deoarece nu avem permisiuni. Nici comanda rm nu va functiona. Folosind comanda exit vom inchide utilitarul. Ruland din nou utilitarul si folosind comanda ls, vom vedea ca datele au fost salvate. 
