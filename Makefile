CXX = g++
CXXFLAGS_ROOT = $(shell root-config --cflags)
ifeq ($(CXXFLAGS_ROOT),)
  $(error Cannot find root-config. Please source thisroot.sh)
endif

# In case we want/need to debug and/or profile stuff
ifeq ($(DEBUG),yes)
	CXXFLAGS = -std=c++17 -Wall -pthread -g -O0 $(CXXFLAGS_ROOT)
else
	CXXFLAGS = -std=c++17 -Wall -pthread -O3 $(CXXFLAGS_ROOT)
endif

LDFLAGS = -lROOTNTuple $(shell root-config --libs)

TARGETS=

.PHONY: all clean

all: $(TARGETS)

clean:
	rm -f $(TARGETS)
