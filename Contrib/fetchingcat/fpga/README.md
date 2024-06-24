

### FPGA ###

This is an FPGA recreation of the gigatron.  I have currently tested it on Nexys A7 and Basys 3 but it should work on most anything
if willing to change out the Vivado IP BRAM and Clocking Wizard for vendor specific or custom implementations.  I will likely add a couple more boards if I find the time. This is my first version and not optimized yet but is already tiny (around 150 LUTs). The recreation while being cycle accurate I believe loses some of the beauty of the design, especially around the ALU. But it is a logical and functional recreation. 

In order to build the project open vivado and execute these commands in the CLI window

```
cd /git/gigatron-rom/Contrib/fetchingcat/fpga  
```

Then execute one of these depending on your board:  

```
source gigatron_basys.tcl  
```

```
source gigatron_nexys.tcl   
```

![gigatronFPGA](https://github.com/fetchingcat/gigatron-rom/assets/61957768/dacd3771-114f-44f1-9e8f-2d72e1c47f33)

The controller input is using the onboard buttons and switches and an attempt at audio using the onboard nexys a7 mono out.  The audio is not perfect.




