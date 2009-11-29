# graphics

data/%.tex:
	$(SFCCON) quantisize $(filter %.act, $^) $(filter-out %.act, $^) 0 > $@

%.nmap: %.png
	$(SFCCON) pic2nmap $^ > $@

%.st: %.act
	$(SFCCON) shadetable $^ 0.5 0 > $@

%.xpal:
	$(SFCCON) reduce 256 0 $^ > $@

%.act: %.xpal
	$(SFCCON) actpal @$< > $@

%.pal: %.act
	$(SFCCON) gbapal $< > $@

data/test.xpal : datasrc/p_env.png
#data/p_env.tex : data/test.xpal datasrc/p_env.png

data/crack.xpal : datasrc/skene_filer/crack/*.png
	$(SFCCON) reduce 128 0 $^ > $@

data/crack.act : data/crack.xpal
	$(SFCCON) shiftpal @$< FFFFFF 1 0 > $@

data/crack.st:
data/crack.tex : data/crack.act datasrc/skene_filer/crack/crack.png
data/crack2.tex : data/crack.act datasrc/skene_filer/crack/crack2.png
data/crack_fade2.tex : data/crack.act datasrc/skene_filer/crack/crack_fade2.png

data/flower.xpal: datasrc/green.png datasrc/p_env.png
	$(SFCCON) reduce 1 0 datasrc/green.png > $@
	$(SFCCON) reduce 255 0 datasrc/p_env.png >> $@

data/parallax_glow.raw: datasrc/parallax_glow.avi
	$(SFCCON) glow_precalc $< 0.5 2.0 0.8 10  >$@

#mercury
data/mercury.st:
data/mercury.xpal:  datasrc/skene_filer/slide/Plane06CompleteMap.png datasrc/skene_filer/slide/env4.JPG  datasrc/skene_filer/slide/Box10CompleteMap.png datasrc/skene_filer/slide/Box11CompleteMap.png
data/plane06completemap.tex: data/mercury.act datasrc/skene_filer/slide/Plane06CompleteMap.png
data/env4.tex: data/mercury.act datasrc/skene_filer/slide/env4.JPG
data/box10completemap.tex: data/mercury.act datasrc/skene_filer/slide/Box10CompleteMap.png
data/box11completemap.tex: data/mercury.act datasrc/skene_filer/slide/Box11CompleteMap.png

#break kids are here:
data/sfc_bp_bkah_01.xpal: sfc_bp_bkah_01.png
data/sfc_bp_bkah_01.tex:  sfc_bp_bkah_01.act sfc_bp_bkah_01.png

data/zoomer_full.xpal: zoomer_full.png
data/zoomer_full.tex:  zoomer_full.act zoomer_full.png

data/sfc_bp_bkah_02.xpal: sfc_bp_bkah_02.png
data/sfc_bp_bkah_02.tex:  sfc_bp_bkah_02.act sfc_bp_bkah_02.png

data/sfc_bp_bkah_03.xpal: sfc_bp_bkah_03.png
data/sfc_bp_bkah_03.tex:  sfc_bp_bkah_03.act sfc_bp_bkah_03.png


data/sfc_bp_bkah_04.xpal: sfc_bp_bkah_04.png
data/sfc_bp_bkah_04.tex:  sfc_bp_bkah_04.act sfc_bp_bkah_04.png

data/lockedloaded.xpal: lockedloaded.png
data/lockedloaded.tex:  lockedloaded.act lockedloaded.png

data/start01.act: start01.png
	$(SFCCON) actpal $< > $@
data/start01.tex: start01.png
	$(SFCCON) pic2tex $< > $@

#polse
data/poelse.xpal: datasrc/skene_filer/poelse/room_completemap.png datasrc/skene_filer/poelse/p_env.png
data/room_completemap.tex: data/poelse.act datasrc/skene_filer/poelse/room_completemap.png
data/p_env.tex: data/poelse.act datasrc/skene_filer/poelse/p_env.png

data/start02.xpal: start02.png
data/start02.tex:  data/start02.act start02.png
data/start03.xpal: start03.png
data/start03.tex:  data/start03.act start03.png
data/start04.xpal: start04.png
data/start04.tex:  data/start04.act start04.png




# telos
data/telos.st:
data/telos.xpal: datasrc/skene_filer/telos/ref6.png datasrc/skene_filer/telos/ned.png datasrc/skene_filer/telos/opp.png  datasrc/skene_filer/telos/01.png datasrc/skene_filer/telos/04.png  datasrc/skene_filer/telos/03.png datasrc/skene_filer/telos/02.png
data/ref6.tex: data/telos.act datasrc/skene_filer/telos/ref6.png 
data/ned.tex: data/telos.act datasrc/skene_filer/telos/ned.png 
data/opp.tex: data/telos.act datasrc/skene_filer/telos/opp.png 
data/t01.tex: data/telos.act datasrc/skene_filer/telos/01.png 
data/t02.tex: data/telos.act datasrc/skene_filer/telos/02.png 
data/t03.tex: data/telos.act datasrc/skene_filer/telos/03.png 
data/t04.tex: data/telos.act datasrc/skene_filer/telos/04.png 
data/telos_glow.raw: datasrc/telos_glow.avi
	$(SFCCON) glow_precalc $< 0.5 2.0 0.8 30  >$@


# mirror
data/mirror.st:
data/textures1.tex: data/mirror.act datasrc/skene_filer/mirror/textures1.png 
data/textures2.tex: data/mirror.act datasrc/skene_filer/mirror/textures2.png 
data/textures3.tex: data/mirror.act datasrc/skene_filer/mirror/textures3.png 
data/textures4.tex: data/mirror.act datasrc/skene_filer/mirror/textures4.png 
data/nm_corner.nmap: datasrc/skene_filer/mirror/nm_corner.png
	$(SFCCON) pic2nmap $^ > $@
data/nm_middle.nmap: datasrc/skene_filer/mirror/nm_middle.png
	$(SFCCON) pic2nmap $^ > $@



# skate stuff
data/skate0.flikk: datasrc/skate_optimize/skate0.gif
	$(SFCCON) flikk $^ 0.5 > $@
data/skate1.flikk: datasrc/skate_optimize/skate1.gif
	$(SFCCON) flikk $^ 0.5 > $@
data/faceplant.flikk: datasrc/skate_optimize/faceplant01.gif
	$(SFCCON) flikk $^ 1.0 > $@
	
	

# audio

data/%.adpcm : datasrc/%.raw
	tools/adpcm $< $@

%.raw : %.wav
	tools/sox $< -s -w -c1 $@
