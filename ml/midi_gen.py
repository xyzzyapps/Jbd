import time
import rtmidi

midiout = rtmidi.MidiOut()
available_ports = midiout.get_ports()
print(available_ports)

if available_ports:
    midiout.open_port(1)

tempo = 120
lines = 128
beat = tempo / 60.0
ticks = 0

def secondsToTicks(seconds):
    line_time = beat / lines
    ticks = 0
    while seconds >= 0:
        ticks += 1
        seconds -= line_time

    return ticks

def ticksToSeconds(ticks):
    line_time = beat / lines
    return ticks * line_time
    
    
def extract_bars(midi_file,track_no=0,beat=4, tempo=120, ticks_beat=960):
    mid = mido.midifile(midi_file)

    beat_time = beat * ticks_beat

    track = mid.tracks[track_no]
    time = 0
    bars = 1
    current_bar = {}
    all_bars = []
    for msg in track:
        if not msg.is_meta:
            time += msg.time
            if time <= (bars * beat_time):
                current_bar[time % beat_time] = msg.note
            else:
                bars += 1
                all_bars.append(normalise_bar(current_bar, beat, tempo, ticks_beat))
                current_bar = {}
                current_bar[time % beat_time] = msg.note
    return all_bars


with midiout:
    while 1:
        r = int(ticks % lines)
        if r % int(lines / 4) == 0: 
            note_on = [0x90, 60, 112] # channel 1, middle C, velocity 112
            print(note_on)
            midiout.send_message(note_on)
        #if r in [3, 7, 11, 15]:
        #    note_off = [0x80, 60, 0]
        #    midiout.send_message(note_off)
        time.sleep(beat / lines)
        ticks += 1
        seconds = ticksToSeconds(ticks)
        print(secondsToTicks(seconds))


del midiout