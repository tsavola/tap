import sys

sys.path.remove("")

import tap
import tap.core

def main():
	if sys.argv[1] == "1":
		f = test1
	elif sys.argv[1] == "2":
		f = test2

	f()

def test1():
	def func(obj, xprint, out):
		xprint(obj, file=out)
		xprint(sys.prefix, file=out)
		xprint(sys.maxsize, file=out)
		builtinfunc = obj[-1]
		xprint(builtinfunc())

	print(tap.core.Opaque(), file=sys.stderr)

	peer = tap.Peer()
	buf = bytearray()
	t = (5435452652,)
	obj = (func, (1324, 5435432, t, None), (54325235, 9, t, 765376542, None, 9), sys.exc_info)
	tap.marshal(peer, buf, obj)

	print("{} bytes".format(len(buf)), file=sys.stderr)

	with open(sys.argv[2], "wb") as f:
		f.write(buf)

def test2():
	with open(sys.argv[2], "rb") as f:
		data = f.read()

	peer = tap.Peer()
	obj = tap.unmarshal(peer, data)

	func = obj[0]
	func(obj, print, sys.stderr)

if __name__ == "__main__":
	main()
