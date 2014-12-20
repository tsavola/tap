import gc
import sys

import tap
import tap.core

def main():
	if sys.argv[1] == "1":
		f = test1
	elif sys.argv[1] == "2":
		f = test2

	f()
	gc.collect()

def test1():
	def func(obj):
		print("obj:", obj)
		print("sys.prefix:", sys.prefix)
		print("sys.maxsize:", sys.maxsize)
		getrefcount = obj[-1]
		print("refcount:", getrefcount(None))

	print("opaque:", tap.core.Opaque())

	peer = tap.Peer()
	print("peer:", peer)

	buf = bytearray()
	t = (5435452652,)
	obj = (func, (1324, 5435432, t, None), (54325235, 9, t, 765376542, None, 9), sys.getrefcount)
	tap.marshal(peer, buf, obj)

	print("size:", len(buf))

	with open(sys.argv[2], "wb") as f:
		f.write(buf)

def test2():
	with open(sys.argv[2], "rb") as f:
		data = f.read()

	peer = tap.Peer()
	obj = tap.unmarshal(peer, data)

	func = obj[0]
	func(obj)

if __name__ == "__main__":
	main()
