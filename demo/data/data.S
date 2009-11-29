.section .rodata

#include "data_macros.S"
// #include "scenes.S"


// mirror
// datafile flare_raw, data/flare.raw
// datafile flare_pal, data/flare.pal
datafile mirror_st, data/mirror.st
datafile mirror_pal, data/mirror.pal
datafile textures1_tex, data/textures1.tex
datafile textures2_tex, data/textures2.tex
datafile textures3_tex, data/textures3.tex
datafile textures4_tex, data/textures4.tex
datafile nmcorner_nmap, data/nm_corner.nmap
datafile nmmiddle_nmap, data/nm_middle.nmap

//mesage
datafile sfc_bp_end_raw, data/sfc_bp_end.raw
datafile sfc_bp_wait4part2_raw, data/sfc_bp_wait4part2.raw

// skate
datafile skate0_pal, data/skate0.pal
datafile skate0_flikk, data/skate0.flikk
datafile skate1_pal, data/skate1.pal
datafile skate1_flikk, data/skate1.flikk
datafile faceplant_pal, data/faceplant.pal
datafile faceplant_flikk, data/faceplant.flikk

// mercury
datafile mercury_st, data/mercury.st
datafile mercury_pal, data/mercury.pal
datafile plane06completemap_tex, data/plane06completemap.tex
datafile env4_tex, data/env4.tex
datafile box10completemap_tex, data/box10completemap.tex
datafile box11completemap_tex, data/box11completemap.tex


// TELOS
datafile telos_glow, data/telos_glow.raw
datafile t01_tex, data/t01.tex
datafile t02_tex, data/t02.tex
datafile t03_tex, data/t03.tex
datafile t04_tex, data/t04.tex
datafile ned_tex, data/ned.tex
datafile opp_tex, data/opp.tex
datafile ref6_tex, data/ref6.tex
datafile telos_pal, data/telos.pal
datafile telos_st, data/telos.st

datafile sfc_bp_bkah_01_pal, data/sfc_bp_bkah_01.pal
datafile sfc_bp_bkah_02_pal, data/sfc_bp_bkah_02.pal
datafile sfc_bp_bkah_03_pal, data/sfc_bp_bkah_03.pal
datafile sfc_bp_bkah_04_pal, data/sfc_bp_bkah_04.pal

datafile sfc_bp_bkah_01_tex, data/sfc_bp_bkah_01.tex
datafile sfc_bp_bkah_02_tex, data/sfc_bp_bkah_02.tex
datafile sfc_bp_bkah_03_tex, data/sfc_bp_bkah_03.tex
datafile sfc_bp_bkah_04_tex, data/sfc_bp_bkah_04.tex

datafile flextextiles_pal, data/flextextiles.pal
datafile flextextiles_tex, data/flextextiles.tex
datafile growy_tex,        data/kvast.tex
datafile part_growy_pal,   data/part_growy.pal


datafile lattice_tex, data/lattice_32f.tex
datafile lattice_pal, data/lattice.pal

datafile poelse_st,           data/poelse.st
datafile poelse_pal,          data/poelse.pal
datafile penv_tex,            data/p_env.tex
datafile roomcompletemap_tex, data/room_completemap.tex
//datafile test_xpal,			  data/test.xpal

// music
// datafile empire_lofi_xm_bin,     data/empire_lofi.xm.bin
// datafile sample_bank_bin, data/sample_bank.bin
// datafile adpcm_data, data/tune.adpcm
datafile tune_adpcm, data/tune.adpcm
datafile tune2_adpcm, data/tune2.adpcm


datafile dist_persp1_raw, data/dist_persp1.raw
overlay speaker01
overlay speaker02
overlay speaker03
overlay speaker04
datafile speaker01_mask, data/speaker01.mask

datafile parallax_glow, data/parallax_glow.raw
datafile scroll2_pal, data/scroll2.pal
datafile scroll2_tile_deltas_bin, data/scroll2_tile_deltas.bin
datafile scroll2_tile_delta_offsets_bin, data/scroll2_tile_delta_offsets.bin
datafile scroll2_0_tileset_raw, data/scroll2_0.tileset.raw
datafile scroll2_1_tileset_raw, data/scroll2_1.tileset.raw
datafile scroll2_2_tileset_raw, data/scroll2_2.tileset.raw
datafile scroll2_3_tileset_raw, data/scroll2_3.tileset.raw

 
// zoom tables
datafile zoom_table, data/zoom_table.bin
datafile zoom_map_raw, data/zoom_map.raw
datafile zoom_map_pal, data/zoom_map.pal

// glow tables
datafile glow_pali, data/glow.pali
datafile glow_spr, data/glow.spr

datafile sfc7_mask, data/sfc7.mask
overlay sfc_logo
overlay sfc7_1

overlay speaker1_01
overlay speaker1_02

datafile lockedloaded_tex, data/lockedloaded.tex
datafile lockedloaded_pal, data/lockedloaded.pal

datafile zoomer_full_tex, data/zoomer_full.tex
datafile zoomer_full_pal, data/zoomer_full.pal

datafile offsets,    data/sphere/offsets.raw
datafile sphere_tex, data/sphere/sphere.tex
datafile sphere_pal, data/sphere/sphere.pal

datafile spheregrid_overlay1_raw  data/spheregrid_overlay1.raw
datafile spheregrid_overlay2_raw  data/spheregrid_overlay2.raw
datafile spheregrid_overlay3_raw  data/spheregrid_overlay3.raw

datafile start01_tex, data/start01.tex
datafile start01_pal, data/start01.pal
datafile start02_tex, data/start02.tex
datafile start02_pal, data/start02.pal
datafile start03_tex, data/start03.tex
datafile start03_pal, data/start03.pal
datafile start04_tex, data/start04.tex
datafile start04_pal, data/start04.pal

datafile cubegrid_st,  data/cubegrid.st
datafile cubegrid_tex, data/cubegrid.tex
datafile cubegrid_pal, data/cubegrid.pal

datafile crack_st,   data/crack.st
datafile crack_pal,  data/crack.pal
datafile crack_xpal, data/crack.xpal
datafile crack_tex,  data/crack.tex
datafile crack2_tex, data/crack2.tex
datafile crackfade2_tex, data/crack_fade2.tex

datafilearray 0, 64, sphere, data/sphere/sphere, raw

.macro make_sphere_array from=0, to=64
.word sphere\from&_raw
.if    \to-\from
make_sphere_array  %(\from+1), \to
.endif
.endm

.global spheremaps
.align
spheremaps:
make_sphere_array
