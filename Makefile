# Makefile-x86
#
# Makefile pour compilation du projet pour cible x86


#
# Variables
#
ifndef ARCH
	ARCH = x86
endif
CROSS_COMPILE=/opt/siema/siema-powerpc-buildroot-linux-gnu-x86_64-pc-linux-gnu-1.0.1/usr/bin/powerpc-linux-
CC = $(CROSS_COMPILE)gcc

# -ansi: tells the compiler to implement the ANSI language option. This turns off certain "features" of GCC which are incompatible with the ANSI standard.
# -pedantic: used in conjunction with -ansi, this tells the compiler to be adhere strictly to the ANSI standard, rejecting any code which is not compliant.
# -std=c99: Determine the language standard to ISO C99
CFLAGS = -W -Wall -ansi -pedantic -std=c99 -D_GNU_SOURCE
ifeq ($(DEBUG),1)
	CFLAGS+=-g
endif

ifeq ($(NOUDPSERV),1)
	CFLAGS+=-D_UDP_SERVER_NOT_ACTIVE
endif

ifeq ($(NOUDPCLI),1)
	CFLAGS+=-D_UDP_CLIENT_NOT_ACTIVE
endif

ifeq ($(NOTCPSERV),1)
	CFLAGS+=-D_TCP_SERVER_NOT_ACTIVE
endif

ifeq ($(NOTCPCLI),1)
	CFLAGS+=-D_TCP_CLIENT_NOT_ACTIVE
endif


LDFLAGS = -lpthread -D_REENTRANT -lrt
INC = -Isrc/ -Isrc/common/ -I../../VLB-TFNG-SW/software/
LIB = -L.

RM = @rm -rf
MAKEDIRS = @mkdir -p

SRC_DIR = src/
SRC_DIR_COMMON = $(SRC_DIR)common/


OBJ_DIR = build/$(ARCH)/obj/
BIN_DIR = build/$(ARCH)/bin/

#
# Règles de compilation
#
default: all

#all: msg test_udp_serv2 test_udp_cli test_types
all: msg comm_ucc_pdt

msg:
ifeq ($(DEBUG),1)
	@echo "Generation en mode debug pour $(ARCH)"
else
	@echo "Generation en mode release pour $(ARCH)"
endif

comm_ucc_pdt: $(OBJ_DIR)main.o \
			$(OBJ_DIR)tlog.o \
			$(OBJ_DIR)common.o \
			$(OBJ_DIR)code5t.o \
			$(OBJ_DIR)code5t_proc.o \
			$(OBJ_DIR)config.o \
			$(OBJ_DIR)server_udp.o \
			$(OBJ_DIR)client_udp.o \
			$(OBJ_DIR)server_tcp.o \
			$(OBJ_DIR)client_tcp.o \
			$(OBJ_DIR)heartbeat.o \
			$(OBJ_DIR)pto_mgt.o \
			$(OBJ_DIR)process_audio.o \
			$(OBJ_DIR)gtw_core.o \
			$(OBJ_DIR)socks_mgt.o \
			$(OBJ_DIR)socks_tcp_mgt.o \
			$(OBJ_DIR)Siema_NC_CORE_Asterisk_Utils.o
	${MAKEDIRS} $(BIN_DIR)
	$(CC) $(LIB) -o $(BIN_DIR)$@  $^ $(LDFLAGS)

$(OBJ_DIR)%.o: $(SRC_DIR_COMMON)%.c
	${MAKEDIRS} $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	${MAKEDIRS} $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<



#
# Règles de nettoyage
#
.PHONY: clean, mrproper

clean:
	${RM} *.bak
	${RM} $(OBJ_DIR)*.o
 
mrproper: clean
	${RM} $(BIN_DIR)/*


