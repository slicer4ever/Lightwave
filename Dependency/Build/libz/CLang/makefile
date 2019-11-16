CFlags = -Wall -Wfatal-errors -I../../../libz/
CC = clang $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = clang -g $(CFlags)
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libz
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../libz/
Sources = adler32.c
Sources += compress.c
Sources += crc32.c
Sources += deflate.c
Sources += gzclose.c
Sources += gzlib.c
Sources += gzread.c
Sources += gzwrite.c
Sources += infback.c
Sources += inffast.c
Sources += inflate.c
Sources += inftrees.c
Sources += trees.c
Sources += uncompr.c
Sources += zutil.c
T = $(Sources:.c=.o)
Objs = $(addprefix $(ObjPath), $(T))
all: Dirs $(Objs)
	ar crf $(Target)$(TargetLibName).a $(Objs)
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.o: $(SrcPath)%.c
	mkdir -p $(dir $@)
	$(CC) -c  $(subst $(ObjPath),$(SrcPath), $(basename $@)).c -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
