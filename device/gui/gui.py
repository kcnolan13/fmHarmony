#!/usr/bin/env python
import subprocess
import sys
from gi.repository import Gtk
import sched, time

def main():
  window_main = MainWindow()
  Gtk.main()

#---- GUI WINDOW CLASSES ----#

#the primary application window
class MainWindow:

  #insert text into a GTK buffer
  def buffer_insert(self, buffer, str):
    buffer.insert(buffer.get_iter_at_mark(buffer.get_insert()),str)
    buffer.set_modified(True)

  #call for explicit GTK update
  def update_gui(self):
    while Gtk.events_pending():
        Gtk.main_iteration_do(True)

  #execute system commands with a sub-process
  def execute_command(self, command):
    #create a new subprocess
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)

    #keep a handle to this subprocess in the upload window's handler object
    self.window_upload.myHandler.active_process = p

    stdout = []
    stations_uploaded = 0
    counter = 0

    #track progress and subprocess stdout
    while True:
      counter = counter + 1
      line = p.stdout.readline()

      #catch "serial port not opened" stop condition
      if (line.find("serial port not opened") > -1) or (line.find("error") > -1) or (line.find("cannot open") > -1):
        self.buffer_insert(self.window_upload.buffer_messages,"\n\nSERIAL PORT UNRESPONSIVE\n\n")
        p.terminate()
        self.upload_failed = True
        return

      #track progress and output text progress to console window
      if (line.find("Uploading Station") > -1):
        stations_uploaded = stations_uploaded + 1
        self.buffer_insert(self.window_upload.buffer_messages,"Uploading Stations ("+str(stations_uploaded)+" / "+str(self.num_stations)+")\n")
        
      #update progress bar
      self.window_upload.progressbar.set_fraction(float(stations_uploaded) / float(self.num_stations))
      stdout.append(line)
      print line,

      #always scroll to the bottom of the output window
      vadjustment = self.window_upload.scrolledwindow_output.get_vadjustment()
      vadjustment.set_value(vadjustment.get_upper())
      vadjustment.value_changed()

      #call for a GUI update
      self.update_gui()

      if line == '' and p.poll() != None:
        break

    #stop condition met --> subprocess has finished
    print "\n============================\nexecution complete\n============================\n"
    self.window_upload.myHandler.active_process = None
    return ''.join(stdout)

  def on_main_window_destroy(self, object, data=None):
    print "quitting fmHarmony Uploader"
    Gtk.main_quit()

  def on_finish_clicked(self, object, data=None):
    #handler will actually cover this
    print "bad"

  def on_upload_clicked(self, object, data=None):
    if (self.uploading):
      print "already uploading ... please wait ..."
      return

    #disable multiple upload button clicks
    self.button_upload.set_sensitive(False)

    #validate the input text in the log window
    self.parse_log()

    #perform the update if the log file is valid
    if (self.log_valid):

      #force a graphical component update
      self.update_gui()
      time.sleep(1)

      #fire up the upload window
      self.window_upload.buffer_messages.set_text("")
      self.window_upload.window.show()

      #write out the gui log file to the real log file
      self.buffer_insert(self.window_upload.buffer_messages,"saving log file...\n")
      try:
        fo = open("./lib_serial/log.txt", "wb")
        fo.write(self.buffer_input.get_text(self.buffer_input.get_start_iter(),self.buffer_input.get_end_iter(),True))
        fo.close()
      except ValueError:
        print "FAILED to save log file for serial transmission"
        self.upload_failed = True
        self.button_upload.set_sensitive(True)
        return

      #proceed with the update
      self.upload_failed = False
      self.buffer_insert(self.window_upload.buffer_messages,"preparing low-level routines...\n")

      #use sub-processes to prepare the underlying c libraries
      self.execute_command("make -C ./lib_serial/ clean")
      self.execute_command("make -C ./lib_serial/")

      #send the log file
      self.buffer_insert(self.window_upload.buffer_messages,"\n=============================\nTransmitting Serial Data\n=============================\n")
      self.uploading = True
      self.execute_command("./lib_serial/log_send")

      #clean up the underlying c libraries
      self.buffer_insert(self.window_upload.buffer_messages,"cleaning up...\n")
      self.execute_command("make -C ./lib_serial/ clean")
      self.buffer_insert(self.window_upload.buffer_messages,"done!\n")
      self.uploading = False
      self.buffer_messages.set_text("")
      self.update_gui()

      #display upload completion status
      if (self.upload_failed):
        self.window_complete.label_upload_complete.set_text("Upload Failed")
      else:
        self.window_complete.label_upload_complete.set_text("Upload Complete")

      #show the upload finished window
      self.window_complete.window.show()
      self.window_complete.myHandler.window_upload = self.window_upload
      self.window_complete.myHandler.window = self.window_complete.window

    #re-sensitize the upload button
    self.button_upload.set_sensitive(True)


  #mark a log file line as invalid ---> prevents serial upload
  def invalid_line(self, line_num):
    self.log_valid = False
    print "FAILED\n\n"
    self.buffer_messages.set_text("Invalid fmHarmony Log File\n========================\n\nCheck Line #"+str(line_num))

    #scroll the input text window to the offending line
    vadjustment = self.scrolledwindow_input.get_vadjustment()
    vadjustment.set_value(vadjustment.get_lower()+12*(line_num-1))
    vadjustment.value_changed()

    #turn some text red
    self.buffer_messages.apply_tag(self.tag_red_messages, self.buffer_messages.get_start_iter(), self.buffer_messages.get_iter_at_line(2))
    self.buffer_input.apply_tag(self.tag_red_input, self.buffer_input.get_iter_at_line(line_num-1), self.buffer_input.get_iter_at_line(line_num))

  #determine log file validity before trying to send via serial
  def parse_log(self):

    #clear all text formatting on the input and messages buffer
    self.buffer_messages.remove_all_tags(self.buffer_messages.get_start_iter(), self.buffer_messages.get_end_iter())
    self.buffer_input.remove_all_tags(self.buffer_input.get_start_iter(), self.buffer_input.get_end_iter())

    print "========================\nchecking log file integrity\n========================"
    self.buffer_messages.set_text("checking log file integrity...\n\n")

    #retrieve contents of log file
    self.buff = self.buffer_input.get_text(self.buffer_input.get_start_iter(),self.buffer_input.get_end_iter(),True)

    counter = 0
    station_counter = 0
    #convert to line-by-line list
    rows = self.buff.splitlines(True)
    self.num_stations = 0

    if len(rows) < 2:
      self.invalid_line(counter+1)
      return

    #---- PARSE THE LOG FILE ----#
    for unstripped_line in rows:
      #remove leading and trailing whitespace
      line = unstripped_line.strip()
      #obtain each word
      words = line.split(' ')

      #keep track of the line number being parsed
      counter = counter + 1
      print "checking line #"+str(counter)+" ----> "+str(line)

      #grab the number of stations
      if (counter==1):
        try:
          self.num_stations = int(line)
        except ValueError:
          self.invalid_line(counter)
          return

        print "\t"+str(self.num_stations)+" stations found, from "+str(len(words))+" supplied argument"
        
        if (self.num_stations < 1) or (self.num_stations > 100) or (len(words) > 1):
          self.invalid_line(counter)
          return

      #grab each station line
      else:
        station_counter = station_counter + 1
        #make sure only 6 parameters passed
        if (len(words) != 6):
          self.invalid_line(counter)
          return
        #make sure the callsign has 8 chars or less
        if (len(words[0]) > 8):
          self.invalid_line(counter)
          return
        #make sure the last five are valid floats
        for i in range(1,6):
          try:
            float(words[i])
          except ValueError:
            self.invalid_line(counter)
            return

    #cross-reference supposed number of stations and num stations supplied
    if (station_counter != self.num_stations):
      print "incorrect number of stations supplied: "+str(station_counter)+" / "+str(self.num_stations)
      self.invalid_line(counter)
      return

    #all lines have been parsed --> it is a valid log file
    self.log_valid = True
    self.buffer_messages.set_text("fmHarmony Log File Is Valid\n========================\n\n")
    self.buffer_messages.apply_tag(self.tag_green_messages, self.buffer_messages.get_start_iter(), self.buffer_messages.get_end_iter())



  def __init__(self):
    #both other windows are children of the main window
    self.window_upload = UploadWindow()
    self.window_complete = CompleteWindow()
    self.uploading = False
    self.log_valid = False
    self.num_stations = False
    self.upload_failed = False

    #glade application layout file + builder entities
    self.gladefile = "gui.glade"
    self.builder = Gtk.Builder()
    self.builder.add_from_file(self.gladefile)
    self.builder.connect_signals(self)
    self.window = self.builder.get_object("MainWindow")

    #show the main window
    self.window.show()

    #acquire handles to the major application components
    self.textview_messages = self.builder.get_object("textview_messages")
    self.textview_messages.set_cursor_visible(False)
    self.textview_messages.set_editable(False)
    self.textview_input = self.builder.get_object("textview_input")
    self.scrolledwindow_input = self.builder.get_object("scrolledwindow_input")
    self.buffer_input = self.textview_input.get_buffer()
    self.buffer_messages = self.textview_messages.get_buffer()
    self.button_upload = self.builder.get_object("button1")

    #define text color pallete
    self.tag_green_messages = self.buffer_messages.create_tag("green", foreground="green")
    self.tag_red_messages = self.buffer_messages.create_tag("red", foreground="red")
    self.tag_green_input = self.buffer_input.create_tag("green", foreground="green")
    self.tag_red_input = self.buffer_input.create_tag("red", background="red")


#the upload progress meter
class UploadWindow:

  def __init__(self):
    self.gladefile = "gui.glade"
    self.builder = Gtk.Builder()
    self.builder.add_from_file(self.gladefile)

    self.myHandler = Handler()
    self.builder.connect_signals(self.myHandler)

    self.window = self.builder.get_object("UploadWindow")
    self.window.connect('delete-event',lambda widget, event: True)
    self.window.set_deletable(False)

    self.textview_messages = self.builder.get_object("textview_stdout")
    self.textview_messages.set_cursor_visible(False)
    self.textview_messages.set_editable(False)
    self.buffer_messages = self.textview_messages.get_buffer()

    self.progressbar = self.builder.get_object("progressbar")
    self.scrolledwindow_output = self.builder.get_object("scrolledwindow_output")

#upload complete message
class CompleteWindow:

  def __init__(self):
    self.gladefile = "gui.glade"
    self.builder = Gtk.Builder()
    self.builder.add_from_file(self.gladefile)

    self.myHandler = Handler()
    self.builder.connect_signals(self.myHandler)

    self.window = self.builder.get_object("CompleteWindow")
    self.window.connect('delete-event',lambda widget, event: True)
    self.window.set_deletable(False)
    self.window.set_keep_above(True)
    self.label_upload_complete = self.builder.get_object("label_upload_complete")

#handler class to receive and react to various application signals
class Handler:

  def __init__(self):
    self.active_process = None

  def on_main_window_destroy(self, object, data=None):
    print "quitting fmHarmony Uploader"

    if (self.active_process):
      print "\nterminating 1 active process\n"
      self.active_process.terminate()

    Gtk.main_quit()
  def on_upload_clicked(self, object, data=None):
    print "bad"
    
  def on_finish_clicked(self, object, data=None):
    self.window_upload.window.hide()
    #print "made it"
    self.window.hide()
    self.window_upload.progressbar.set_fraction(0)


if __name__ == "__main__":
  main()