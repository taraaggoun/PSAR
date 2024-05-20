import matplotlib.pyplot as plt
import numpy as np

def get_dict(filename):
    data_dict = []
    with open(filename, 'r') as file:
        for line in file:
            tokens = line.strip().split()
            data_dict.append(int(tokens[0])/1000000)
    return data_dict

def plot_bar_2_with_std(values1, values2, type, configuration):
    # Calcul de la moyenne et de l'écart type
    moyenne1 = np.mean(values1)
    moyenne2 = np.mean(values2)
    std_dev1 = np.std(values1)
    std_dev2 = np.std(values2)
    print("Lecture " + type + " configurations " + configuration + " local: moyenne :" + str(moyenne1) + " ecart type " + str(std_dev1))
    print("Lecture " + type + " configurations " + configuration + " distant: moyenne :" + str(moyenne2) + " ecart type " + str(std_dev2))

    x = np.arange(2)
    largeur = 0.35
    fig, ax = plt.subplots()
    rects1 = ax.bar(x[0], moyenne1, largeur, yerr=std_dev1, capsize=5, label='Locale')
    rects2 = ax.bar(x[1], moyenne2, largeur, yerr=std_dev2, capsize=5, label='Distant')

    # Étiquettes et légendes
    ax.set_ylabel('Temps en milliseconde')
    ax.set_title('Temps moyen de lecture ' + type + ' avec écart type')
    ax.set_xticks(x)
    ax.set_xticklabels(['Locale', 'Distant'])
    ax.legend()
    # plt.show()
    plt.savefig(type + configuration + ".png") # Sauvegarder le graphique dans un fichier
    plt.close()

import numpy as np
import matplotlib.pyplot as plt

def plot_bar_4_with_std(values1, values2, values3, values4, type, configuration):
    # Calcul de la moyenne et de l'écart type
    moyenne1 = np.mean(values1)
    moyenne2 = np.mean(values2)
    moyenne3 = np.mean(values3)
    moyenne4 = np.mean(values4)
    
    std_dev1 = np.std(values1)
    std_dev2 = np.std(values2)
    std_dev3 = np.std(values3)
    std_dev4 = np.std(values4)

    x = np.arange(4)
    largeur = 0.2
    fig, ax = plt.subplots()
    rects1 = ax.bar(x[0], moyenne1, largeur, yerr=std_dev1, capsize=5, label='0')
    rects2 = ax.bar(x[1], moyenne2, largeur, yerr=std_dev2, capsize=5, label='10')
    rects3 = ax.bar(x[2], moyenne3, largeur, yerr=std_dev3, capsize=5, label='100')
    rects4 = ax.bar(x[3], moyenne4, largeur, yerr=std_dev4, capsize=5, label='1000')

    # Étiquettes et légendes
    ax.set_ylabel('Temps en milliseconde')
    ax.set_title('Temps moyen de lecture avec plusieurs ' + type + ' avec écart type')
    ax.set_xticks(x)
    ax.set_xticklabels(['0', '10', '100', '1000'])
    # ax.legend()
    # plt.show()
    plt.savefig(type + configuration + ".png") # Sauvegarder le graphique dans un fichier
    plt.close()

def main():
    print("------------ configuration 1 ------------")
    local_seq_1 = get_dict("local_seq_1")
    distant_seq_1 = get_dict("distant_seq_1")
    plot_bar_2_with_std(local_seq_1, distant_seq_1, "sequentielle", "1")

    local_alea_1 = get_dict("local_alea_1")
    distant_alea_1 = get_dict("distant_alea_1")
    plot_bar_2_with_std(local_alea_1, distant_alea_1, "aleatoire", "1")

    f0_1 = get_dict("f0_1")
    f10_1 = get_dict("f10_1")
    f100_1 = get_dict("f100_1")
    f1000_1 = get_dict("f1000_1")
    plot_bar_4_with_std(f0_1, f10_1, f100_1, f1000_1, "fichier", "1")

    p0_1 = get_dict("p0_1")
    p10_1 = get_dict("p10_1")
    p100_1 = get_dict("p100_1")
    p1000_1 = get_dict("p1000_1")
    plot_bar_4_with_std(p0_1, p10_1, p100_1, p1000_1, "processus", "1")

    print("------------ configuration 2 ------------")
    local_seq_2 = get_dict("local_seq_2")
    distant_seq_2 = get_dict("distant_seq_2")
    plot_bar_2_with_std(local_seq_2, distant_seq_2, "sequentielle", "2")

    local_alea_2 = get_dict("local_alea_2")
    distant_alea_2 = get_dict("distant_alea_2")
    plot_bar_2_with_std(local_alea_2, distant_alea_2, "aleatoire", "2")

    f0_2 = get_dict("f0_2")
    f10_2 = get_dict("f10_2")
    f100_2 = get_dict("f100_2")
    f1000_2 = get_dict("f1000_2")
    plot_bar_4_with_std(f0_2, f10_2, f100_2, f1000_2, "fichier", "2")

    p0_2 = get_dict("p0_2")
    p10_2 = get_dict("p10_2")
    p100_2 = get_dict("p100_2")
    p1000_2 = get_dict("p1000_2")
    plot_bar_4_with_std(p0_2, p10_2, p100_2, p1000_2, "processus", "2")

    print("------------ configuration 3 ------------")
    local_seq_3 = get_dict("local_seq_3")
    distant_seq_3 = get_dict("distant_seq_3")
    plot_bar_2_with_std(local_seq_3, distant_seq_3, "sequentielle", "3")

    local_alea_3 = get_dict("local_alea_3")
    distant_alea_3 = get_dict("distant_alea_3")
    plot_bar_2_with_std(local_alea_3, distant_alea_3, "aleatoire", "3")

    f0_3 = get_dict("f0_3")
    f10_3 = get_dict("f10_3")
    f100_3 = get_dict("f100_3")
    f1000_3 = get_dict("f1000_3")
    plot_bar_4_with_std(f0_3, f10_3, f100_3, f1000_3, "fichier", "3")

    p0_3 = get_dict("p0_3")
    p10_3 = get_dict("p10_3")
    p100_3 = get_dict("p100_3")
    p1000_3 = get_dict("p1000_3")
    plot_bar_4_with_std(p0_3, p10_3, p100_3, p1000_3, "processus", "3")

if __name__ == "__main__":
    main()
