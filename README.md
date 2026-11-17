# Acoustic 260 (VST3)

Behavioural model of the Acoustic Control Corp. Model 260 guitar head and 261 cabinet,
built from the 260/160 schematic. Chain: input buffer -> Volume + Bright -> gain stages ->
Treble/Bass -> Variamp resonance (1.5 H + selectable cap) -> channel level -> Fuzz ->
spring-style Reverb -> Tremolo (UJT oscillator / lamp / LDR) -> solid-state power amp -> 2x15 + horn cab.
The whole amp runs 4x oversampled.

## Build (GitHub Actions)
1. Create a new private repo and push this folder to it.
2. Open the Actions tab -> "Build VST3" -> wait for the run to finish (first run fetches JUCE, ~5-8 min).
3. Download the `Acoustic260-VST3` artifact, unzip, copy `Acoustic 260.vst3` to
   `C:\Program Files\Common Files\VST3\`, rescan plugins.

## Notes
- Values read from the schematic: Variamp 1.5 H with .18/.047/.012/.0027 uF = 306/599/1186/2501 Hz,
  25k treble/bass pots, 500k fuzz Attack/Gain, 50k volume pots, 70 V power-amp rail.
- Estimated (not solvable from the scan): tone-stack corner frequencies, fuzz transistor bias,
  tremolo timing capacitor, cabinet voicing. Tremolo is capped at ~80-90% depth per Acoustic's own claim.
- `test/test.cpp` is a standalone DSP check (g++ -O2 -std=c++17 test/test.cpp).
