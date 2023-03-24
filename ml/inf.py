import torch
import numpy as np
import torch.optim as optim
from torch import nn, optim
import torch.nn as nn
import mido
import torch.nn.functional as f
import numpy as np
import os
from fastapi import FastAPI
import uvicorn
from fastapi import Request, FastAPI, Response

app = FastAPI()

def normalise_midi(notes, bars=1, lines=128, beat_length=16, tempo=120):
	beats_sec = tempo / 60
	bar_time = beats_sec * bars * beat_length
	counter = 0
	final_notes = []
	tick_length = bar_time / lines	
	current_tick = 0

	while counter != lines - 1:
		note_added = False
		for note in notes:
			freq = note.split(";")[0]
			time = note.split(";")[1]
			if current_tick + tick_length >= float(time):
				final_notes.append(float(freq))
				notes.remove(note)
				note_addded = True
				break
		if not note_added:
			final_notes.append(0)

		current_tick += tick_length
		counter += 1

	return final_notes

def normalise_output(onotes, threshold=10, bars=1, lines=128, beat_length=16, tempo=120, loopLength=2 * 16):
	notes = ""
	beats_sec = tempo / 60
	bar_time = beats_sec * bars * beat_length
	current_tick = 0
	tick_length = bar_time / lines	
	current_tick = 0
	counter = 0

	while counter != lines - 1:
		if current_tick <= float(abs(loopLength)):
			note = onotes[counter]
			if note  * 10 <= threshold:
				pass
			else:
				notes += str(note * 10) + ";" + str(current_tick) + "\n"
		current_tick += tick_length
		counter += 1

	return notes


@app.get("/")
async def root():
    return {"message": "Hello World"}

@app.post('/infer')
async def infer(request: Request):
	data =  await request.json()
	midi = data.get('midi')
	bars = data.get('bars', 1)
	print(data)
	loopLength = data.get('loopLength', 2 * 16)
	notes = midi.split("\n")
	fnotes = []
	for note in notes:
		if note.strip() != "":
			fnotes.append(note)
	normalised = normalise_midi(fnotes, bars)
	normalised = normalised[0:128]
	print(normalised)
	a = np.array(normalised, dtype='float32')
	t = torch.from_numpy(a)
	model = torch.load("model.pt")
	model.eval()
	res = model(t)
	print(res.tolist())
	res2 = normalise_output(res.tolist(), loopLength=loopLength)
	print(res2)
	return Response(content=res2, media_type="text/plain")



if __name__ == "__main__":
    uvicorn.run(app, host="0.0.0.0", port=3000)