-include config.make

PYTHON		:= python3.4
PYFLAKES	:= pyflakes3

build::
	$(PYTHON) setup.py build_ext -i -f

check:: build
	$(PYFLAKES) tap *.py

test:: build
	PYTHONASYNCIODEBUG=1 $(PYTHON) test.py

clean::
	rm -f tap/core.cpython-34m.so
	rm -rf build
