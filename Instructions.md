# Build Guide

https://youtu.be/qIy4P3Xe6nA

# Install Guide

## Visual C++ Redistributable

Download and install (if not already):  
[Visual C++ Redistributable 2015-2022](https://aka.ms/vs/17/release/vc_redist.x64.exe)

## Download DLL dependencies

1. Download The Latest [PCILeech Release](https://github.com/ufrisk/pcileech/releases/latest). Download The win_x64 File.
![image](https://github.com/user-attachments/assets/4c209401-446c-48a1-af47-19d9be983b37)
**Unzip it in safe location.**  
     
3. Download the [FTD3xx Dlls](https://ftdichip.com/wp-content/uploads/2024/06/FTD3XXLibrary_1.3.0.10.zip).  

<p align="Left">
  <img src="https://github.com/IntelSDM/RustDMACheat/assets/86087830/3a26c1d1-689f-48a5-897a-ca873e74b89e"
    style="width: 75%;" />
</p>
<b>Unzip it in safe location.</b><br/><br/>  

## Add Environment Variables

You need to add absolute path to this two extracted folders to system's `PATH` environment variable, so cheat can find .dlls it need:  
  
Open Environment Variables:
<p align="Left">
  <img src="https://github.com/IntelSDM/RustDmaCheat/assets/86087830/a44e7eaa-f09d-4cf3-9c38-05a8658bb16c"
    style="width: 50%;" />
</p>  
<p align="Left">
  <img src="https://github.com/IntelSDM/RustDmaCheat/assets/86087830/1ec41330-8f64-4305-9214-d397f289d343"
    style="width: 50%;" />
</p>  

Select in lower half of the window `PATH` variable, click edit and add your absolute path to unzipped folders (then click ok on all windows):  
  
![image](https://github.com/user-attachments/assets/30dd6d23-abb5-46b4-8da1-4a479796cfeb)  
  
![image](https://github.com/user-attachments/assets/52429063-2f98-4420-9632-36e0d6cea333)  

> [!CAUTION]
> **Be careful, in FTD3XXLibrary you only need subfolder: /x64/DLL**

## Test you DMA card

Run `pcileech benchmark -device fpga` in CMD.  
  
For best experience you should have  
4000+ reads/s on 8 B  
150+ MB/s on 16 MB.  
