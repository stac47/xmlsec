@echo off
REM 
REM This is my personal configuration file. 
REM I am lazy to type all this crap again and again
REM You are welcome to customize this file for your
REM needs but do not check it into the CVS, please.
REM
REM Aleksey Sanin <aleksey@aleksey.com>
REM 

REM SET ICONV=d:\sdk\bin\iconv
REM SET LIBXML2=d:\sdk\bin\libxml2
REM SET LIBXSLT=d:\sdk\bin\libxslt
REM SET OPENSSL=d:\sdk\bin\openssl
REM SET XMLSEC_PREFIX=d:\sdk\bin\xmlsec
REM SET XMLSEC_INCLUDE=%ICONV%\include;%LIBXML2%\include;%LIBXSLT%\include;%OPENSSL%\include
REM SET XMLSEC_LIB=%ICONV%\lib;%LIBXML2%\lib;%LIBXSLT%\lib;%OPENSSL%\lib

SET XMLSEC_PREFIX=d:\sdk
SET XMLSEC_INCLUDE=%XMLSEC_PREFIX%\include
SET XMLSEC_LIB=%XMLSEC_PREFIX%\lib

SET INCLUDE=%XMLSEC_INCLUDE%;%INCLUDE%
SET LIB=%XMLSEC_LIB%;%LIB%

nmake -f Makefile.w32 %1 %2 %3