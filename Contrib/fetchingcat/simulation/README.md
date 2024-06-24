

![GigaTronDigital2](https://github.com/fetchingcat/gigatron-rom/assets/61957768/ab7bc01f-d568-4da6-a76b-e6fd36c105d5)

### Simulator ###

This is a simulated digital logic recreation of the gigatron based on the gigatron [schematics](https://cdn.hackaday.io/files/20781889094304/Schematics%202020-03-20.pdf)  
This simulation runs using [Hneemann's Digital Logic Simulator](https://github.com/hneemann/Digital).  
This project was used as a map for helping creating the FPGA version, though I found I mostly implemented it without being a chip for chip recreation.  
This project drew inspirations from monsonite's much more aesthetically pleasing version [here](https://github.com/monsonite/Gigatron-Simulator)  

I worked to try to be as accurate as possible and as such you could use this to help diagnose or just investigate the inner workerings closer.  
In order to get the VGA output to render correctly I needed to modify the Digital simulator by adding the VGA timings its using.  
I currently have a custom build deployed [here](https://github.com/fetchingcat/Digital/releases/tag/patched_release).   
Otherwise you can use the normal Digital however it will generate an error once it starts to try to render a VGA screen.  


The clocking works by having 4 clock steps for the VGA pixel clock and 1 step for the gigatron.  
This gives us the 4:1 ratio needed to simulate 25mhz and 6.25mhz. 
There is also a simple hardware breakpoint wired in to a comparator.  
I added input switches, keep in mind you to wait to toggle the inputs due to the simulation speed

Just hit the play button and be patient as it takes a minute or two to load.  

![GigaTronDigital](https://github.com/fetchingcat/gigatron-rom/assets/61957768/7c7539ab-083d-4d7b-a9ca-0bb86b942c56)
