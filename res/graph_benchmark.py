import matplotlib.pyplot as plt
import numpy as np

def get_dict(filename):
    data_dict = []
    with open(filename, 'r') as file:
        for line in file:
            tokens = line.strip().split()
            data_dict.append(int(tokens[0])/1000000)
    return data_dict

def plot_bar_with_std(values1, values2, type, configuration):
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
    plt.savefig(type + type + ".png") # Sauvegarder le graphique dans un fichier
    plt.close()

def main():
    local_seq_1 = get_dict("test_local_seq_1")
    distant_seq_1 = get_dict("test_distant_seq_1")
    plot_bar_with_std(local_seq_1, distant_seq_1, "sequentielle", "1")

    local_alea_1 = get_dict("test_local_alea_1")
    distant_alea_1 = get_dict("test_distant_alea_1")
    plot_bar_with_std(local_alea_1, distant_alea_1, "aleatoire", "1")



if __name__ == "__main__":
    main()
