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
    
    for i in range(int(interval/4)):
        dummy = (i*delta)*2
        data.append(dummy)                      # different waveform
    
    gap_1 = data[int(interval/4-1)]
    for i in range(int(interval/4),int(3*interval/4)):
        dummy = -2*gap_1+ (i*delta)*2
        data.append(dummy)                      # different waveform
    
    gap_2 = -2*gap_1+ ((3*interval/4)*delta)*2

    for i in range(int(3*interval/4),int(interval)):
        dummy = -2*gap_1+ (i*delta)*2 - gap_2
        data.append(dummy)                      # different waveform
    
    ''' Potential Bug
    print(data[59]/59*60)
    print(4.0*math.pi)
    '''
    return data,x_range

def plot():
    arr,x = generate_wave(60)
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