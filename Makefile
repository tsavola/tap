-include config.mk

TARGET	:= env/lib/python3.4/site-packages/tap/__init__.py
SOURCES	:= $(wildcard tap/*.py tap/core/*.cpp tap/core/*.hpp)

build:: $(TARGET)

test:: build
	cp test.py env/lib/
	. env/bin/activate && cd env && python -B lib/test.py 1 data
	. env/bin/activate && cd env && python -B lib/test.py 2 data

$(TARGET): env/bin/activate $(SOURCES)
	. env/bin/activate && python setup.py clean install
	touch $@

env/bin/activate: cpython/python
	cpython/python -m venv --clear --without-pip env

cpython/python: cpython/Makefile
	$(MAKE) -C cpython -j$(J)

cpython/Makefile: cpython/configure
	- $(MAKE) -C cpython distclean
	cd cpython && ./configure

cpython/configure:
	git submodule update --init --recursive

clean::
	- python3 setup.py clean
	rm -rf env build
	- $(MAKE) -C cpython clean
	- rm -f cpython/python

purge:: clean
	- $(MAKE) -C cpython distclean
