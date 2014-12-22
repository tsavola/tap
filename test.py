import asyncio
import multiprocessing
import os
import sys

import tap

def test_server():
	loop = asyncio.get_event_loop()

	@asyncio.coroutine
	def connected(reader, writer):
		print("server: connection from client")

		try:
			with tap.Connection(reader, writer) as conn:
				while True:
					obj = yield from conn.receive()
					if obj is None:
						break

					print("server: received object from client")

					func = obj[0]
					func(obj)

				print("server: EOF from client")

			print("server: connection closed")
		finally:
			loop.stop()

	if os.path.exists("socket"):
		os.remove("socket")

	loop.run_until_complete(asyncio.start_unix_server(connected, "socket"))
	loop.run_forever()

def test_client():
	def func(obj):
		print("obj:", obj)
		print("sys.prefix:", sys.prefix)
		print("sys.maxsize:", sys.maxsize)
		getrefcount = obj[-1]
		print("refcount:", getrefcount(None))

	loop = asyncio.get_event_loop()
	reader, writer = loop.run_until_complete(asyncio.open_unix_connection("socket"))

	print("client: connected to server")

	with tap.Connection(reader, writer) as conn:
		print("client: sending object to server")

		t = (5435452652,)
		l = [54325235, 9, t, 765376542, None, 9]
		obj = (func, (1324, 5435432, t, None), l, sys.getrefcount)

		loop.run_until_complete(conn.send(obj))

		print("client: sending object to server")

		l[0] = 777

		loop.run_until_complete(conn.send(obj))

	print("client: connection closed")

def main():
	procs = []

	for target, name in [(test_server, "server"), (test_client, "client")]:
		p = multiprocessing.Process(target=target, name=name)
		p.start()

		procs.append(p)

	for p in procs:
		p.join()

	for p in procs:
		if p.exitcode:
			print(p, file=sys.stderr)

	if any(p.exitcode for p in procs):
		sys.exit(1)

if __name__ == "__main__":
	main()
