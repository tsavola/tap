import asyncio
import gc
import struct

from . import core

class ProtocolError(Exception):
	pass

class Connection:

	def __init__(self, reader, writer):
		self._peer = core.Peer()
		self._reader = reader
		self._writer = writer

	def __enter__(self):
		return self

	def __exit__(self, *exc):
		self.close()

	def close(self):
		self._writer.close()

	@asyncio.coroutine
	def receive(self):
		obj = yield from receive(self._peer, self._reader)
		return obj

	@asyncio.coroutine
	def send(self, obj):
		yield from send(self._peer, self._writer, obj)

@asyncio.coroutine
def receive(peer, reader):
	while True:
		try:
			data = yield from reader.readexactly(4)
		except asyncio.IncompleteReadError as e:
			if e.partial:
				raise
			else:
				return None

		size, = struct.unpack(b"<I", data)
		if size < 4:
			raise ProtocolError()

		data = yield from reader.readexactly(size - 4)

		obj = core.unmarshal(peer, data)
		if obj is not None:
			return obj

@asyncio.coroutine
def send(peer, writer, obj):
	gc.collect()

	buf = bytearray(4)
	core.marshal(peer, buf, obj)
	buf[:4] = struct.pack(b"<I", len(buf))

	writer.write(buf)
	yield from writer.drain()
