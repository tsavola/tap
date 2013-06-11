from distutils.core import setup, Extension
from glob import glob

setup(
	name = "tap",
	packages = [
		"tap",
	],
	ext_modules = [
		Extension(
			"tap/core",
			sources = glob("tap/core/*.cpp"),
		),
	],
)
