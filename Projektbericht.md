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
Die Methode nutzt SIMD-Befehle (Single Instruction Multiple Data) um 8 Pixel gleichzeitig zu verarbeiten. Dazu werden
zunächst 2 * 16 Byte an RGB-Daten geladen, diese nach Farben sortiert, auf 8 Werte beschnitten und in einzelne Register
isoliert. Mittels Fix-Komma-Berechnung werden die Koeffizienten auf die Farbwerte angewandt und die Summe der 3 Register
als Grauwert gespeichert.

#### 2.1.3 Implementierung mit Bit-shift
Statt der präzisen Werte wird 0.25 (0.299) für Rot, 0.5 (0.587) für Grün und 0.25 (0.144) für Blau verwendet, die mit
simplen Shifts angewandt werden können. Eine einfache Implementierung, die je nach Anwendungszweck ordentliche
Ergebnisse liefert.

### 2.2 Sobel-Filter
#### 2.2.1 Referenzimplementierung (naiv) - V0
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

#### 2.2.2 Implementierung mit Kernel Unroll - V1
Wir haben festgestellt, dass die vertikale Matrix eine Spalte mit Nullen und die horizontale Matrix eine Zeile mit Nullen enthält.
Daher können wir uns bei der Multiplikation jeder Matrix mit den Bildpixeln die Berechnung dieser Nullen sparen.
Außerdem konnten wir erkennen, dass die verbleibenden Werte beider Matrizen auf die Konstanten 1, -1, 2 und -2 beschränkt sind.
Daher haben wir die Iterationen über beide Matrizen optimiert, indem wir diese fixen Werte direkt genutzt haben.

#### 2.2.3 Implementierung mit separierten Filtern - V2
Hierbei handelt es sich um eine Implementierung des Sobel-Filters mithilfe von separierten Filtern.
Dabei wird in die Anzahl der nötigen Operationen bei einem 2D-Bildfilter reduziert, indem der Filter in zwei 1D-Filter aufgeteilt wird.
So kann von MxN Operationen auf M+N Operationen reduziert werden.
Dies ist hier ein wenig langsamer bei unserem Beispiel, da wir nur 3x3 Filter verwenden, der Vorteil wird bei größeren Filtern deutlicher.

#### 2.2.4 SIMD-Implementierung - V3
Die Methode basiert auf dem obigen Algorithmus mit kernel unroll und nutzt zusätzlich SIMD-Instruktionen zur Berechnung.
Mithilfe von SIMD werden 8 Pixel gleichzeitig verarbeitet und somit die größten Zeitersparnisse beim Anwenden des Sobel-Filters erzielt.
Überlappungen in nicht zur Bildzeile gehörende Speicherbereiche wurden mittels 0-Masken ignoriert.

#### 2.2.5 Implementierung mit LUT - V4
Diese Implementierung basiert wieder auf der naiven Implementierung des Sobel-Filters.
Ausnahme dabei ist, dass hierbei eine Lookup-Tabelle genutzt wird, um die Quadratwurzel zu berechnen, anstatt die Wurzelfunktion aus der C-Bibliothek zu nutzen.
Alle Quadratwurzeln mit den Werten 0 bis 254 wurden vorab berechnet und in einer Lookup-Tabelle gespeichert.
An dieser Stelle kann ebenso erwähnt werden, dass eine weitere Form der Quadratwurzel implementiert wurde, diese approximate das Ergebnis und nutzt nur simple mathematische Operationen.
Beide Versionen lieferten enorme Performanzeinbußen und wurden in Bezug auf die Optimierung nicht mehr verwendet.

## 3 Performanzmessungen
### 3.1 Messumgebung
Getestet wurde auf einem System mit einem AMD Ryzen 5 4680U Prozessor, 2.20 GHz, 8 GB Arbeitsspeicher, Ubuntu 24.10, 64 Bit, Linux-Kernel 6.11.
Kompiliert wurde mit GCC 13.3.0 mit der Option -O2.

### 3.2 Methodik
Die Berechnungen wurden mit Eingabegrößen von 640×426 bis 5184×3456 Pixeln jeweils 1000-mal durchgeführt und das arithmetische Mittel für jede Eingabegröße wurde in folgendes Diagramm eingetragen.

### 3.3 Ergebnisse
<img src="Sobel_Benchmark.png" alt="Sobel Filter Result" width="512">

In unserem Testrahmen mit der Referenzimplementierung als Standard, konnten wir feststellen, dass die SIMD-Implementierung, wie erwartet, für relevante Bildgrößen die schnellste ist.
Die spezielleren Versuche den Algorithmus zu verbessern, haben zu eher kleineren Verbesserung der Laufzeit geführt (zumindest bei derzeitiger Methodik).
Lookup-Tabellen hingegen haben unsere Laufzeit deutlich verschlechtert.

## 4 Anteile der einzelnen Projektmitglieder
### 4.1 Tobias Langer
- Refactoring und generelle Code-Verbesserungen
- Implementierung von Sobel mit separierten Filtern
- Implementierung von Graustufen-Konvertierung und Sobel-Filter mit SIMD mit 8 Pixeln
- Erstellen von Grafiken für die Präsentation
- Strukturierung der Tests in Form eines simplen Frameworks

### 4.2 Luca Tänzler
- Erste Zeitmessung für Sobel-Filter und Graustufen-Konvertierung
- Implementierung einer simplen Quadratwurzel-Funktion
- Verfassen des Projektberichts und der Präsentationsfolien
- Durchführung der Performanzmessungen

### 4.3 Salim Daoud
- Erste naive Sobel Implementierung mit Makefile und Argumenten-Verarbeitung
- Graustufen-Konvertierung mit SIMD und paralleles Lesen und Schreiben
- Erste Implementierung von Sobel SIMD und kernel unroll
- Tests zum Prüfen von Korrektheit