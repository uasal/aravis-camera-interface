SOURCE_DIR = src
TEST_DIR = tst
CLEAN_DIRS = $(SOURCE_DIR) $(TEST_DIR)

export CXX = g++
export ARAVIS_DEPS = `pkg-config --libs --cflags aravis-0.6` \
	-lm -pthread -lgio-2.0 -lgobject-2.0 \
	-lxml2 -lgthread-2.0 -lglib-2.0 -lz -lpng 

source: $(SOURCE_DIR)
	$(MAKE) -C $(SOURCE_DIR) \

test: $(TEST_DIR)
	$(MAKE) -C $(TEST_DIR) test \

clean: $(CLEAN_DIRS)
	for dir in $(CLEAN_DIRS); do \
		$(MAKE) -C $$dir -f Makefile $@; \
	done
	
