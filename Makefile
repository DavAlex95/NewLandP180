CROSS_COMPILE=arm-linux-gnueabi-

CC=$(CROSS_COMPILE)gcc
RM=rm
CP=cp
MKUPT=upt

# Application directory definition
BINDIR = bin
SRCDIR = src
INCDIR = inc
OBJDIR = obj_err
LIBAPPDIR = lib/gcc4.9.4
LIBINCDIR = libapi

# Module directory definition
PRINTINCDIR = module/libprint/inc
PRINTSRCDIR = module/libprint/src
SECURITYIINCDIR = module/libsecurity/inc
SECURITYISRCDIR = module/libsecurity/src
UIINCDIR = module/libui/inc
UISRCDIR = module/libui/src
TOOLINCDIR = module/libtool/inc
TOOLSRCDIR = module/libtool/src
COMMINCDIR = module/libcomm/inc
COMMSRCDIR = module/libcomm/src

# SDK directory definition
SDKDIR = C:\\NPT_SDK
SDKAPIDIR = $(SDKDIR)\\Linux\\Core\\api\\legacy_ndk
SDKAPILIB = $(SDKAPIDIR)\\lib\\gcc_4.9.4

OPENSSLDIR = $(SDKDIR)\\Linux\\Core\\third-party\\openssl\\1.0.2m
OPENSSLINCDIR =  $(OPENSSLDIR)\\openssl

# NAPI
SDKNAPIDIR = $(SDKDIR)\\Linux\\Core\\api\\napi
SDKNAPIINC = $(SDKNAPIDIR)\\inc
SDKNAPILIB = $(SDKNAPIDIR)\\lib\\gcc_4.9.4

# EMV L2
LIBL2EMVDIR = $(SDKDIR)\\Linux\\Core\\EMV\\L2_SDK\\lib\\gcc_4.9.4

# Libc
SDKSYSLIB = $(SDKDIR)\\Linux\\Compiler\\4.9.4\\arm-linux-gnueabi\\libc\\usr\\lib

# -D USE_NTOMS  NTOMS means TOMS 2.0
# CFLAGS += -Wall -Werror -DNDEBUG $(CROSS_CFLAGS) -O -g $(INCPATH)  -D USE_NTOMS
CFLAGS += -Werror -DNDEBUG $(CROSS_CFLAGS) -O -g $(INCPATH)  -D USE_NTOMS
CFLAGS += -funwind-tables
CFLAGS += -DAPP_VERSION=\"$(PARAMINI_VER)\"
# File search path
VPATH = src $(PRINTSRCDIR) $(SECURITYISRCDIR) $(UISRCDIR) $(TOOLSRCDIR) $(COMMSRCDIR) $(OBJDIR)

# Header file search path
INCLPATH = -I$(INCDIR) -I$(PRINTINCDIR) -I$(SECURITYIINCDIR) -I$(UIINCDIR) -I$(TOOLINCDIR) -I$(COMMINCDIR)  -I$(LIBINCDIR) -I$(SDKNAPIINC) -I$(OPENSSLDIR) 

# Program link parameters
LDFLAGS += -L$(SDKAPILIB) -lndk

LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_app -lnapi_c_sysinfo
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_datetime -lnapi_c_kb -lnapi_c_power -lnapi_c_beeper -lnapi_c_timer
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_eth -lnapi_c_modem -lnapi_c_wifi -lnapi_c_wlm -lnapi_c_bt
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_net -lnapi_c_externalmemory -lnapi_c_socket -lnapi_c_fs
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_ctlscard -lnapi_c_magcard -lnapi_c_smartcard
LDFLAGS += -L$(SDKNAPILIB) -lnapi_c_crypto -lnapi_c_alg -lnapi_c_printer -lnapi_c_led -lnapi_c_display -lnapi_c_serialport

LDFLAGS += -L$(SDKSYSLIB) -lssl -lcrypto -lcurl -lpthread -ldl -lz -lm

# EMV L2 Lib
LDFLAGS += -L$(LIBL2EMVDIR) -lemvl2 -lentrypoint -lbase
# App Lib
LDFLAGS += -L$(LIBAPPDIR) -lemvl3
# App Lib for TOMS 2.0
LDFLAGS += -L$(LIBAPPDIR) -lconfig
LDFLAGS += -L$(LIBAPPDIR) -lntoms
LDFLAGS += -L$(LIBAPPDIR) -lntoms
#LDFLAGS += -Llib -lmbedcrypto

# model such as SP630/sp630Pro/sp830/sp930/U1000/ME60, these are compiled by gcc4.9.4
MODEL = GCC494

# Application Name
PARAMINI_NAME = BANKDEMO
# Application Version
PARAMINI_VER = 1.0.0

# Generated program name
NAME = main
APPNAME = $(MODEL)$(PARAMINI_NAME)_$(PARAMINI_VER).NLD

# Module used in the program
SRCS = $(wildcard $(SRCDIR)/*.c $(PRINTSRCDIR)/*.c $(SECURITYISRCDIR)/*.c $(UISRCDIR)/*.c $(TOOLSRCDIR)/*.c $(COMMSRCDIR)/*.c )
SRSS = $(notdir $(SRCS))
OBJS = $(subst .c,.o,$(SRSS))
OBJSD = $(addprefix $(OBJDIR)/,$(SRSS))
OBJSY = $(subst .c,.o,$(OBJSD))

# Include dependency file
all: NLD

$(NAME):config $(OBJS)
	$(CC) $(CFLAGS) -o $(BINDIR)/$(NAME) $(OBJSY) $(LDFLAGS)
	iniupdate Name=$(PARAMINI_NAME) $(BINDIR)/param.ini
	iniupdate Version=$(PARAMINI_VER) $(BINDIR)/param.ini
%.o: %.c
	$(CC) -c $(CFLAGS) $(INCLPATH) $< -o $(OBJDIR)/$@
# Automatic generation of dependency files
config: $(subst .o,.deps, $(OBJS))

%.deps: %.c
	$(CC) -MM $(INCLPATH) $< >$(OBJDIR)/$@

.PHONY:clean
clean:
	-$(RM) $(BINDIR)/$(NAME)
	-$(RM) $(OBJDIR)/*.o
	-$(RM) $(OBJDIR)/*.deps

NLD:$(NAME)
	$(MKUPT) -h $(BINDIR)/filepath.txt -p $(BINDIR)/param.ini -o $(BINDIR)/$(APPNAME)
