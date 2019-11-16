CFlags = -Wall -Wfatal-errors -I../../../libpng/
CC = emcc $(CFlags) -O2
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libpng
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../libpng/
Sources = png.c
Sources += pngerror.c
Sources += pngget.c
Sources += pngmem.c
Sources += pngpread.c
Sources += pngread.c
Sources += pngrio.c
Sources += pngrtran.c
Sources += pngrutil.c
Sources += pngset.c
Sources += pngtest.c
Sources += pngtrans.c
Sources += pngwio.c
Sources += pngwrite.c
Sources += pngwtran.c
Sources += pngwutil.c

T = $(Sources:.c=.bc)
Objs = $(addprefix $(ObjPath), $(T))
all: Dirs $(Objs)
	emcc $(addprefix $(ObjPath), $(notdir $(Objs)))-o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.c
	mkdir -p $(dir $@)
	$(CC) $(subst $(ObjPath),$(SrcPath), $(basename $@)).c -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
