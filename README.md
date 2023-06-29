# RA_CacheSim
Cache Simulator Projekt für Rechnerarchitektur

Bedienungsanleitung:

Der Simulator nimmt eine Trace-Datei als Input und führt die Cachesimulation anhand der festgesetzten Parameter durch. 
Das Ergebnis der Simulation wird in eine Output-Datei geschrieben.
Außer der Input- und Output-Datei müssen die Parameter nicht zwingend dem Simulator übergeben werden. 
Falls ein Parameter nicht angegeben wird, wird die Simulation mit dem Standardwert für diesen Parameter durchgeführt:

-t "input";     Input-Datei für den Simulator. Sie muss sich im gleichen Ordner wie das Programm befinden oder mit dem vollständigen Pfad angegeben werden.
                Format: # [L/S] [32-Bit Adresse]    -> Alle zeilen sollten so aufgebaut sein. L/S ist 0 für Load-befehle und 1 für Store-befehle;    

-n "1024";      Gesamtanzahl an Cache blöcken.

-g "32";        Größe von den einzelnen Cache blöcken in Byte.

-a "2";         Assoziativität des Caches.

-r "LRU";       Ersetzungsrichtline für den Cache -> "LRU" = Least recently used, "FIFO" = First-in-first-out und "Random" = Zufallsersetzung.

-w "NoAlloc";   Schreibrichtlinie für den Cache -> "Alloc" = Write allocate, "NoAlloc" = No allocate.

-o "output";    Output-Datei für den Simulator. Gleiche Bedingungen wie für den Input. Die Datei wird nicht überschrieben.

Beispiel:
./cachesim -t art.trace -n 1024 -g 32 -a 2 -r LRU -w NoAlloc -o output.txt

Dies sind auch die Standardwerte für die jeweiligen Parameter.

ENG

Cache Simulator Project for Computer Architecture

User Manual:

The simulator takes a trace file as input and performs cache simulation based on the set parameters. 
The result of the simulation is written to an output file.
Except for the input and output files, the parameters do not necessarily need to be passed to the simulator. 
If a parameter is not provided, the simulation will be performed using the default value for that parameter.

-t "input";     Input file for the simulator, must be in the same directory as the program or specified with the complete path.
                Format: # [L/S] [32-bit address] -> All lines should follow this structure, where L/S is 0 for load instructions and 1 for store instructions.

-n "1024";      Total number of cache blocks.

-g "32";        Size of each cache block in bytes.

-a "2";         Associativity of the cache.

-r "LRU";       Replacement policy for the cache -> "LRU" = Least Recently Used, "FIFO" = First In First Out, and "Random" = Random.

-w "NoAlloc";   Write policy for the cache -> "Alloc" = Write Allocate, "NoAlloc" = No Allocate.

-o "output";    Output file for the simulator, same conditions as the input file. The file will not be overwritten.

Example:
./cachesim -t art.trace -n 1024 -g 32 -a 2 -r LRU -w NoAlloc -o output.txt

These are also the default values for each parameter.
