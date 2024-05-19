import matplotlib.pyplot as plt
import numpy as np

def plot_curve_with_std(values1, values2):
    # Calcul de la moyenne et de l'écart type
    moyenne1 = np.mean(values1)
    moyenne2 = np.mean(values2)
    std_dev1 = np.std(values1)
    std_dev2 = np.std(values2)

    # Création du graphique
    plt.figure()

    # Courbe pour les valeurs 1
    plt.errorbar(0, moyenne1, yerr=std_dev1, fmt='o', label='Courbe 1')

    # Courbe pour les valeurs 2
    plt.errorbar(1, moyenne2, yerr=std_dev2, fmt='o', label='Courbe 2')

    # Lignes horizontales pour les écarts types
    plt.plot([-0.1, -0.1], [moyenne1 - std_dev1, moyenne1 + std_dev1], linestyle='--', color='gray')
    plt.plot([0.1, 0.1], [moyenne2 - std_dev2, moyenne2 + std_dev2], linestyle='--', color='gray')

    # Étiquettes et légendes
    plt.xticks([0, 1], ['Courbe 1', 'Courbe 2'])
    plt.ylabel('Valeurs')
    plt.title('Courbe avec moyenne et écart type')
    plt.legend()

    plt.show()

# Exemple d'utilisation avec des données d'exemple
values1 = [10, 15, 20, 25, 30]
values2 = [12, 17, 22, 27, 32]
plot_curve_with_std(values1, values2)
