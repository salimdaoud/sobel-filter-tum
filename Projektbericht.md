### <center> LEHRSTUHL FÜR RECHNERARCHITEKTUR UND PARALLELE SYSTEME 
# <center> Grundlagenpraktikum: Rechnerarchitektur - Sobel-Filter (A201)

---

### <center> Wintersemester 24/25 - Abschlussprojekt t001
<div style="text-align: center;"> Tobias Langer - Luca Tänzler - Salim Daoud </div>

---
## 1 Aufgabenstellung
Im Rahmen des Praktikums soll ein Sobel-Filter für die Bildverarbeitung implementiert werden.
Ziel der Aufgabe ist es, ein farbiges Bild zunächst in ein Graustufenbild umzuwandeln und anschließen mithilfe des Sobel-Filters Kanten hervorzuheben.
Die Implementierung soll dabei in C erfolgen und die Performanz der verschiedenen Implementierungen mithilfe von Benchmarks verglichen werden.
Außerdem muss ein Rahmenprogramm geschrieben werden, welches Optionen für das Programm entgegennimmt und sich auch um Fehlerbehandlung kümmert.

## 2 Überblick über die Implementierungen
### 2.1 Graustufen-Konvertierung
#### 2.1.1 Referenzimplementierung (naiv) 
Ein Pixel (dargestellt durch einen Vektor aus den drei Farbkanälen Rot (R), Grün (G) und Blau (B))
wird in Graustufen umgewandelt, indem der gewichtete Durchschnitt D mittels der Koeffizienten a, b und c berechnet wird:
$$
D = \frac{a \cdot R + b \cdot G + c \cdot B}{a + b + c}
$$

#### 2.1.2 SIMD-Implementierung
Die Methode nutzt SIMD-Befehle (Single Instruction Multiple Data) um 4 Pixel gleichzeitig zu verarbeiten und dadurch die Rechenzeit zu optimieren.
Die zu grunde liegende Formel ist dieselbe wie bei der Referenzimplementierung.
Pixel, die nicht durch 4 teilbar sind, werden einzeln in einer zusätzlichen Schleife verarbeitet.

#### 2.1.3 Implementierung mit Bit-shift
Wir haben versucht mithilfe von Bit-shifts die Effizienz der Berechnungen zu verbessern.
Dabei haben wir festgestellt, dass die Multiplikation der Pixelwerte mit den idealen Koeffizienten durch eine Approximation mittels Bit-shifts dargestellt werden kann.

### 2.2 Sobel-Filter
#### 2.2.1 Referenzimplementierung (naiv)
Der Sobel-Filter wird in mehreren Stufen angewandt.
Zunächst wird das Graustufenbild Q getrennt nach vertikalen (v) und nach horizontalen (h) Kanten gefiltert: 
$$
Q^v = M^v \ast Q \quad Q^h = M^h \ast Q
$$
mit den Faltungsmatrizen $M^v$, $M^h$ der Dimension 3:
$$ 
M^v = \begin{pmatrix} 1 & 0 & -1 \\ 2 & 0 & -2 \\ 1 & 0 & -1 \end{pmatrix} \quad M^h = \begin{pmatrix} 1 & 2 & 1 \\ 0 & 0 & 0 \\ -1 & -2 & -1 \end{pmatrix} 
$$
Somit lässt sich die Berechnung des neuen Graustufenpixels mittels der Faltung folgendermaßen darstellen (i und j bezeichnen wie gewohnt die (Null-basierte) Reihe und Spalte einer Matrix):
$$
Q^d_{(x, y)} = \sum_{i=-1}^{1} \sum_{j=-1}^{1} M^d_{(1+i, 1+j)} \cdot Q_{(x+i, y+j)} \quad d \in \{v, h\}
$$
Sind $Q^v$ und  $Q^h$ berechnet worden, kombiniert man beide Bilder pixelweise zum Gesamtbild $Q'$:
$$ 
Q'_{(x, y)} = \text{clamp}_{0}^{255} \left(\sqrt{\left(Q^v_{(x, y)} \right)^2 + \left(Q^h_{(x, y)} \right)^2} \right) 
$$

### 2.2.2 sobel_squareroot_lookup_V1
Dies ist erneut die naive Implementierung des Sobel-Filter-Algorithmus, allerdings wird hier eine Lookup-Tabelle verwendet, um Quadratwurzeln zu berechnen, anstatt die Wurzelfunktion aus der C-Bibliothek zu nutzen. Alle Quadratwurzeln für Werte zwischen 0 und 255 wurden vorab berechnet und in einer Lookup-Tabelle gespeichert.
### 2.2.3 sobel_kernel_unroll_V2
Hier haben wir festgestellt, dass die vertikale Matrix eine Spalte mit Nullen und die horizontale Matrix eine Zeile mit Nullen enthält. Daher konnten wir uns bei der Multiplikation jeder Matrix mit den Bildpixeln die Berechnung dieser Nullen sparen. Außerdem haben wir festgestellt, dass die verbleibenden Werte beider Matrizen auf die Konstanten 1, -1, 2 und -2 beschränkt sind.

Daher haben wir die Iterationen über beide Matrizen optimiert, indem wir diese fixen Werte direkt genutzt haben. Darüber hinaus wurde die Multiplikation mit 2 durch einen Shift-Left-Operator ersetzt und die Multiplikation mit -1 durch eine einfache Vorzeichenumkehr optimiert.
### 2.2.4 sobel_SIMD_V3
Sobel_SIMD basiert auf dem Algorithmus von Sobel_kernel_unroll, nutzt jedoch SIMD-Instruktionen zur Berechnung. Mit SIMD werden 4 Pixel gleichzeitig parallel verarbeitet. Falls die Anzahl der Pixel nicht durch 4 teilbar ist, wird die letzte Zeile separat ohne Verwendung von SIMD-Instruktionen verarbeitet.
### 2.2.5 sobel_separated_convolution_V4
Die Funktion `sobel_separated_convolution_V4` bietet eine effiziente Implementierung des Sobel-Filters durch die Kombination von separierter Faltung, optimierter Schleifenstruktur und dynamischem Speichermanagement.
# 3 Dokumentation der Messumgebung und Ergebnisse
## 3.1 Dokumentation der Messumgebung
Getestet wurde auf einem System mit einem Intel i5-8250U Prozessor, 1.60 GHZ, 3,7 GiB Arbeitsspeicher, Ubuntu 24.04.1 LTS, 64 Bit, Linux-Kernel 6.8.0-51-generic kompiliert wurde mit gcc 13.3.0 mit der Option -O3.
## 3.2 Ergebnisse
[Performance-Sobel-function.png](https://postimg.cc/Mfw0WDkk)
# 4 Persönliche Anteile der einzelnen Projektmitglieder

