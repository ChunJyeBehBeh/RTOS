import numpy as np
import matplotlib.pyplot as plt 
import math

def generate_wave(interval):
    '''
    Interval: 
    '''
    delta=(2.0*math.pi)/interval				# 2pi/interval
    data = []
    x_range = [j for j in range(interval)]
    for i in range(interval):
        dummy = (math.sin(i*delta)+ 1.0)        # <--- change here to generate 
        data.append(dummy)                      # different waveform
    return data,x_range

def plot():
    arr,x = generate_wave(50)
    plt.title('sine wave')
    plt.scatter(x, arr)
    plt.show()
    

if __name__ == "__main__":
    plot()