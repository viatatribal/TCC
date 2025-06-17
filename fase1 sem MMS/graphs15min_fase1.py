import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv("timestamp15min.csv")

dt1 = df["timestamp1"] / 1000

media = dt1.mean()
maximo_total = dt1.max()
minimo_total = dt1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 1, 1)  # 1 linha, 1 coluna, gráfico 1
box = plt.boxplot(dt1, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo_quadradodt1 = box['caps'][0].get_ydata()[0]
maximo_quadradodt1 = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo_quadradodt1, color='blue', label=f"Mínimo: {minimo_quadradodt1:.2f} us")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana:.2f} us")
plt.scatter(1, maximo_quadradodt1, color='red', label=f"Máximo: {maximo_quadradodt1:.2f} us")
plt.scatter(1, media, color='purple', label=f"Média: {media:.2f} us", marker='D')
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total:.2f} us", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total:.2f} us", marker="d")


# Estética
plt.title("Timestamp 15 minutos")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

# Soma dos timestamp por Irms calcualdo
amostra = 80
timestampIrms1 = []
sumTotalIrms1 = 0
mediaIrms1 = 0
minstampIrms1 = 9999999
maxstampIrms1 = 0

sum = 0
for i in range(len(dt1)):
    sum += dt1[i]
    if (i+1) % amostra == 0 and i > 0:
       timestampIrms1.append(sum)
       sumTotalIrms1 += sum
       mediaIrms1 += 1
       if sum < minstampIrms1:
           minstampIrms1 = sum
       elif sum > maxstampIrms1:
           maxstampIrms1 = sum
timestampIrms1.append(sum)
mediaIrms1 = sumTotalIrms1 / (mediaIrms1 + 1)

media = mediaIrms1
maximo_total = maxstampIrms1
minimo_total = minstampIrms1

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 1, 1)  # 1 linha, 1 coluna, gráfico 1
box = plt.boxplot(timestampIrms1, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo = box['caps'][0].get_ydata()[0]
maximo = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo, color='blue', label=f"Mínimo: {maximo/1000:.2f} ms")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana/1000:.2f} ms")
plt.scatter(1, maximo, color='red', label=f"Máximo: {maximo/1000:.2f} ms")
plt.scatter(1, media, color='purple', label=f"Média: {media/1000:.2f} ms", marker='D')
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total/1000:.2f} ms", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total/1000:.2f} ms", marker="d")

# Estética
plt.title("Timestamp por Irms 15 minutos")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()


# Irms
df = pd.read_csv("irms15min.csv")

di1 = df["Irms1"]

media = di1.mean()
maximo_total = di1.max()
minimo_total = di1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 1, 1)  # 1 linha, 1 coluna, gráfico 1
box = plt.boxplot(di1, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo = box['caps'][0].get_ydata()[0]
maximo = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo, color='blue', label=f"Mínimo quadrado: {minimo} A")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana} A")
plt.scatter(1, maximo, color='black', label=f"Máximo quadrado: {maximo} A")
plt.scatter(1, media, color='purple', label=f"Média: {media} A")
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total} A", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total} A", marker="d")

# Estética
plt.title("Irms 15 minutos")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

di1 = df["Irms1"].round(2)

media = di1.mean()
maximo_total = di1.max()
minimo_total = di1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 1, 1)  # 1 linha, 1 coluna, gráfico 1
box = plt.boxplot(di1, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo = box['caps'][0].get_ydata()[0]
maximo = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo, color='blue', label=f"Mínimo quadrado: {minimo:.2f} A")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana:.2f} A")
plt.scatter(1, maximo, color='black', label=f"Máximo quadrado: {maximo:.2f} A")
plt.scatter(1, media, color='purple', label=f"Média: {media:.2f} A")
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total:.2f} A", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total:.2f} A", marker="d")

# Estética
plt.title("Irms 15 minutos")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.show()


outlierdt1 = [t for t in dt1 if t > maximo_quadradodt1]


print(f'Máximo tempo no quadrado para 15 minutos fase A: {maximo_quadradodt1}')
print(f'Porcentagem de outlier para 15 minutos fase A: {(len(outlierdt1) / len(dt1)) * 100:.2f}%\n\n')

ciclo = 60 * 60 * 15
total = len(di1)
perdas = ((ciclo-total)/ciclo) * 100
print(f'60 ciclos por segundo x 15 minutos: {ciclo}')
print(f'Total de Irms calculos em 15 minutos: {total}')
print(f'Porcentagem de amostras perdidas em 15 minutos: {perdas:.2f} %\n')
