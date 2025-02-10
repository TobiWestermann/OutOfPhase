# Out of Phase

- Bearbeitung auf Frequenzebene, also FFT des Signals [und dann np.angle()] und dann stuff damit machen und irgendwie wieder zusammenführen mit IFFT...oder so

**Phase Spectrum Analyzer**: Display zum Anzeigen der Phase, FFT und dann Phase davon, 1024 / 2048 Blöcke bspw.,  möglicherweise einstellbar
  - Eingang vs. Ausgang (nochmal FFT, vlt auch anders möglich, Bearbeitung auf Freqebene, 2te FFT nicht nötig) darstellen
  - evtl. TF darstellen (LTI Ja / Nein? Eher Nein)
  - Anzeige in Bändern evtl.

**Knöpfe**: Parameter / Einstellungen
  - Random Phase: Blockweise randomisierung der Phase (Verteilung der Randomisierung einstellbar machen?)
  - Phase to Zero: Phase auf 0 setzen für alle freqs
  - Phase Frost: Phasenspektrum des aktuellen Blocks wird auf Knopfdruck eingeforen
  - Phase Flip: *-1, selbsterklärend
  - (Feedback: Ausgang auf Eingang zurückführen) (???)
  - Mix: Dry / Wet, also bearbeitetes Signal und Originalsignal mixen

**Extra Features**:
  - Bearbeitung in Bändern, möglicherweise in Tabs oder mit Multibuttons
  - Bzw. Bearbeitung eines Bandes im Signal, trotzdem Gesamtsignal ausgeben <br>
    $\rightarrow$ Bearbeitung in entsprechenden FFT-Bins
  - Latenzanzeige

### Ablaufplan:
1. Logik schrittweise in Python implementieren
   - Einzelne Features anhand eines Audiosignals testen
2. Umsetzung in C++
3. GUI

**Aufteilung grob**:
- Python und maths verstehen: Malte
- GUI: Benno
- FFT in C++: Tobi (Wie WOLA nutzen, wie da drin FFT nutzen? Am Ende eine for Schleife in ProcessSynchronBlock die über Blöcke (FreqBins) iteriert)
