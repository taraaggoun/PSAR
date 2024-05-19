import matplotlib.pyplot as plt
import numpy as np

def get_dict(filename):
    data_dict = []
    with open(filename, 'r') as file:
        for line in file:
            tokens = line.strip().split()
            data_dict.append(int(tokens[0])/1000000)
    return data_dict

def plot_bar_with_std(values1, values2, str, type):
    # Calcul de la moyenne et de l'écart type
    moyenne1 = np.mean(values1)
    moyenne2 = np.mean(values2)
    std_dev1 = np.std(values1)
    std_dev2 = np.std(values2)

    x = np.arange(2)
    largeur = 0.35
    fig, ax = plt.subplots()
    rects1 = ax.bar(x[0], moyenne1, largeur, yerr=std_dev1, capsize=5, label='Locale')
    rects2 = ax.bar(x[1], moyenne2, largeur, yerr=std_dev2, capsize=5, label='Distant')

    # Étiquettes et légendes
    ax.set_ylabel('Temps en milliseconde')
    ax.set_title('Temps moyen de lecture ' + str + ' avec écart type')
    ax.set_xticks(x)
    ax.set_xticklabels(['Locale', 'Distant'])
    ax.legend()
    plt.show()
    plt.savefig(str + type + ".png") # Sauvegarder le graphique dans un fichier
    plt.close()

def plot_curve_with_std(values1, values2, str, type):
    # Calcul de la moyenne et de l'écart type
    moyenne1 = np.mean(values1)
    moyenne2 = np.mean(values2)
    std_dev1 = np.std(values1)
    std_dev2 = np.std(values2)

    # Création du graphique
    plt.figure()

    # Courbe pour les valeurs 1
    plt.errorbar(0, moyenne1, yerr=std_dev1, fmt='o', label='Locale')

    # Courbe pour les valeurs 2
    plt.errorbar(1, moyenne2, yerr=std_dev2, fmt='o', label='Distant')

    # Relier les points entre les deux courbes
    plt.plot([0, 1], [moyenne1, moyenne2], linestyle='--', color='gray')

    # Étiquettes et légendes
    plt.xticks([0, 1], ['Locale', 'Distant'])
    plt.ylabel('Temps en milliseconde')
    plt.title('Temps moyen de lecture ' + str + ' avec écart type')
    plt.legend()

    plt.show()
    plt.savefig(str + type + ".png") # Sauvegarder le graphique dans un fichier
    plt.close()

def main():
    local_seq_1 = get_dict("test_local_seq_1")
    distant_seq_1 = get_dict("test_distant_seq_1")
    plot_bar_with_std(local_seq_1, distant_seq_1, "sequentielle", "1")

    local_alea_1 = get_dict("test_local_alea_1")
    distant_alea_1 = get_dict("test_distant_alea_1")
    plot_bar_with_std(local_alea_1, distant_alea_1, "aleatoire", "1")

    local_seq_p10_1 = get_dict("test_local_seq_p10_1")
    distant_seq_p10_1 = get_dict("test_distant_seq_p10_1")
    plot_bar_with_std(local_seq_p10_1, distant_seq_p10_1, "aleatoire", "1")

    local_alea_p10_1 = get_dict("test_local_alea_p10_1")
    distant_alea_p10_1 = get_dict("test_distant_alea_p10_1")
    plot_bar_with_std(local_alea_p10_1, distant_alea_p10_1, "aleatoire", "1")


if __name__ == "__main__":
    main()
