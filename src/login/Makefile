
COMMON_H = $(shell ls ../common/*.hpp)
COMMON_AR = ../common/obj/common.a
COMMON_INCLUDE = -I../common/

RA_INCLUDE = -I../

LIBCONFIG_H = $(shell ls ../../3rdparty/libconfig/*.h)
LIBCONFIG_AR = ../../3rdparty/libconfig/obj/libconfig.a
LIBCONFIG_INCLUDE = -I../../3rdparty/libconfig

LOGIN_OBJ = $(shell ls *.cpp | sed -e "s/\.cpp/\.o/g")
LOGIN_DIR_OBJ = $(LOGIN_OBJ:%=obj/%)
LOGIN_H = $(shell ls *.hpp)

RAPIDYAML_OBJ = $(shell find ../../3rdparty/rapidyaml/ -type f -name "*.cpp" | sed -e "s/\.cpp/\.o/g" )
RAPIDYAML_DIR_OBJ = $(RAPIDYAML_OBJ:%=obj/%)
RAPIDYAML_AR = ../../3rdparty/rapidyaml/obj/ryml.a
RAPIDYAML_H = $(shell find ../../3rdparty/rapidyaml/ -type f -name "*.h*")
RAPIDYAML_INCLUDE = -I../../3rdparty/rapidyaml/src -I../../3rdparty/rapidyaml/ext/c4core/src

HAVE_MYSQL=yes
ifeq ($(HAVE_MYSQL),yes)
	SERVER_DEPENDS=login-server
else
	SERVER_DEPENDS=needs_mysql
endif
ALL_DEPENDS=server



#####################################################################
.PHONY :all server clean help

all: $(ALL_DEPENDS)

server: $(SERVER_DEPENDS)

clean:
	@echo "	CLEAN	login"
	@rm -rf *.o obj ../../login-server

help:
	@echo "possible targets are 'server' 'all' 'clean' 'help'"
	@echo "'server'    - login server"
	@echo "'all'       - builds all above targets"
	@echo "'clean'     - cleans builds and objects"
	@echo "'help'      - outputs this message"

#####################################################################

needs_mysql:
	@echo "MySQL not found or disabled by the configure script"
	@exit 1

# object directories

obj:
	@echo "	MKDIR	obj"
	@-mkdir obj

#executables

login-server: obj $(LOGIN_DIR_OBJ) $(COMMON_AR) $(RAPIDYAML_AR)
	@echo "	LD	login-server"
	@g++ -m64 -L/usr/lib -o ../../login-server $(LOGIN_DIR_OBJ) $(COMMON_AR) $(LIBCONFIG_AR) $(RAPIDYAML_AR) -lz  -ldl -L/usr/lib/x86_64-linux-gnu/ -lmariadb

# login object files

obj/%.o: %.cpp $(LOGIN_H) $(COMMON_H) $(LIBCONFIG_H) $(RAPIDYAML_H)
	@echo "	CXX	$<"
	@g++ -O2 -DNDEBUG -m64 -std=c++17 $(COMMON_INCLUDE) $(RA_INCLUDE) $(LIBCONFIG_INCLUDE) $(RAPIDYAML_INCLUDE) -DWITH_SQL -I/usr/include/mariadb -I/usr/include/mariadb/mysql  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -c $(OUTPUT_OPTION) $<

# missing object files
$(COMMON_AR):
	@$(MAKE) -C ../common server

$(LIBCONFIG_AR):
	@$(MAKE) -C ../../3rdparty/libconfig

$(RAPIDYAML_AR):
	@$(MAKE) -C ../../3rdparty/rapidyaml
