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

	t = (5435452652,)
	l = [54325235, 9, t, 765376542, None, 9]
	obj = (func, (1324, 5435432, t, None), l, sys.getrefcount)

	buf = bytearray()
	tap.marshal(peer, buf, obj)
	print("size:", len(buf))

	with open("data-0", "wb") as f:
		f.write(buf)

	l[0] = 777
	gc.collect()

	buf = bytearray()
	tap.marshal(peer, buf, obj)
	print("size:", len(buf))

	with open("data-1", "wb") as f:
		f.write(buf)

def test2():
	peer = tap.Peer()

	for n in range(2):
		with open("data-{}".format(n), "rb") as f:
			data = f.read()

		obj = tap.unmarshal(peer, data)

		func = obj[0]
		func(obj)

		gc.collect()

if __name__ == "__main__":
	main()
