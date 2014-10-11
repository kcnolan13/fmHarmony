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

  def buffer_insert(self, buffer, str):
    buffer.insert(buffer.get_iter_at_mark(buffer.get_insert()),str)
    buffer.set_modified(True)

  def update_gui(self):
    while Gtk.events_pending():
        Gtk.main_iteration_do(True)

  def execute_command(self, command):
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)

    self.window_upload.myHandler.active_process = p

    stdout = []
    stations_uploaded = 0
    cells_uploaded = float(0)

    counter = 0

    while True:
      counter = counter + 1

      line = p.stdout.readline()

      #intercept "serial port not opened"
      if (line.find("serial port not opened") > -1):
        self.buffer_insert(self.window_upload.buffer_messages,"\n\nFAILED to open serial port\n\n")
        p.terminate()
        self.upload_failed = True
        return

      #set progress
      if (line.find("Uploading Station") > -1):
        if (stations_uploaded==0):
          self.buffer_insert(self.window_upload.buffer_messages,"\n\n...........\n\n")

        stations_uploaded = stations_uploaded + 1
        self.buffer_insert(self.window_upload.buffer_messages,"Uploading Stations ("+str(stations_uploaded)+" / "+str(self.num_stations)+")\n")

      if (line.find("|  ") > -1):
        self.buffer_insert(self.window_upload.buffer_messages,"Uploading Cells ("+str(int(round((cells_uploaded+0.333333)*10)))+" / 100)\n")
        cells_uploaded = cells_uploaded + 0.333333
        
      self.window_upload.progressbar.set_fraction((float(stations_uploaded)+float(cells_uploaded)) / float(self.num_stations+10))

      stdout.append(line)
      print line,

      #self.buffer_insert(self.window_upload.buffer_messages,line)

      #always scroll to the bottom of the output window
      vadjustment = self.window_upload.scrolledwindow_output.get_vadjustment()
      vadjustment.set_value(vadjustment.get_upper())
      vadjustment.value_changed()

      self.update_gui()

      if line == '' and p.poll() != None:
        break


    print "\n============================\nexecution complete\n============================\n"
    #self.buffer_insert(self.window_upload.buffer_messages,"\n============================\nexecution complete\n============================\n")
    self.window_upload.myHandler.active_process = None
    return ''.join(stdout)

  def on_main_window_destroy(self, object, data=None):
    print "quitting fmHarmony Uploader"
    Gtk.main_quit()

  def on_finish_clicked(self, object, data=None):
    print "bad"

  def on_upload_clicked(self, object, data=None):

    if (self.uploading):
      print "already uploading ... please wait ..."
      return

    self.parse_log()

    if (self.log_valid):
      self.update_gui()
      time.sleep(1)

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
        return

      self.upload_failed = False
      self.buffer_insert(self.window_upload.buffer_messages,"preparing low-level routines...\n")
      self.execute_command("make -C ./lib_serial/ clean")
      self.execute_command("make -C ./lib_serial/")
      self.buffer_insert(self.window_upload.buffer_messages,"\n=============================\nTransmitting Serial Data\n=============================\n")
      self.uploading = True
      self.execute_command("./lib_serial/log_send")
      self.buffer_insert(self.window_upload.buffer_messages,"cleaning up...\n")
      self.execute_command("make -C ./lib_serial/ clean")
      self.buffer_insert(self.window_upload.buffer_messages,"done!\n")
      self.uploading = False
      self.buffer_messages.set_text("")
      self.update_gui()

      if (self.upload_failed):
        self.window_complete.label_upload_complete.set_text("Upload Failed")
      else:
        self.window_complete.label_upload_complete.set_text("Upload Complete")

      self.window_complete.window.show()
      self.window_complete.myHandler.window_upload = self.window_upload
      self.window_complete.myHandler.window = self.window_complete.window


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

    if len(rows) < 3:
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

      #grab the station distribution per Maine grid cell
      elif (counter==2):
        for i in range(0, len(words)-1):
          #make sure only numbers were passed here, and that there are exactly 100
          if (not (words[i].isdigit())):
            print "argument "+str(i+1)+" is not a number"
            self.invalid_line(counter)
            return

        if (len(words) != 100):
          print "\tERROR: supplied ("+str(len(words))+" / 100) grid cells"
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

    self.gladefile = "gui.glade"
    self.builder = Gtk.Builder()
    self.builder.add_from_file(self.gladefile)
    self.builder.connect_signals(self)
    self.window = self.builder.get_object("MainWindow")
    self.window.show()

    self.textview_messages = self.builder.get_object("textview_messages")
    self.textview_messages.set_cursor_visible(False)
    self.textview_messages.set_editable(False)

    self.textview_input = self.builder.get_object("textview_input")
    self.scrolledwindow_input = self.builder.get_object("scrolledwindow_input")

    self.buffer_input = self.textview_input.get_buffer()
    self.buffer_messages = self.textview_messages.get_buffer()

    #define possible text colors
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
    self.window.set_keep_above(True)
    self.label_upload_complete = self.builder.get_object("label_upload_complete")

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