README
======

With JBD you try to build a MIDI inference engine between two instruments, say guitar and drums and interact with the inference engine from your DAW with the help of MIDI.

Partially working. The current setup can send MIDI data to the ML sever and get some trained data back.
Playback of the MIDI data doesn't work.

SETUP
=====

```
cd vst
git clone JUCE
cmake -B cmake-build-install 
cmake --build cmake-build-install 
cd ml
pip install -r requirements.txt
```

Training
========

1. Split stems
2. Get MIDI from stems
3. Rename midi to a.mid and b.mid and move it to ml
4. python bar_fit.py --tempo --signature --ticks-per-beat --bar
5. python inf.py to run inference server on localhost:3000


VST
===

1. Create some MIDI data and loop over a region
2. Click infer
3. See the new MIDI data that is got from the server



