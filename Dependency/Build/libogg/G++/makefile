CFlags = -Wall -Wfatal-errors -I../../../libogg/include/
CC = gcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = gcc -g $(CFlags) 
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libogg
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath= ../../../libogg/src/
Sources = bitwise.c
Sources += framing.c
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
