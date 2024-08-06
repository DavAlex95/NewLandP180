/**
 * @file ex_token.c
 * @author Guillermo Garcia Maynez R. (ggarcia@necsweb.com)
 * @brief 
 * @version 0.1
 * @date 2021-05-14
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "apiinc.h"
#include "libapiinc.h"
#include "appinc.h"

#include <ex_token.h>
#include <log.h>


int parse_ex(const char *ex, struct ExToken *out) {
  const char *YYMARKER;
  const char *key, *ksn, *kcv, *status, *crc32;

#line 24 "ex_token.c"
  {
    char yych;
    yych = *ex;
    switch (yych) {
      case '!':
        goto yy4;
      default:
        goto yy2;
    }
  yy2:
    ++ex;
  yy3 :
#line 48 "ex_token.re"
  {
    log_warn("Token EX no valido");
    return -1;
  }
#line 37 "ex_token.c"
  yy4:
    yych = *(YYMARKER = ++ex);
    switch (yych) {
      case ' ':
        goto yy5;
      default:
        goto yy3;
    }
  yy5:
    yych = *++ex;
    switch (yych) {
      case 'E':
        goto yy7;
      default:
        goto yy6;
    }
  yy6:
    ex = YYMARKER;
    goto yy3;
  yy7:
    yych = *++ex;
    switch (yych) {
      case 'X':
        goto yy8;
      default:
        goto yy6;
    }
  yy8:
    yych = *++ex;
    switch (yych) {
      case '0':
        goto yy9;
      default:
        goto yy6;
    }
  yy9:
    yych = *++ex;
    switch (yych) {
      case '0':
        goto yy10;
      default:
        goto yy6;
    }
  yy10:
    yych = *++ex;
    switch (yych) {
      case '0':
        goto yy11;
      default:
        goto yy6;
    }
  yy11:
    yych = *++ex;
    switch (yych) {
      case '6':
        goto yy12;
      default:
        goto yy6;
    }
  yy12:
    yych = *++ex;
    switch (yych) {
      case '8':
        goto yy13;
      default:
        goto yy6;
    }
  yy13:
    yych = *++ex;
    switch (yych) {
      case ' ':
        goto yy14;
      default:
        goto yy6;
    }
  yy14:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy15;
      default:
        goto yy6;
    }
  yy15:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy16;
      default:
        goto yy6;
    }
  yy16:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy17;
      default:
        goto yy6;
    }
  yy17:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy18;
      default:
        goto yy6;
    }
  yy18:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy19;
      default:
        goto yy6;
    }
  yy19:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy20;
      default:
        goto yy6;
    }
  yy20:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy21;
      default:
        goto yy6;
    }
  yy21:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy22;
      default:
        goto yy6;
    }
  yy22:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy23;
      default:
        goto yy6;
    }
  yy23:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy24;
      default:
        goto yy6;
    }
  yy24:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy25;
      default:
        goto yy6;
    }
  yy25:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy26;
      default:
        goto yy6;
    }
  yy26:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy27;
      default:
        goto yy6;
    }
  yy27:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy28;
      default:
        goto yy6;
    }
  yy28:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy29;
      default:
        goto yy6;
    }
  yy29:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy30;
      default:
        goto yy6;
    }
  yy30:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy31;
      default:
        goto yy6;
    }
  yy31:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy32;
      default:
        goto yy6;
    }
  yy32:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy33;
      default:
        goto yy6;
    }
  yy33:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy34;
      default:
        goto yy6;
    }
  yy34:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy35;
      default:
        goto yy6;
    }
  yy35:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy36;
      default:
        goto yy6;
    }
  yy36:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy37;
      default:
        goto yy6;
    }
  yy37:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy38;
      default:
        goto yy6;
    }
  yy38:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy39;
      default:
        goto yy6;
    }
  yy39:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy40;
      default:
        goto yy6;
    }
  yy40:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy41;
      default:
        goto yy6;
    }
  yy41:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy42;
      default:
        goto yy6;
    }
  yy42:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy43;
      default:
        goto yy6;
    }
  yy43:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy44;
      default:
        goto yy6;
    }
  yy44:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy45;
      default:
        goto yy6;
    }
  yy45:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy46;
      default:
        goto yy6;
    }
  yy46:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy47;
      default:
        goto yy6;
    }
  yy47:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy48;
      default:
        goto yy6;
    }
  yy48:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy49;
      default:
        goto yy6;
    }
  yy49:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy50;
      default:
        goto yy6;
    }
  yy50:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy51;
      default:
        goto yy6;
    }
  yy51:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy52;
      default:
        goto yy6;
    }
  yy52:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy53;
      default:
        goto yy6;
    }
  yy53:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy54;
      default:
        goto yy6;
    }
  yy54:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy55;
      default:
        goto yy6;
    }
  yy55:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy56;
      default:
        goto yy6;
    }
  yy56:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy57;
      default:
        goto yy6;
    }
  yy57:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy58;
      default:
        goto yy6;
    }
  yy58:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy59;
      default:
        goto yy6;
    }
  yy59:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy60;
      default:
        goto yy6;
    }
  yy60:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy61;
      default:
        goto yy6;
    }
  yy61:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy62;
      default:
        goto yy6;
    }
  yy62:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy63;
      default:
        goto yy6;
    }
  yy63:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy64;
      default:
        goto yy6;
    }
  yy64:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy65;
      default:
        goto yy6;
    }
  yy65:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy66;
      default:
        goto yy6;
    }
  yy66:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy67;
      default:
        goto yy6;
    }
  yy67:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy68;
      default:
        goto yy6;
    }
  yy68:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy69;
      default:
        goto yy6;
    }
  yy69:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy70;
      default:
        goto yy6;
    }
  yy70:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy71;
      default:
        goto yy6;
    }
  yy71:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy72;
      default:
        goto yy6;
    }
  yy72:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
        goto yy73;
      default:
        goto yy6;
    }
  yy73:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
        goto yy74;
      default:
        goto yy6;
    }
  yy74:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy75;
      default:
        goto yy6;
    }
  yy75:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy76;
      default:
        goto yy6;
    }
  yy76:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy77;
      default:
        goto yy6;
    }
  yy77:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy78;
      default:
        goto yy6;
    }
  yy78:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy79;
      default:
        goto yy6;
    }
  yy79:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy80;
      default:
        goto yy6;
    }
  yy80:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy81;
      default:
        goto yy6;
    }
  yy81:
    yych = *++ex;
    switch (yych) {
      case ' ':
      case '0' ... '9':
      case 'A' ... 'F':
        goto yy82;
      default:
        goto yy6;
    }
  yy82:
    yych = *++ex;
    switch (yych) {
      case 0x00:
      case '0':
        goto yy83;
      default:
        goto yy6;
    }
  yy83:
    ++ex;
    key = ex - 69;
    ksn = ex - 37;
    kcv = ex - 17;
    status = ex - 11;
    crc32 = ex - 9;
#line 30 "ex_token.re"
    {
      memcpy(out->encryptedIpek, key, 32);
      out->encryptedIpek[32] = '\0';

      memcpy(out->initialKsn, ksn, 20);
      out->initialKsn[20] = '\0';

      memcpy(out->kcv, kcv, 6);
      out->kcv[6] = '\0';

      memcpy(out->requestStatus, status, 2);
      out->requestStatus[2] = '\0';

      memcpy(out->crc32, crc32, 8);
      out->crc32[8] = '\0';
      return 0;
    }
#line 601 "ex_token.c"
  }
#line 50 "ex_token.re"
}