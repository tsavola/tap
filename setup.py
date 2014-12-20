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
			extra_compile_args = ["-std=c++11"],
			sources = glob("tap/core/*.cpp"),
		),
	],
)
