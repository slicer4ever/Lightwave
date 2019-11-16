CFlags = -Wall -Wfatal-errors -I../../../libvorbis/include/ -I../../../libogg/include/
CC = gcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = gcc -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libvorbis
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../libvorbis/lib/
Sources = analysis.c
Sources += barkmel.c
Sources += bitrate.c
Sources += block.c
Sources += codebook.c
Sources += envelope.c
Sources += floor0.c
Sources += floor1.c
Sources += info.c
Sources += lookup.c
Sources += lpc.c
Sources += lsp.c
Sources += mapping0.c
Sources += mdct.c
Sources += psy.c
Sources += registry.c
Sources += res0.c
Sources += sharedbook.c
Sources += smallft.c
Sources += synthesis.c
Sources += tone.c
Sources += vorbisfile.c
Sources += window.c
T = $(Sources:.c=.o)
Objs = $(addprefix $(ObjPath),$(T))
all: Dirs $(Objs)
	ar crf $(Target)$(TargetLibName).a $(addprefix $(ObjPath), $(notdir $(Objs)))
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.o: $(SrcPath)%.c
	mkdir -p $(dir $@)
	$(CC) -c  $(subst $(ObjPath),$(SrcPath), $(basename $@)).c -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
