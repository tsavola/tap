for i in range(2, 12, 3):
	print(i)

def func0():
	return 5

def func1(value):
	return value + 2

x = 10
r = func0() + func1(x)
print("r =", r)

def passthrough(x):
	return x

while x:
	print("foo")
	x = 0

s = "hello" + passthrough(" ") + "world"
print(repr(s), len(s))
print("id(None) =", id(None))
print(None, 1234.5 + passthrough(6.78), None)

import tap

def other0():
	print("spawnee with", 0, "arguments")

def other1(arg1):
	print("spawnee with", 1, "argument:", arg1)

def other2(arg1, arg2):
	print("spawnee with", 2, "arguments:", arg1, arg2)

def other3(arg1, arg2, arg3):
	print("spawnee with", 3, "arguments:", arg1, arg2, arg3)

tap.spawn(other0)
tap.spawn(other1, True)
tap.spawn(other2, False, True)
tap.spawn(other3, None, False, True)
