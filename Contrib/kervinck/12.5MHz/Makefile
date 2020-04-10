#-----------------------------------------------------------------------
#
#	Makefile for Gigatron ROM and tooling
#
#-----------------------------------------------------------------------

# Allow application-specific SYS extensions to live in Apps/
export PYTHONPATH:=Core:Apps/Loader:Apps/Racer
export PYTHONDONTWRITEBYTECODE:=please

ifdef OS # Windows
 export PYTHONPATH:=$(subst :,;,$(PYTHONPATH))
endif

#-----------------------------------------------------------------------
#	Experimental and temporary (to be removed)
#-----------------------------------------------------------------------

# Adds vCPU slices on scanlines to get 400 cycles per scanline
ROMv3y.rom:
	cd ../../.. && \
	python3 Contrib/kervinck/12.5MHz/ROMv3y.asm.py\
		Apps/Snake/Snake_v2.gcl\
		Apps/Racer/Racer_v1.gcl\
		Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Apps/Pictures/Pictures_v2.gcl\
		Apps/Credits/Credits_v2.gcl\
		Apps/Loader/Loader_v2.gcl\
		Apps/Tetronis/Tetronis_v1.gt1\
		Apps/Bricks/Bricks_v1.gt1\
		Apps/TinyBASIC/TinyBASIC_v2.gcl\
		Apps/WozMon/WozMon_v2.gt1\
		Egg=Apps/Sprites/Sprites_v1.gt1\
		Main=Apps/MainMenu/MainMenu_v3.gcl\
		Contrib/kervinck/12.5MHz/Reset_v3y.gcl

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------
# vi: noexpandtab
