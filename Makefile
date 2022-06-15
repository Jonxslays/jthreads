PY=python
SETUP=setup.py
CMD=build_ext
BFLAGS=--inplace
TEST=tests
TESTS=$(wildcard $(TEST)/test_*.py)

all: clean
	$(PY) $(SETUP) $(CMD) $(BFLAGS)

install: clean
	$(PY) $(SETUP) install -f

test: $(TESTS)
	@for file in $^; do $(PY) $${file}; done;

clean:
	pip uninstall jthreads && \
		$(RM) -r build dist jthreads*.so jthreads*.egg-info
