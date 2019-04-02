MKDIR = mkdir -p
BINDIR = ./bin
SRCDIR = .
OBJDIR = obj
INCLUDEDIRS =
LIBDIRS =
LIBS =
CPPFLAGS = -std=c++17 -pthread
CFLAGS = -std=c11 -D_GNU_SOURCE

CPPSRCS = $(subst $(SRCDIR)/, , $(wildcard $(SRCDIR)/*.cpp))
CPPBINS = $(patsubst %.cpp, $(BINDIR)/%, $(CPPSRCS))
CSRCS = $(subst $(SRCDIR)/, , $(wildcard $(SRCDIR)/*.c))
CBINS = $(patsubst %.c, $(BINDIR)/%, $(CSRCS))

all: directories $(CPPBINS) $(CBINS)

debug: CFLAGS += -ggdb -o0
debug: CPPFLAGS += -ggdb -o0
debug: directories $(CPPBINS) $(CBINS)

directories:
	$(MKDIR) $(BINDIR)

$(BINDIR)/%: $(SRCDIR)/%.cpp
	g++ $(CPPFLAGS) $(INCLUDEDIRS) -o $@ $< $(LIBDIRS) $(LIBS)

$(BINDIR)/%: $(SRCDIR)/%.c
	gcc $(CFLAGS) $(INCLUDEDIRS) -o $@ $< $(LIBDIRS) $(LIBS)

clean:
	rm -f $(BINDIR)/*

