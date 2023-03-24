import torch
import numpy as np
import torch.optim as optim
from torch import nn, optim
from torch.autograd.variable import variable
import torch.nn as nn
import mido
import torch.nn.functional as f
from torch.utils.data import tensordataset
from torch.utils.data import dataloader
import numpy as np

def fit(num_epochs, model, loss_fn, opt, train_dl):
    
    # repeat for given number of epochs
    for epoch in range(num_epochs):
        
        # train with batches of data
        for xb,yb in train_dl:
            
            # 1. generate predictions
            pred = model(xb)
            
            # 2. calculate loss
            loss = loss_fn(pred, yb)
            
            # 3. compute gradients
            loss.backward()
            
            # 4. update parameters using gradients
            opt.step()
            
            # 5. reset the gradients to zero
            opt.zero_grad()
        
        # print the progress
        if (epoch+1) % 10 == 0:
            print('epoch [{}/{}], loss: {:.4f}'.format(epoch+1, num_epochs, loss.item()))


def normalise_bar(bar,beat=4, tempo=120, ticks_beat=960):
    i = 0
    beat_time = beat * ticks_beat
    arr = []
    while i <= beat_time:
        arr.append([i, bar.get(i,0)])
        i += 1

    i = 0
    farr = []
    while i < 32 * beat:
        farr.append(arr[i * 30][1])
        i += 1

    return farr


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

beat_time = 120 * 960


class net(nn.module):
    def __init__(self):
      super(net, self).__init__()

      # self.conv1 = nn.conv1d(beat_time, 32, 4, stride=1)
      self.fc1 = nn.linear(128, 128)

    def forward(self, data):
      # pass data through conv1
    #  data = self.conv1(data)
      data = self.fc1(data)
  #    output = f.tanh(data, dim=1)
      return data

a_bars = extract_bars("a.mid")
b_bars = extract_bars("b.mid")
a_bars_final = a_bars[0:len(b_bars)]
print(len(a_bars_final))
print(len(b_bars))

inputs = np.array(a_bars_final, dtype='float32')
targets = np.array(b_bars, dtype='float32')
inputs = torch.from_numpy(inputs)
targets = torch.from_numpy(targets)

model = net()

for name, param in model.named_parameters():
    print(name, param.shape)

opt = torch.optim.sgd(model.parameters(), lr=1e-5)
loss_fn = f.mse_loss

train_ds = tensordataset(inputs, targets)
batch_size = 4
train_dl = dataloader(train_ds, batch_size, shuffle=true)

example = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

example = np.array(example, dtype='float32')
example = torch.from_numpy(example)

fit(100, model, loss_fn , opt ,train_dl)

traced_script_module = torch.jit.trace(model, example)
traced_script_module.save("model.pt")