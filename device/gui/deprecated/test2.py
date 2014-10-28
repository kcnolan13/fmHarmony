#!/usr/bin/python
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

class MyWindow(Gtk.Window):

	def __init__(self):
		Gtk.Window.__init__(self, title="fmUpdate")
		#Gtk.Window.set_default_size(self,500,500)
		Gtk.Window.set_border_width(self, 30)
		self.vBox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=20)
		self.add(self.vBox)

		self.hBox = Gtk.Box(spacing = 20)
		self.hBox2 = Gtk.Box(spacing = 20)

		self.lbl_title = Gtk.Label("FM Harmony")
		self.lbl_subtitle = Gtk.Label('"No More Blind Dates With Local Radio"')

		self.vBox.pack_start(self.lbl_title, True, True, 0)
		self.vBox.pack_start(self.lbl_subtitle, True, True, 0)


		self.vBox.pack_start(self.hBox2, True, True, 0)

		#messages and input log file labels
		self.lbl_input = Gtk.Label("Paste fmHarmony Log File Here:")
		self.lbl_input.set_size_request(300,40)
		self.lbl_msg = Gtk.Label("Messages:")
		self.lbl_msg.set_size_request(300,40)

		self.hBox2.pack_start(self.lbl_input, True, True, 0)
		self.hBox2.pack_start(self.lbl_msg, True, True, 0)

		self.vBox.pack_start(self.hBox, True, True, 0)

		#the input box
		self.win_input = Gtk.ScrolledWindow()
		self.win_input.set_size_request(300,500)
		self.win_input.set_hexpand(True)
		self.win_input.set_vexpand(True)

		self.txt_view = Gtk.TextView()
		self.txt_buffer = self.txt_view.get_buffer()
		self.txt_buffer.set_text("")
		self.win_input.add(self.txt_view)

		self.hBox.pack_start(self.win_input, True, True, 0)

		#the message box
		self.win_msg = Gtk.ScrolledWindow()
		self.win_msg.set_size_request(300,40)
		self.win_msg.set_hexpand(True)
		self.win_msg.set_vexpand(True)
		self.msg_view = Gtk.TextView()
		self.msg_buffer = self.msg_view.get_buffer()
		self.win_msg.add(self.msg_view)

		self.hBox.pack_start(self.win_msg, True, True, 0)


		self.btn_upload = Gtk.Button(label="Upload")
		self.btn_upload.connect("clicked", self.on_upload_clicked)
		self.add(self.btn_upload)

	def on_upload_clicked(self, widget):
		execute_command("make -C ./lib_serial/ clean");
		execute_command("make -C ./lib_serial/");
		execute_command("./lib_serial/log_send");
		execute_command("make -C ./lib_serial/ clean");

def main():
	win = MyWindow()
	win.connect("delete-event", Gtk.main_quit)
	win.show_all()
	Gtk.main()

if __name__ == '__main__': 
	main()