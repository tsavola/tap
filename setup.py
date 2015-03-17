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
				"-Iboost/core/include",
				"-Iboost/endian/include",
				"-Wextra",
				"-Wno-missing-field-initializers",
				"-Wno-unused-parameter",
				"-fvisibility=hidden",
				"-std=c++0x",
			],
			sources = glob("tap/core/*.cpp"),
		),
	],
)
