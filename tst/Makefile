SRC_PATH = ../src
# possible regex '.*\/(?!main.*$)\w+((\.cpp)|(\.h))$'
SRC_DEPS = $(SRC_PATH)/*.h $(SRC_PATH)/hdcamera.cpp

TEST_OUTPUTS := $(patsubst %.cpp,%.out,$(wildcard *.cpp))

TEST_SUCCESS_CODE = 0

# TODO: add dependency to src files
%.out: %.cpp
	echo $(SRC_DEPS)
	$(CXX) -o $@ $^ $(SRC_DEPS) -I$(SRC_PATH) $(CFLAGS) $(ARAVIS_DEPS)

test: $(TEST_OUTPUTS)
	tests_passed=0 ; \
	tests_total=0 ; \
	for file in $^ ; do \
		echo print ; \
		echo $${tests_passed} ; \
		echo $${tests_total} ; \
		tests_total=$$(( $${tests_total}+1 )); \
		./$${file} ; \
		exit_code=$$? ; \
		if ($${exit_code} = $(TEST_SUCCESS_CODE)) ; then \
			echo "Passed $${file}" ; \
			tests_passed=$$(( $${tests_passed}+1 )); \
		else \
			echo "Failed $${file}" ; \
		fi ; \
		echo print2 ; \
		echo $${tests_passed} ; \
		echo $${tests_total} ; \
	done
	echo print3 ; \
	echo $${tests_passed} ; \
	echo $${tests_total} ; \
	echo "Passed $${tests_passed}/$${tests_total} tests." ; \

clean:
	rm -r -f *.out
