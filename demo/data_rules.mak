# gridexpander-texture
data/tex1.act: datasrc/tex1.png
data/tex1.shiftpal.act: data/tex1.act
	$(SFCCON) shiftpal $< 000000 0 0 > $@
data/tex1.tex: tex1.shiftpal.act datasrc/tex1.png
data/tex1.pal: tex1.shiftpal.act
	$(SFCCON) gbapal $< > $@

data/tex2.act: datasrc/tex2.png
data/tex2.shiftpal.act: data/tex2.act
	$(SFCCON) shiftpal $< 000000 0 0 > $@
data/tex2.tex: tex2.shiftpal.act datasrc/tex2.png
data/tex2.pal: tex2.shiftpal.act
	$(SFCCON) gbapal $< > $@


data/intro1.act: datasrc/intro1.png
data/intro1.tex: data/intro1.act datasrc/intro1.png
data/intro2.act: datasrc/intro2.png
data/intro2.tex: data/intro2.act datasrc/intro2.png
data/intro3.act: datasrc/intro3.png
data/intro3.tex: data/intro3.act datasrc/intro3.png
data/intro4.act: datasrc/intro4.png
data/intro4.tex: data/intro4.act datasrc/intro4.png
data/intro5.act: datasrc/intro5.png
data/intro5.tex: data/intro5.act datasrc/intro5.png
data/intro6.act: datasrc/intro6.png
data/intro6.tex: data/intro6.act datasrc/intro6.png
data/intro7.act: datasrc/intro7.png
data/intro7.tex: data/intro7.act datasrc/intro7.png
data/intro8.act: datasrc/intro8.png
data/intro8.tex: data/intro8.act datasrc/intro8.png
data/intro9.act: datasrc/intro9.png
data/intro9.tex: data/intro9.act datasrc/intro9.png
data/credits.act: datasrc/credits.png
data/credits.tex: data/credits.act datasrc/credits.png


#implicit rules

data/%.tex:
	$(SFCCON) quantisize $(filter %.act, $^) $(filter-out %.act, $^) 0 > $@

%.act:
	$(SFCCON) actpal @$< > $@

%.act: %.xpal
	$(SFCCON) actpal @$< > $@

%.pal: %.act
	$(SFCCON) gbapal $< > $@

data/%.adpcm : datasrc/%.raw
	tools/adpcm $< $@

%.raw : %.wav
	tools/sox $< -s -w -c1 $@
