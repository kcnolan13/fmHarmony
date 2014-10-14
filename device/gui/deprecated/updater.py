#!/usr/bin/python
import subprocess
import sys

def execute_command(command):
	p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
	stdout = []
	while True:
		line = p.stdout.readline()
		stdout.append(line)
		print line,
		if line == '' and p.poll() != None:
			break
	print "\n============================\nexecution complete\n============================\n"
	return ''.join(stdout)

def main():
	execute_command("make -C ./lib_serial/ clean");
	execute_command("make -C ./lib_serial/");
	execute_command("./lib_serial/log_send");
	execute_command("make -C ./lib_serial/ clean");

if __name__ == '__main__': 
	main()