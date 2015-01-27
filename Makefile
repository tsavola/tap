-include config.mk

PYTHON	:= python3.4

build::
	$(PYTHON) setup.py build_ext -i -f

test:: build
	PYTHONASYNCIODEBUG=1 $(PYTHON) test.py

clean::
	rm -f tap/core.cpython-34m.so
	rm -rf build
