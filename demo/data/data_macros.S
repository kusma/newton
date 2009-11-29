.altmacro

.macro datafile symbol, filename
.global \symbol
.align
\symbol:
.incbin "\filename"
.endm

.macro overlay filename
datafile \filename&_tileset_raw data/\filename&.tileset.raw
datafile \filename&_tileset_pal data/\filename&.tileset.pal
datafile \filename&_oam data/\filename&.oam
.endm

.macro datafilearray from=0, to=5, symbol_basename, file_basename, ext=raw
datafile \symbol_basename\from&_\ext, \file_basename\from&.\ext
.if    \to-\from
datafilearray    %(\from+1), \to, \symbol_basename, \file_basename, \ext
.endif
.endm
