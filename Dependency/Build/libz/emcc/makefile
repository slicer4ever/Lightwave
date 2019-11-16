CFlags = -Wall -Wfatal-errors -I../../../libz/
CC = emcc $(CFlags) -O2
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = emcc -g $(CFlags)
endif
PlatformTarget = JS
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
	emcc $(Objs) -o $(Target)$(TargetLibName).bc
Dirs:
	mkdir -p $(ObjPath)
	mkdir -p $(Target)
$(ObjPath)%.bc: $(SrcPath)%.c
	mkdir -p $(dir $@)
	$(CC) $(subst $(ObjPath),$(SrcPath), $(basename $@)).c -o $@
clean:
	rm -rf $(ObjPath)
	rm -f $(Target)$(TargetLibName).a
