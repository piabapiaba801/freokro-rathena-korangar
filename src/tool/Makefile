
COMMON_OBJ = minicore.o malloc.o showmsg.o strlib.o utils.o des.o grfio.o nullpo.o
COMMON_DIR_OBJ = $(COMMON_OBJ:%=../common/obj/%)
COMMON_H = $(shell ls ../common/*.hpp)
COMMON_INCLUDE = -I../common/

RA_INCLUDE = -I../

LIBCONFIG_INCLUDE = -I../../3rdparty/libconfig

RAPIDYAML_OBJ = $(shell find ../../3rdparty/rapidyaml/src/ -type f -name "*.cpp" | sed -e "s/\.cpp/\.o/g" )
RAPIDYAML_DIR_OBJ = $(RAPIDYAML_OBJ:%=obj/%)
RAPIDYAML_AR = ../../3rdparty/rapidyaml/obj/ryml.a
RAPIDYAML_H = $(shell find ../../3rdparty/rapidyaml/src/ -type f -name "*.h*")
RAPIDYAML_INCLUDE = -I../../3rdparty/rapidyaml/src -I../../3rdparty/rapidyaml/ext/c4core/src

YAML_CPP_OBJ = $(shell find ../../3rdparty/yaml-cpp/ -type f -name "*.cpp" | sed -e "s/\.cpp/\.o/g" )
YAML_CPP_DIR_OBJ = $(YAML_CPP_OBJ:%=obj/%)
YAML_CPP_AR = ../../3rdparty/yaml-cpp/obj/yaml-cpp.a
YAML_CPP_H = $(shell find ../../3rdparty/yaml-cpp/ -type f -name "*.h")
YAML_CPP_INCLUDE = -I../../3rdparty/yaml-cpp/include

OTHER_H = ../config/renewal.hpp

MAPCACHE_OBJ = obj_all/mapcache.o

CSV2YAML_OBJ = obj_all/csv2yaml.o

YAML2SQL_OBJ = obj_all/yaml2sql.o

YAMLUPGRADE_OBJ = obj_all/yamlupgrade.o



#####################################################################
.PHONY : all mapcache csv2yaml yaml2sql yamlupgrade clean help

all: mapcache csv2yaml yaml2sql yamlupgrade

mapcache: obj_all $(MAPCACHE_OBJ) $(COMMON_DIR_OBJ)
	@echo "	LD	$@"
	@g++ -m64 -L/usr/lib -o ../../mapcache $(MAPCACHE_OBJ) $(COMMON_DIR_OBJ) -lz  -ldl

csv2yaml: obj_all $(CSV2YAML_OBJ) $(COMMON_DIR_OBJ) $(RAPIDYAML_AR) $(YAML_CPP_AR)
	@echo "	LD	$@"
	@g++ -m64 -L/usr/lib -o ../../csv2yaml $(CSV2YAML_OBJ) $(COMMON_DIR_OBJ) ../common/obj/database.o $(RAPIDYAML_AR) $(YAML_CPP_AR) -lz  -ldl

yaml2sql: obj_all $(YAML2SQL_OBJ) $(COMMON_DIR_OBJ) $(RAPIDYAML_AR) $(YAML_CPP_AR)
	@echo "	LD	$@"
	@g++ -m64 -L/usr/lib -o ../../yaml2sql $(YAML2SQL_OBJ) $(COMMON_DIR_OBJ) $(RAPIDYAML_AR) $(YAML_CPP_AR) -lz  -ldl

yamlupgrade: obj_all $(YAMLUPGRADE_OBJ) $(COMMON_DIR_OBJ) $(RAPIDYAML_AR) $(YAML_CPP_AR)
	@echo "	LD	$@"
	@g++ -m64 -L/usr/lib -o ../../yamlupgrade $(YAMLUPGRADE_OBJ) $(COMMON_DIR_OBJ) ../common/obj/database.o $(RAPIDYAML_AR) $(YAML_CPP_AR) -lz  -ldl

clean:
	@echo "	CLEAN	tool"
	@rm -rf obj_all/*.o ../../mapcache ../../csv2yaml ../../yaml2sql ../../yamlupgrade

help:
	@echo "possible targets are 'mapcache' 'csv2yaml' 'yaml2sql' 'yamlupgrade' 'all' 'clean' 'help'"
	@echo "'mapcache'     - mapcache generator"
	@echo "'csv2yaml'     - converts TXT databases to YAML"
	@echo "'yaml2sql'     - converts YAML databases to SQL"
	@echo "'yamlupgrade'  - upgrades YAML databases to latest version"
	@echo "'all'          - builds all above targets"
	@echo "'clean'        - cleans builds and objects"
	@echo "'help'         - outputs this message"

#####################################################################

obj_all:
	-mkdir obj_all

obj_all/%.o: %.cpp $(COMMON_H) $(OTHER_H) $(RAPIDYAML_H) $(YAML_CPP_H)
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(RAPIDYAML_INCLUDE) $(YAML_CPP_INCLUDE)  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

# missing common object files
$(COMMON_DIR_OBJ):
	@$(MAKE) -C ../common server

$(RAPIDYAML_AR):
	@$(MAKE) -C ../../3rdparty/rapidyaml

$(YAML_CPP_AR):
	@$(MAKE) -C ../../3rdparty/yaml-cpp
