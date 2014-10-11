#!/usr/bin/env python
import subprocess
import sys

from gi.repository import Gtk



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

class MainWindow(Gtk.Window):

	def __init__(self):
		#Set the Glade file
		


def main():
	win = MainWindow()
	Gtk.main()

if __name__ == '__main__': 
	main()