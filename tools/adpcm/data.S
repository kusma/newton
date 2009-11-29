.macro datafile symbol, filename
.global \symbol
.align
\symbol:
.incbin "\filename"
.endm

# datafile test_adpcm, tongpoo.gba
datafile test_adpcm, test.adpcm
