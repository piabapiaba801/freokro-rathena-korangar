
HTTPLIB_OBJ =  $(shell find * -type f -name "*.cc" | sed -e "s/\.cc/\.o/g")
HTTPLIB_DIR_OBJ = $(HTTPLIB_OBJ:%=obj/%)
HTTPLIB_H =  $(shell find * -type f -name "*.h")
HTTPLIB_AR = obj/httplib.a

CXXFLAG =-std=c++11

HTTPLIB_DEPENDS=httplib



#####################################################################
.PHONY : all clean help httplib

all: $(HTTPLIB_DEPENDS)

clean:
	@echo "	CLEAN	httplib"
	@rm -rf obj *.o

help:
	@echo "possible targets are 'all' 'clean' 'help'"
	@echo "'httplib' - build $(HTTPLIB_AR)
	@echo "'all'       - builds $(HTTPLIB_DIR_OBJ)"
	@echo "'clean'     - deletes $(HTTPLIB_DIR_OBJ)"
	@echo "'help'      - outputs this message"

#####################################################################

obj:
	@echo "	MKDIR	obj"
	@mkdir -p obj
	
obj/%.o: %.cc $(HTTPLIB_H)
	@echo "	CXX	$<"
	@g++ $(CXXFLAG)   -O2 -DNDEBUG -m64 -pipe -ffast-math -Wall -Wempty-body -Wno-switch -Wno-missing-field-initializers -Wno-maybe-uninitialized -Wno-clobbered -Wshadow  -I../common -DHAS_TLS -fno-strict-aliasing -DHAVE_GETADDRINFO -DHAVE_SETRLIMIT -DHAVE_STRNLEN -DPACKETVER=20250716  -I/usr/include -DHAVE_MONOTONIC_CLOCK -DCPPHTTPLIB_SEND_FLAGS=MSG_NOSIGNAL -g -c $(OUTPUT_OPTION) $<

httplib: obj $(HTTPLIB_DIR_OBJ) $(HTTPLIB_AR)

$(HTTPLIB_AR): $(HTTPLIB_DIR_OBJ)
	@echo "	AR	$@"
	@/usr/bin/ar rcs obj/httplib.a $(HTTPLIB_DIR_OBJ)
