import asyncio
import logging
import multiprocessing
import os
import sys

logging.basicConfig(level=logging.DEBUG)

import tap

log = logging.getLogger("test")

def test_server():
	loop = asyncio.get_event_loop()

	@asyncio.coroutine
	def connected(reader, writer):
		log.info("server: connection from client")

		try:
			with tap.Connection(reader, writer) as conn:
				count = 0

				while True:
					log.info("server: receiving object from client")

					obj = yield from conn.receive()
					if obj is None:
						break

					log.info("server: received object from client")

					f = obj[3][0]
					print("dir(f) =", dir(f))
					print("f.f_back =", f.f_back)
					print("f.f_builtins =", f.f_builtins)
					print("f.f_code =", f.f_code)
					print("f.f_globals.keys() =", f.f_globals.keys())
					for key, value in f.f_globals.items():
						print("f_globals: key =", key)
						print("f_globals: value =", value)
					print("f.f_lasti =", f.f_lasti)
					print("f.f_lineno =", f.f_lineno)
					print("f.f_locals =", f.f_locals)
					print("f.f_trace =", f.f_trace)

					func = obj[0]
					func(obj)

					count += 1
					if count == 2:
						obj2 = obj[1:3]
						yield from conn.send(obj2)

				log.info("server: EOF from client")

			log.info("server: connection closed")
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
	import time
	time.sleep(1)
	reader, writer = loop.run_until_complete(asyncio.open_unix_connection("socket"))

	log.info("client: connected to server")

	with tap.Connection(reader, writer) as conn:
		log.info("client: sending object to server")

		m = {"foo": "bar"}
		t = (5435452652, m)
		l = [54325235, 9, t, 765376542, None, 9]
		obj = (func, sys.stdout, l, (1324, 5435432, t, None), sys.getrefcount)

		loop.run_until_complete(conn.send(obj))

		log.info("client: sent object to server")
		log.info("client: sending object to server")

		l[0] = 777

		m["foo"] = "bar2"
		m[1] = 2
		m[False] = True

		loop.run_until_complete(conn.send(obj))

		log.info("client: sent object to server")
		log.info("client: receiving object from server")

		obj2 = loop.run_until_complete(conn.receive())

		log.info("client: received object from server")

		stdout2, l2 = obj2
		l2[-1] = 11

		print("loopback:", l2, file=stdout2)
		assert l is l2

	log.info("client: connection closed")

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
			log.error(p)

	if any(p.exitcode for p in procs):
		sys.exit(1)

if __name__ == "__main__":
	main()
