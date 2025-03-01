#!/usr/bin/python
import os
import subprocess

files = os.listdir(".")
files.remove("runTests.py")

def runTest(filename : str) -> bool:
	binFn = filename.removesuffix(".txt")
	if(os.system("../main " + filename + " -o " + binFn) != 0):
		return False
	
	try:
		binOutput = subprocess.check_output("./" + binFn, shell=True, text=True)
	except:
		return False
	try:
		interpreterOutput = subprocess.check_output("../main -i -lc " + filename, shell=True, text=True)
	except:
		return False
	binOutput = binOutput.strip()
	interpreterOutput = interpreterOutput.strip()
	print("comparing binOut to interpreterOut: ", binOutput, interpreterOutput)
	return binOutput == interpreterOutput

def clean() -> None:
	files = os.listdir(".")
	files.remove("runTests.py")
	files = [f for f in files if not f.endswith(".txt")]
	print("removing: ", files)
	for file in files:
		os.remove(file)

passes = []
fails = []
for file in files:
	if runTest(file):
		passes.append(file)
	else:
		fails.append(file)

clean()
print("tests done with ", len(passes), " passes and ", len(fails), " fails")
print("passes: ", passes)
print("fails: ", fails)
