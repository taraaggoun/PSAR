import matplotlib.pyplot as plt
# import pylab
# import numpy
import seaborn

# def get_dict(filename):
# 	data_dict = {}
# 	with open(filename, 'r') as file:
# 		for line in file:
# 			tokens = line.strip().split()
# 			type = tokens[0]
# 			time = int(tokens[1])
# 			if type in data_dict:
# 				data_dict[type].append(time)
# 			else:
# 				data_dict[type] = [time]
# 	return data_dict

def main():
    filename = 'benchmark.data'
    x = []
    y = []
    with open(filename, 'r') as file:
        for line in file:
            tokens = line.strip().split()
            x.append(tokens[0])
            y.append(int(tokens[1]) / 100000)
    plt.scatter(x, y)
    plt.show()


    # names = ['Local-Local', 'Distant-Local', 'Local-Distant', 'Disatant-Distant']
    # data_dict = get_dict("benchmark.data")

    # seaborn.set(style="whitegrid")
    # tips = seaborn.load_dataset("tips")

    # seaborn.swarmplot(x="Type", y="time (in ns)", data=tips)
    # plt.plot(132)
    # for i in range (data_dict[].len):
    #     plt.scatter(names, [data_dict[0][i], data_dict[1][i], data_dict[2][i], data_dict[3][i]])
    # plt.title('Blabla')
    # plt.show()
    

if __name__ == "__main__":
    main()