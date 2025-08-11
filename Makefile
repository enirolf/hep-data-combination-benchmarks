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

TARGETS = scenario1_without_combinations scenario1_join_first scenario1_union_first \
				 	scenario2_lower_bound scenario2_upper_bound scenario2_join_first scenario2_union_first \
					make_data

.PHONY: all clean moreclean scenario2_join_field

all: $(TARGETS)

make_data: make_data.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

scenario%: scenario%.cxx
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

scenario2_join_field: scenario2_join_field_join_first scenario2_join_field_union_first


clear_page_cache: clear_page_cache.c
	gcc -Wall -g -o $@ $<
	sudo chown root $@
	sudo chmod 4755 $@

clean:
	rm -f $(TARGETS)

auxclean:
	rm -f *.png *.svg *.out *out.old

moreclean: clean auxclean
