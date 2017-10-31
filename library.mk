
vpath %.c src
vpath %.cpp src
vpath %.cc src

FOMAT = " \033[42;30m"
FOMAT_NONE = "\033[0m"
all: $(LIBRARY)

$(LIBRARY): $(OBJS)
	@mkdir -p $(dir $(LIBRARY))
	@$(RM) $(LIBRARY)
	@$(CROSS_TOOLCHAIN)$(AR) r $(LIBRARY) $(OBJS)

# Step to compile source files
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo $(FOMAT) Compiling $^ $(FOMAT_NONE)
	@$(CROSS_TOOLCHAIN)$(IFX_CXX) -c $(CXXFLAGS) $(CFLAGS) $(DEFS) $^ -o $@
	
# Step to compile source files
$(OBJDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	@echo $(FOMAT) Compiling $^ $(FOMAT_NONE)
	@$(CROSS_TOOLCHAIN)$(IFX_CC) -c $(CXXFLAGS) $(CFLAGS) $(DEFS) $^ -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo $(FOMAT) Compiling $^ $(FOMAT_NONE)
	@$(CROSS_TOOLCHAIN)$(IFX_CC) -c $(CFLAGS) $(DEFS) $^ -o $@

clean:
	@$(RM) -r $(OBJDIR)
	@$(RM) $(LIBRARY)

install:
