# Psar 2024 : Optimisation des processus selon les données en mémoire

## Description du code
Notre projet est organisé de façon à ce que chaque répertoire ait une fonction spécifique.

### Patch
Dans le répertoire patch, se trouvent 4 fichiers :
- Le fichier `force_open` a pour but d'ajouter les modifications dans le noyau pour intégrer notre optimisation où l'on force un processus à aller sur un cœur du nœud où les données sont chargées en mémoire.
- Le fichier `reverse_force_open` sert à supprimer les modifications ajoutées avec le fichier `force_open`.
- Le fichier `try_open` a pour but d'ajouter les modifications dans le noyau pour intégrer notre optimisation où l'on propose au scheduler de déplacer le processus sur le nœud où les données sont chargées en mémoire.
- Le fichier `reverse_try_open` sert à supprimer les modifications ajoutées avec le fichier `try_open`.

### Ioctl
Dans le répertoire ioctl, nous avons :
- Le fichier `openctl`, qui est un module pour faire un ioctl qui nous permet de rajouter un pid à la liste des pids que nous monitorons.
- Le fichier `add_open_pid_tab` est un code C qui prend en paramètre un pid et appelle l'ioctl avec ce paramètre.
- Les scripts `insdev` et `rmdev` sont des scripts qui permettent d'insérer et de supprimer l'ioctl.

### Modules
Dans le répertoire module, nous avons deux modules :
- Le premier, `print_node_file`, prend en paramètre un nom de fichier et affiche pour toutes les pages de ce fichier sur quel nœud elles sont chargées.
- Le deuxième, `print_node_pid`, prend un pid et une liste de descripteurs de fichiers et regarde pour tous les descripteurs de fichiers du pid passé en paramètre sur quel nœud sont chargées toutes les pages.

### Benchmark
Dans le répertoire benchmark, nous avons trois codes C :
- Le premier, `benchmark_file`, prend en paramètre un nom de fichier et deux numéros de cœurs, et va charger le fichier en mémoire sur le premier cœur puis va calculer les temps de lecture depuis les deux cœurs.
- Le deuxième, `benchmark_pid`, prend en paramètre une liste de noms de fichiers, les charge tous en mémoire et lit en boucle le premier fichier.
- Enfin, le troisième benchmark prend en paramètre une configuration (1 pour pas de modification, 2 pour l'optimisation où l'on force le processus à aller sur un cœur, 3 pour proposer au scheduler un nœud) et un nœud. Ce code C va exécuter les différents types de tests dont nous avons besoin pour tester notre projet.

### Script
Dans le répertoire script, il y a un script bash qui permet d'installer tous les paquets dont nous avons besoin et de créer tous les fichiers nécessaires pour générer nos tests. Ce script est utilisé au démarrage d'une machine sur Grid5000.

### Res
Dans ce répertoire, se trouvent tous nos fichiers de log ainsi que les graphiques générés et le fichier Python qui permet de générer les graphiques.

## Compilation et exécution

Pour compiler tous nos codes, un Makefile est fourni. Il faudra modifier le chemin vers le Kernel pour pouvoir compiler les modules.

Pour tout compiler, il suffit de faire : 
```bash=
make
```
Pour supprimer tout les fichier généré a la compilation : 
```bash=
make clean
```

Pour lancer la génération des graphes :
```bash=
python3 graph_benchmark.py
```