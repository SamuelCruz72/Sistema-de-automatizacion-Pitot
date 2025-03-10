import os
import re
import pandas as pd
import matplotlib.pyplot as plt

carpeta_logs = "logs"

# Obtener el archivo más reciente en la carpeta
archivos = [os.path.join(carpeta_logs, f) for f in os.listdir(carpeta_logs) if f.endswith(".log")]

# Leer el archivo y extraer datos
archivo = max(archivos, key=os.path.getmtime) 
patron = re.compile(r"(\d{2}:\d{2}:\d{2}\.\d{3}) > .*? (\d+\.\d+)")
N = 33  # Número de líneas de encabezado a eliminar
datos = []
with open(archivo, "r") as file:
    lineas = file.readlines()[N:]  # Salta las primeras N líneas

for par in lineas:
    match = patron.search(par)
    if match:
        tiempo = match.group(1)  # Extrae el tiempo
        medicion = float(match.group(2))  # Extrae la medición
        datos.append((tiempo, medicion))

# Convertir a DataFrame
df = pd.DataFrame(datos, columns=["Tiempo", "Medición"])

# Convertir el tiempo a formato datetime para graficar correctamente
df["Tiempo"] = pd.to_datetime(df["Tiempo"], format="%H:%M:%S.%f")

# Graficar
plt.figure(figsize=(10, 5))
plt.plot(df["Tiempo"], df["Medición"], marker="o", linestyle="-")
plt.xlabel("Tiempo")
plt.ylabel("Medición")
plt.title("Mediciones vs Tiempo")
plt.xticks(rotation=45)
plt.grid()
plt.show()