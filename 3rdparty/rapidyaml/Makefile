
RAPIDYAML_OBJ = $(shell find * -type f -name "*.cpp" | sed -e "s/\.cpp/\.o/g")
RAPIDYAML_DIR_OBJ = $(RAPIDYAML_OBJ:%=obj/%)
RAPIDYAML_H =  $(shell find * -type f -name "*.h*")
RAPIDYAML_AR = obj/ryml.a

CXXFLAG =-std=c++11
INC=-Isrc -Iext/c4core/src



#####################################################################
.PHONY : all clean help rapidyaml

all: rapidyaml

clean:
	@echo "	CLEAN	rapidyaml"
	@rm -rf obj *.o

help:
	@echo "possible targets are 'all' 'clean' 'help'"
	@echo "'rapidyaml' - build $(RAPIDYAML_AR)
	@echo "'all'       - builds $(RAPIDYAML_DIR_OBJ)"
	@echo "'clean'     - deletes $(RAPIDYAML_DIR_OBJ)"
	@echo "'help'      - outputs this message"

#####################################################################

obj:
	@echo "	MKDIR	obj/src/c4/yml"
	@echo "	MKDIR	obj/ext/c4core/src/c4"
	@mkdir -p obj/src/c4/yml
	@mkdir -p obj/ext/c4core/src/c4

obj/%.o: %.cpp $(RAPIDYAML_H)
	@echo "	CXX	$<"
	@g++ $(CXXFLAG)   -O2 -DNDEBUG -m64 -pipe -ffast-math -Wall -Wempty-body -Wno-switch -Wno-missing-field-initializers -Wno-maybe-uninitialized -Wno-clobbered -Wshadow  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -g $(INC) -c $(OUTPUT_OPTION) $<

rapidyaml: obj $(RAPIDYAML_DIR_OBJ) $(RAPIDYAML_AR)

$(RAPIDYAML_AR): $(RAPIDYAML_DIR_OBJ)
	@echo "	AR	$@"
	@/usr/bin/ar rcs obj/ryml.a $(RAPIDYAML_DIR_OBJ)
