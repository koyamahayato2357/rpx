#pragma once

#define ESCSI "\033["

#define ESCLR ESCSI "0m"

#define ESBLD ESCSI "1m"
#define ESTHN ESCSI "2m"
#define ESITA ESCSI "3m"
#define ESULN ESCSI "4m"
#define ESBLN ESCSI "5m"
#define ESFBLN ESCSI "6m"
#define ESREV ESCSI "7m"
#define ESHID ESCSI "8m"
#define ESUDO ESCSI "9m"

#define ESCBLK ESCSI "30m"
#define ESCRED ESCSI "31m"
#define ESCGRN ESCSI "32m"
#define ESCYEL ESCSI "33m"
#define ESCBLU ESCSI "34m"
#define ESCMGN ESCSI "35m"
#define ESCCYN ESCSI "36m"
#define ESCWHT ESCSI "37m"

#define ESCBBLK ESCSI "40m"
#define ESCBRED ESCSI "41m"
#define ESCBGRN ESCSI "42m"
#define ESCBYEL ESCSI "43m"
#define ESCBBLU ESCSI "44m"
#define ESCBMGN ESCSI "45m"
#define ESCBCYN ESCSI "46m"
#define ESCBWHT ESCSI "47m"

#define ESCCODE(code) ESCSI "38;5;" #code "m"
#define ESCCODE_RGB(r, g, b) ESCSI "38;2;" #r ";" #g ";" #b "m"

#define ESCUU(n) ESCSI #n "A"
#define ESCUD(n) ESCSI #n "B"
#define ESCUF(n) ESCSI #n "C"
#define ESCUB(n) ESCSI #n "D"
#define ESCNL(n) ESCSI #n "E"
#define ESCPL(n) ESCSI #n "F"
#define ESCHA(n) ESCSI #n "G"
#define ESCUP(n, m) ESCSI #n ";" #m "H"
#define ESED(n) ESCSI #n "J"
#define ESEL(n) ESCSI #n "K"
#define ESSU(n) ESCSI #n "S"
#define ESSD(n) ESCSI #n "T"
#define ESHVP(n, m) ESCSI #n ";" #m "f"
#define ESSCP ESCSI "s"
#define ESRCP ESCSI "u"

void putsequence(char *);
