PY=python
SETUP=setup.py
CMD=install
TEST=tests
TESTS=$(wildcard $(TEST)/test_*.py)

all:
	$(PY) $(SETUP) $(CMD)

test: $(TESTS)
	@for file in $^; do $(PY) $${file}; done;

clean:
	$(RM) -r build dist *.egg-info
