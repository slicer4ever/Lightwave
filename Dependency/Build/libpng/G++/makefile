CFlags = -Wall -Wfatal-errors -I../../../libpng/
CC = gcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = gcc -g $(CFlags)
endif
PlatformTarget = $(shell arch)
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

T = $(Sources:.c=.o)
Objs = $(addprefix $(ObjPath), $(T))
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
