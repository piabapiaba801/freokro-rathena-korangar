
COMMON_H = $(shell ls ../common/*.hpp)
COMMON_AR = ../common/obj/common.a
COMMON_INCLUDE = -I../common/

RA_INCLUDE = -I../

LIBCONFIG_H = $(shell ls ../../3rdparty/libconfig/*.h)
LIBCONFIG_AR = ../../3rdparty/libconfig/obj/libconfig.a
LIBCONFIG_INCLUDE = -I../../3rdparty/libconfig

RAPIDYAML_OBJ = $(shell find ../../3rdparty/rapidyaml/ -type f -name "*.cpp" | sed -e "s/\.cpp/\.o/g" )
RAPIDYAML_DIR_OBJ = $(RAPIDYAML_OBJ:%=obj/%)
RAPIDYAML_AR = ../../3rdparty/rapidyaml/obj/ryml.a
RAPIDYAML_H = $(shell find ../../3rdparty/rapidyaml/ -type f -name "*.h*")
RAPIDYAML_INCLUDE = -I../../3rdparty/rapidyaml/src -I../../3rdparty/rapidyaml/ext/c4core/src

NLOHMANN_H = $(shell find ../../3rdparty/json/include/nlohmann/ -type f -name "*.hpp")
NLOHMANN_INCLUDE = -I../../3rdparty/json/include

# Create object directories for subdirectories
MAP_OBJ_DIRS = $(sort $(dir $(MAP_DIR_OBJ)))
MAP_GEN_OBJ_DIRS = $(sort $(dir $(MAP_GEN_DIR_OBJ)))
MAP_H = $(shell find ../ -type f -name "*.hpp") \
	$(shell ls ../config/*.hpp) \
	../custom/battle_config_struct.inc

MAP_OBJ = $(shell find . -type f -name "*.cpp" \
			-not -path "./skills/*" \
			| sed -e "s/\.cpp/\.o/g" -e "s/^\.\///")
# Add skill factory/impl files as separate translation units per job category
# Individual skill files are NOT compiled directly - they are #included by their job factory TUs
MAP_OBJ += $(shell find ./skills -maxdepth 1 -type f -name "*.cpp" \
			| sed -e "s/\.cpp/\.o/g" -e "s/^\.\///")
MAP_OBJ += $(shell find ./skills -mindepth 2 -type f -name "skill_factory_*.cpp" \
			| sed -e "s/\.cpp/\.o/g" -e "s/^\.\///")
#MAP_OBJ += $(shell ls *.c | sed -e "s/\.c/\.o/g")
MAP_DIR_OBJ = $(MAP_OBJ:%=obj/%)
MAP_GEN_DIR_OBJ = $(MAP_OBJ:%=obj-gen/%)

HAVE_MYSQL=yes
ifeq ($(HAVE_MYSQL),yes)
	SERVER_DEPENDS=map-server
else
	SERVER_DEPENDS=needs_mysql
endif
ALL_DEPENDS=server

HAVE_PCRE=
ifeq ($(HAVE_PCRE),yes)
	PCRE_CFLAGS=-DPCRE_SUPPORT 
else
	PCRE_CFLAGS=
endif

TOOLS_DEPENDS=map-server-generator
TOOLS_FLAGS="-DMAP_GENERATOR"



#####################################################################
.PHONY : all server clean help

all: $(ALL_DEPENDS)

server: $(SERVER_DEPENDS)

tools: $(TOOLS_DEPENDS)

clean:
	@echo "	CLEAN	map"
	@rm -rf *.o obj ../../map-server

help:
	@echo "possible targets are 'server' 'all' 'clean' 'help'"
	@echo "'server' - map server"
	@echo "'all'    - builds all above targets"
	@echo "'clean'  - cleans builds and objects"
	@echo "'help'   - outputs this message"

#####################################################################

needs_mysql:
	@echo "MySQL not found or disabled by the configure script"
	@exit 1

# object directories

obj: $(MAP_OBJ_DIRS)
	@echo "	MKDIR	obj"
	@-mkdir -p obj

obj-gen: $(MAP_GEN_OBJ_DIRS)
	@echo "	MDIR	obj-gen"
	@-mkdir -p obj-gen

# Create subdirectories
$(MAP_OBJ_DIRS):
	@echo "	MKDIR	$@"
	@-mkdir -p $@

$(MAP_GEN_OBJ_DIRS):
	@echo "	MKDIR	$@"
	@-mkdir -p $@

# executables

map-server: obj $(MAP_DIR_OBJ) $(COMMON_AR) $(LIBCONFIG_AR) $(RAPIDYAML_AR)
	@echo "	LD	map-server"
	@g++ -m64 -L/usr/lib -o ../../map-server $(MAP_DIR_OBJ) $(COMMON_AR) $(LIBCONFIG_AR) $(RAPIDYAML_AR) -lz  -ldl  -L/usr/lib/x86_64-linux-gnu/ -lmariadb


map-server-generator: obj-gen $(MAP_GEN_DIR_OBJ) $(COMMON_AR) $(LIBCONFIG_AR) $(RAPIDYAML_AR)
	@echo "	LD	map-server-generator"
	@g++ -m64 -L/usr/lib -o ../../map-server-generator $(MAP_GEN_DIR_OBJ) $(COMMON_AR) $(LIBCONFIG_AR) $(RAPIDYAML_AR) -lz  -ldl  -L/usr/lib/x86_64-linux-gnu/ -lmariadb


# map object files
#cause this one failling otherwise
obj/npc.o: npc.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H)
	@echo "	CXX	$< (custom rule)"
	@g++ -std=c++17  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

# these have extra dependencies
obj/atcommand.o: atcommand.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H) ../custom/atcommand_def.inc ../custom/atcommand.inc
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

obj/script.o: script.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H) ../custom/script_def.inc ../custom/script.inc
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

obj/battle.o: battle.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H) ../custom/battle_config_init.inc
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

obj/%.o: %.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H)
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

obj-gen/npc.o: npc.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H)
	@echo "	CXX	$< (custom rule)"
	@g++ -std=c++17  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK $(TOOLS_FLAGS) $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<
	
obj-gen/%.o: %.cpp $(MAP_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H) $(NLOHMANN_H)
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(TOOLS_FLAGS) $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(PCRE_CFLAGS) $(RAPIDYAML_INCLUDE) $(NLOHMANN_INCLUDE) -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

# missing object files
$(COMMON_AR):
	@$(MAKE) -C ../common server

$(LIBCONFIG_AR):
	@$(MAKE) -C ../../3rdparty/libconfig

$(RAPIDYAML_AR):
	@$(MAKE) -C ../../3rdparty/rapidyaml
