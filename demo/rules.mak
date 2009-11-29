#---------------------------------------------------------------------------------
%.gba: $(BUILDDIR)/%.elf
	$(OBJCOPY) -O binary $< $@
	$(GBAFIX) $@ -t$(notdir $(basename $@))

#---------------------------------------------------------------------------------
%_mb.elf:
	@echo linking multiboot
	$(LD) $(LDFLAGS) -specs=gba_mb.specs $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
%.elf:
	@echo linking cartridge
	$(LD) $(LDFLAGS) -specs=gba.specs $(OFILES) $(LIBPATHS) $(LIBS) -o $@

#---------------------------------------------------------------------------------
%.raw %.pal: %.bmp
	$(GFX2GBA) -o$(dir $@) -p$(notdir $(basename $@)).pal $<

#---------------------------------------------------------------------------------
$(BUILDDIR)/%.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MF $(DEPSDIR)/$(basename $(@F)).d $(CPPFLAGS) $(CXXFLAGS) $(THUMB) -c $< -o $@

$(BUILDDIR)/%.iwram.o: %.cpp
	@echo $(notdir $<)
	$(CXX) -MMD -MF $(DEPSDIR)/$(basename $(@F)).d $(CPPFLAGS) $(CXXFLAGS) $(ARM) -c $< -o $@

#---------------------------------------------------------------------------------
$(BUILDDIR)/%.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MF $(DEPSDIR)/$(basename $(@F)).d $(CPPFLAGS) $(CFLAGS) $(THUMB) -c $< -o $@

$(BUILDDIR)/%.iwram.o: %.c
	@echo $(notdir $<)
	$(CC) -MMD -MF $(DEPSDIR)/$(basename $(@F)).d $(CPPFLAGS) $(CFLAGS) $(ARM) -c $< -o $@

#---------------------------------------------------------------------------------
$(BUILDDIR)/%.o: %.S
	@echo $(notdir $<)
	$(CC) -MMD -MF $(DEPSDIR)/$(basename $(@F)).d -x assembler-with-cpp -trigraphs $(ASFLAGS) -c $< -o $@


#---------------------------------------------------------------------------------
$(BUILDDIR)/%.S: %.cpp
	@echo $(notdir $<)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(THUMB) -fverbose-asm -S $< -o $@

$(BUILDDIR)/%.iwram.S: %.cpp
	@echo $(notdir $<)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(ARM) -fverbose-asm -S $< -o $@

#---------------------------------------------------------------------------------
$(BUILDDIR)/%.S: %.c
	@echo $(notdir $<)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THUMB) -fverbose-asm -S $< -o $@

$(BUILDDIR)/%.iwram.S: %.c
	@echo $(notdir $<)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ARM) -fverbose-asm -S $< -o $@
