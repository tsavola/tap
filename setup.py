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
			extra_compile_args = [
				"-fvisibility=hidden",
				"-std=c++11",
			],
			sources = glob("tap/core/*.cpp"),
		),
	],
)
