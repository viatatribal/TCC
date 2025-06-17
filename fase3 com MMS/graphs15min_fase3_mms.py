import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

df = pd.read_csv("timestamp15min.csv")

dt1 = df["timestamp1"] / 1000
dt2 = df["timestamp2"] / 1000
dt3 = df["timestamp3"] / 1000

media = dt1.mean()
maximo_total = dt1.max()
minimo_total = dt1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 3, 1)  # 1 linha, 3 colunas, gráfico 1
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
plt.title("Timestamp 15 minutos fase A")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()


media = dt2.mean()
maximo_total = dt2.max()
minimo_total = dt2.min()

# Criar boxplot com média
plt.subplot(1, 3, 2)  # 1 linha, 3 colunas, gráfico 2
box = plt.boxplot(dt2, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo_quadradodt2 = box['caps'][0].get_ydata()[0]
maximo_quadradodt2 = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo_quadradodt1, color='blue', label=f"Mínimo: {minimo_quadradodt1:.2f} us")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana:.2f} us")
plt.scatter(1, maximo_quadradodt2, color='red', label=f"Máximo: {maximo_quadradodt2:.2f} us")
plt.scatter(1, media, color='purple', label=f"Média: {media:.2f} us", marker='D')
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total:.2f} us", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total:.2f} us", marker="d")


# Estética
plt.title("Timestamp 15 minutos fase B")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()
#plt.show()


media = dt3.mean()
maximo_total = dt3.max()
minimo_total = dt3.min()

# Criar boxplot com média
plt.subplot(1, 3, 3)  # 1 linha, 3 colunas, gráfico 3
box = plt.boxplot(dt3, patch_artist=True, widths=0.5, showmeans=True)

# Pegar elementos do boxplot
minimo_quadradodt3 = box['caps'][0].get_ydata()[0]
maximo_quadradodt3 = box['caps'][1].get_ydata()[0]
mediana = box['medians'][0].get_ydata()[0]

# Adicionar marcadores no gráfico
plt.scatter(1, minimo_quadradodt3, color='blue', label=f"Mínimo: {minimo_quadradodt3:.2f} us")
plt.scatter(1, mediana, color='green', label=f"Mediana: {mediana:.2f} us")
plt.scatter(1, maximo_quadradodt3, color='red', label=f"Máximo: {maximo_quadradodt3:.2f} us")
plt.scatter(1, media, color='purple', label=f"Média: {media:.2f} us", marker='D')
plt.scatter(1, maximo_total, color='red', label=f"Máximo total: {maximo_total:.2f} us", marker='D')
plt.scatter(1, minimo_total, color='yellow', label=f"Mínimo total: {minimo_total:.2f} us", marker="d")


# Estética
plt.title("Timestamp 15 minutos fase C")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

# Soma dos timestamp por Irms calcualdo
amostra = 80
timestampIrms1 = []
timestampIrms2 = []
timestampIrms3 = []
sumTotalIrms1 = 0
sumTotalIrms2 = 0
sumTotalIrms3 = 0
mediaIrms1 = 0
mediaIrms2 = 0
mediaIrms3 = 0
minstampIrms1 = 9999999
maxstampIrms1 = 0
minstampIrms2 = 9999999
maxstampIrms2 = 0
minstampIrms3 = 9999999
maxstampIrms3 = 0

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

sum = 0
for i in range(len(dt2)):
    sum += dt2[i]
    if (i+1) % amostra == 0 and i > 0:
        timestampIrms2.append(sum)
        sumTotalIrms2 += sum
        mediaIrms2 += 1
        if sum < minstampIrms2:
            minstampIrms2 = sum
        elif sum > maxstampIrms2:
            maxstampIrms2 = sum
timestampIrms2.append(sum)
mediaIrms2 = sumTotalIrms2 / (mediaIrms2 + 1)

sum = 0
for i in range(len(dt3)):
    sum += dt3[i]
    if (i+1) % amostra == 0 and i > 0:
        timestampIrms3.append(sum)
        sumTotalIrms3 += sum
        mediaIrms3 += 1
        if sum < minstampIrms3:
            minstampIrms3 = sum
        elif sum > maxstampIrms3:
            maxstampIrms3 = sum
timestampIrms3.append(sum)
mediaIrms3 = sumTotalIrms3 / (mediaIrms3 + 1)

media = mediaIrms1
maximo_total = maxstampIrms1
minimo_total = minstampIrms1

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 3, 1)  # 1 linha, 3 colunas, gráfico 1
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
plt.title("Timestamp por Irms 15 minutos fase A")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()

media = mediaIrms2
maximo_total = maxstampIrms2
minimo_total = minstampIrms2

# Criar boxplot com média
plt.subplot(1, 3, 2)  # 1 linha, 3 colunas, gráfico 2
box = plt.boxplot(timestampIrms2, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Timestamp por Irms 15 minutos fase B")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()

media = mediaIrms3
maximo_total = maxstampIrms3
minimo_total = minstampIrms3

# Criar boxplot com média
plt.subplot(1, 3, 3)  # 1 linha, 3 colunas, gráfico 3
box = plt.boxplot(timestampIrms3, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Timestamp por Irms 15 minutos fase C")
plt.ylabel("Timestamp")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()


# Irms
df = pd.read_csv("irms15min.csv")

di1 = df["Irms1"]
di2 = df["Irms2"]
di3 = df["Irms3"]

media = di1.mean()
maximo_total = di1.max()
minimo_total = di1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 3, 1)  # 1 linha, 3 colunas, gráfico 1
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
plt.title("Irms 15 minutos fase A")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()


media = di2.mean()
maximo_total = di2.max()
minimo_total = di2.min()

# Criar boxplot com média
plt.subplot(1, 3, 2)  # 1 linha, 3 colunas, gráfico 2
box = plt.boxplot(di2, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Irms 15 minutos fase B")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()

media = di3.mean()
maximo_total = di3.max()
minimo_total = di3.min()

# Criar boxplot com média
plt.subplot(1, 3, 3)  # 1 linha, 3 colunas, gráfico 3
box = plt.boxplot(di3, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Irms 15 minutos fase C")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()

di1 = df["Irms1"].round(2)
di2 = df["Irms2"].round(2)
di3 = df["Irms3"].round(2)

media = di1.mean()
maximo_total = di1.max()
minimo_total = di1.min()

plt.figure(figsize=(12, 6))

# Criar boxplot com média
plt.subplot(1, 3, 1)  # 1 linha, 3 colunas, gráfico 1
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
plt.title("Irms 15 minutos fase A")
plt.ylabel("Irms")
plt.legend(loc='upper right')


media = di2.mean()
maximo_total = di2.max()
minimo_total = di2.min()

# Criar boxplot com média
plt.subplot(1, 3, 2)  # 1 linha, 3 colunas, gráfico 2
box = plt.boxplot(di2, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Irms 15 minutos fase B")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()
#plt.show()

media = di3.mean()
maximo_total = di3.max()
minimo_total = di3.min()

# Criar boxplot com média
plt.subplot(1, 3, 3)  # 1 linha, 3 colunas, gráfico 3
box = plt.boxplot(di3, patch_artist=True, widths=0.5, showmeans=True)

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
plt.title("Irms 15 minutos fase C")
plt.ylabel("Irms")
plt.legend(loc='upper right')
plt.tight_layout()
plt.show()


outlierdt1 = [t for t in dt1 if t > maximo_quadradodt1]
outlierdt2 = [t for t in dt2 if t > maximo_quadradodt2]
outlierdt3 = [t for t in dt3 if t > maximo_quadradodt3]


print(f'Máximo tempo no quadrado para 15 minutos fase A: {maximo_quadradodt1}')
print(f'Porcentagem de outlier para 15 minutos fase A: {(len(outlierdt1) / len(dt1)) * 100:.2f}%')

print(f'Máximo tempo no quadrado para 15 minutos fase B: {maximo_quadradodt2}')
print(f'Porcentagem de outlier para 15 minutos fase B: {(len(outlierdt2) / len(dt1)) * 100:.2f}%')

print(f'Máximo tempo no quadrado para 15 minutos fase C: {maximo_quadradodt3}')
print(f'Porcentagem de outlier para 15 minutos fase C: {(len(outlierdt2) / len(dt1)) * 100:.2f}%\n\n')


ciclo = 60 * 60 * 15
total = len(di1)
perdas = ((ciclo-total)/ciclo) * 100
print(f'60 ciclos por segundo x 15 minutos: {ciclo}')
print(f'Total de Irms calculos em 15 minutos: {total}')
print(f'Porcentagem de amostras perdidas em 15 minutos: {perdas:.2f} %\n')
