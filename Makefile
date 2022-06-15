PY=python
SETUP=setup.py
CMD=build_ext
BFLAGS=--inplace
TEST=tests
TESTS=$(wildcard $(TEST)/test_*.py)

all:
	$(PY) $(SETUP) $(CMD) $(BFLAGS)

test: $(TESTS)
	@for file in $^; do $(PY) $${file}; done;

clean:
	$(RM) -r build jthreads*.so
