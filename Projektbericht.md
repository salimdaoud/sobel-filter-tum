### <center> LEHRSTUHL FÜR RECHNERARCHITEKTUR UND PARALLELE SYSTEME 
# <center> Grundlagenpraktikum: Rechnerarchitektur - Sobel-Filter (A201)

---

### <center> Wintersemester 24/25 - Abschlussprojekt t001
<div style="text-align: center;"> Tobias Langer - Luca Tänzler - Salim Daoud </div>

---
# 1 Aufgabenstellung
In diesem Projekt haben wir uns damit beschäftigt, Farbiger Bilder in Graustufen zu konvertieren und dann den Sobel-Filter Algorithmus darauf durchzuführen. Dieser wird verwendet, um Kanten in Bildern zu erkennen. Als Eingabe für den Sobel-Filter wird ein 24bpp PPM (P6) erwartet. Jeder Pixel in einem Bild wird dabei durch drei Bytes repräsentiert, die die Farbwerte für Rot (R), Grün (G) und Blau (B) enthalten. Durch die Berechnung eines gewichteten Durchschnitts D, wandeln wir zunächst die Bilder in Graustufen im 8bpp PGM (P5) Format um. Das Graustufenbild soll anschließend noch einer Kantenerkennung mittels des SobelFilters unterzogen werden.

# 2 Auflistung der Vergleichsimplementierungen
## 2.1 Graustufen-Konvertierung
### 2.1.1 Image to grayscale naive 
 Die Umwandlung eines Pixels in Graustufen geschieht durch das Berechnen eines gewichteten Durchschnitts D mittels der Koeffizienten a, b und c: 
$$
D = \frac{a \cdot R + b \cdot G + c \cdot B}{a + b + c}
$$
### 2.1.2 Image to grayscale using SIMD
Die Methode nutzt SIMD (Single Instruction Multiple Data), um mehrere Pixel gleichzeitig zu verarbeiten und dadurch die Rechenleistung zu optimieren. Sie verarbeitet 4 Pixel gleichzeitig mithilfe von SIMD-Befehlen, um parallele Berechnungen durchzuführen. Für jeden Pixel wird der Grauwert basierend auf der Formel berechnet:
$$
D = \frac{a \cdot R + b \cdot G + c \cdot B}{a + b + c}
$$
Pixel, die nicht durch 4 teilbar sind, werden einzeln in einer zusätzlichen Schleife verarbeitet. 
### 2.1.3 Image to Graysacle bitshift
Wir haben versucht, Bitshifts zu verwenden, um die Effizienz der Berechnungen zu erhöhen. Dabei haben wir festgestellt, dass die Multiplikation der Pixelwerte mit den idealen Koeffizienten durch eine Approximierung mittels Bitshifts dargestellt werden kann.

## 2.2 Sobel Filter
Das Graustufenbild soll anschließend noch einer Kantenerkennung mittels des SobelFilters unterzogen werden. Der Sobel-Filter wird in mehreren Stufen angewendet. Zunächst filtert man das Graustufenbild getrennt, einmal nach vertikalen (v), und einmal nach horizontalen (h) Kanten $$
Q^v = M^v \ast Q \quad Q^h = M^h \ast Q
$$
mit den Faltungsmatrizen Mv , Mh der Dimension 3:
$$ M^v = \begin{bmatrix} 1 & 0 & -1 \\ 2 & 0 & -2 \\ 1 & 0 & -1 \end{bmatrix} \quad M^h = \begin{bmatrix} 1 & 2 & 1 \\ 0 & 0 & 0 \\ -1 & -2 & -1 \end{bmatrix} $$
Somit lässt sich die Berechnung des neuen Graustufenpixels mittels der Faltung folgendermaßen darstellen (i und j bezeichnen wie gewohnt die (Null-basierte) Reihe und Spalte einer Matrix):
$$ Q^d(x, y) = \sum_{i=-1}^{1} \sum_{j=-1}^{1} M^d(1+i, 1+j) \cdot Q(x+i, y+j), \quad d \in \{v, h\} $$
Sind $Q^v$ und  $Q^h$ berechnet worden, kombiniert man beide Bilder pixelweise zum Gesamtbild $Q'$ :
$$ Q'(x, y) = \text{clamp}_{0}^{255} \left( \sqrt{ \left( Q^v(x, y) \right)^2 + \left( Q^h(x, y) \right)^2 } \right) $$

### 2.2.1 sobel_naive_V0
Diese ist eine Vergleichsimplementierung. Das ist nur eine naive Implementierung des Sobel-Filter Algorithmus und orientiert sich sehr stark an der mathematischen Definition.
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

