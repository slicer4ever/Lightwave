CFlags = -Wall -Wfatal-errors -I../../../libfreetype/include/ -DFT2_BUILD_LIBRARY
CC = gcc $(CFlags) -O3
Config = Release
debug ?= 0
ifeq ($(debug), 1)
	Config = Debug
	CC = gcc -g $(CFlags) -DFT_DEBUG_LEVEL_ERROR -DFT_DEBUG_LEVEL_TRACE
endif
PlatformTarget = $(shell arch)
Target = ../../../Binarys/$(Config)/$(PlatformTarget)/
TargetLibName = libfreetype
ObjPath = $(Config)/$(PlatformTarget)/
SrcPath = ../../../libfreetype/src/
Sources = autofit/autofit.c
Sources += base/ftbase.c
Sources += base/ftbbox.c
Sources += base/ftbitmap.c
Sources += base/ftdebug.c
Sources += base/ftfntfmt.c
Sources += base/ftfstype.c
Sources += base/ftgasp.c
Sources += base/ftglyph.c
Sources += base/ftgxval.c
Sources += base/ftinit.c
Sources += base/ftlcdfil.c
Sources += base/ftmm.c
Sources += base/ftotval.c
Sources += base/ftpatent.c
Sources += base/ftpfr.c
Sources += base/ftstroke.c
Sources += base/ftsynth.c
Sources += base/ftsystem.c
Sources += base/fttype1.c
Sources += base/ftwinfnt.c
Sources += bdf/bdf.c
Sources += cache/ftcache.c
Sources += cff/cff.c
Sources += cid/type1cid.c
Sources += gzip/ftgzip.c
Sources += lzw/ftlzw.c
Sources += pcf/pcf.c
Sources += pfr/pfr.c
Sources += psaux/psaux.c
Sources += pshinter/pshinter.c
Sources += psnames/psmodule.c
Sources += raster/raster.c
Sources += sfnt/sfnt.c
Sources += smooth/smooth.c
Sources += truetype/truetype.c
Sources += type1/type1.c
Sources += type42/type42.c
Sources += winfonts/winfnt.c

T = $(Sources:.c=.o)
Objs = $(addprefix $(ObjPath),$(T))
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
