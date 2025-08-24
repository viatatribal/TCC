import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv("timestamp1min.csv")

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
plt.scatter(1, maximo_total, color='black', label=f"Máximo total: {maximo_total:.2f} us")
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total:.2f} us")


# Estética
plt.ylim(minimo_total,maximo_quadradodt1)
plt.title("Intervalo por amostras 1 minuto")
plt.ylabel("Intervalo por amostras")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

outlierdt1 = [t for t in dt1 if t > maximo_quadradodt1 or t < minimo_quadradodt1]
nooutliert1 = [t for t in dt1 if t <= maximo_quadradodt1 and t >= minimo_quadradodt1]

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
       sum = 0
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
plt.scatter(1, minimo, color='blue', label=f"Mínimo: {minimo/1000:.2f} ms")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana/1000:.2f} ms")
plt.scatter(1, maximo, color='red', label=f"Máximo: {maximo/1000:.2f} ms")
plt.scatter(1, media, color='purple', label=f"Média: {media:.2f} ms", marker='D')
plt.scatter(1, maximo_total, color='black', label=f"Máximo total: {maximo_total/1000:.2f} ms")
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total/1000:.2f} ms")


# Estética
plt.ylim(minimo_total,maximo)
plt.title("Intervalo por Irms 1 minuto")
plt.ylabel("Intervalo por Irms")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

# Irms
df = pd.read_csv("irms1min.csv")

di1 = df["Irms1"]

media = di1.mean()
maximo_total = di1.max()
minimo_total = di1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 1, 1)  # 1 linha, 1 colunas, gráfico 1
box = plt.boxplot(di1, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo = box['caps'][0].get_ydata()[0]
maximo = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo, color='blue', label=f"Mínimo quadrado: {minimo} A")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana} A")
plt.scatter(1, maximo, color='red', label=f"Máximo quadrado: {maximo} A")
plt.scatter(1, media, color='purple', label=f"Média: {media} A")
plt.scatter(1, maximo_total, color='black', label=f"Máximo total: {maximo_total} A")
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total} A")

# Estética
plt.title("Irms 1 minuto")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

print(f'Máximo tempo no quadrado para 1 minuto: {maximo_quadradodt1}')
print(f'Porcentagem de outlier para 1 minuto: {(len(outlierdt1) / len(dt1)) * 100:.2f}%')
print(f'Total de intervalos fora do quadrado: {len(outlierdt1)}')
print(f'Total de intervalos dentor do quadrado: {len(nooutliert1)}\n\n')

ciclo = 60 * 60 * 1
total = len(di1)
perdas = ((ciclo-total)/ciclo) * 100
print(f'60 ciclos por segundo x 1 minuto: {ciclo}')
print(f'Total de Irms calculos em 1 minuto: {total}')
print(f'Porcentagem de amostras perdidas em 1 minuto: {perdas:.2f} %\n')
