@goto Begin 
This is an example dos batch file of how to set environment variable SDK_INSTALL_DIR and MinGW_DIR
to build applications with Makefile
This batch file alters the PATH variable, restores it when done.
:Begin



@rem set Newland SDK installed directory 
@set SDK_INSTALL_DIR=C:\NPT_SDK

@rem Set MinGW_DIR
@set MinGW_DIR=%SDK_INSTALL_DIR%\Linux\Core\third-party\mingw

@rem Set CROSS_COMPILER_DIR
@set CROSS_COMPILER_DIR=%SDK_INSTALL_DIR%\Linux\Compiler\4.9.4

@rem Set PACKAGE_GENERATOR_DIR
@set PACKAGE_GENERATOR_DIR=%SDK_INSTALL_DIR%\Common\tools\Package_Generator

@rem Set DEBUGGER_DIR
@set DEBUGGER_DIR=%SDK_INSTALL_DIR%\Linux\Core\terminal\debugger


@rem Set Complite Path
@set Path=%MinGW_DIR%\bin;%MinGW_DIR%\msys\1.0\bin;%CROSS_COMPILER_DIR%\bin;%PACKAGE_GENERATOR_DIR%\pkgNLD;%PACKAGE_GENERATOR_DIR%\bin;%DEBUGGER_DIR%\bin;
@set OpenSSL_CONF=%SDK_INSTALL_DIR%\Common\tools\Package_Generator\pkgNLD\openssl.cnf

@rem use Makefileto buid application
make -f Makefile


pause
