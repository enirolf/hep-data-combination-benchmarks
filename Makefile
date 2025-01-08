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

TARGETS = make_data scenario1 scenario2 scenario3

.PHONY: all clean

all: $(TARGETS)

make_data: make_data.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

scenario1: scenario1.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

scenario2: scenario2.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

scenario3: scenario3.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clear_page_cache: clear_page_cache.c
	gcc -Wall -g -o $@ $<
	sudo chown root $@
	sudo chmod 4755 $@

clean:
	rm -f $(TARGETS)
