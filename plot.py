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
        dummy = (math.sin(i*delta)+ 1.0)        # sine wave

        for()
        data.append(dummy)                      # different waveform

    return data,x_range

def plot():
    arr,x = generate_wave(50)
    plt.title('Waveform')
    plt.scatter(x, arr)
    plt.show()
    

if __name__ == "__main__":
    plot()


'''
Sine Wave: 
delta=(2.0*math.pi)/interval
dummy = (math.sin(i*delta)+ 1.0)      

Square Wave:
delta=(2.0*math.pi)/interval
if(i<interval/2):
    dummy = 2
else:
    dummy = 0

Triangle Wave:

Sawtooth:
if(i<interval/4):
    dummy = i*delta+1.0
elif(i>=interval/4 and i<=3*interval/4):
    dummy = i*delta-1.0
else:
    dummy = (i*delta+1.0) - 5.0

'''