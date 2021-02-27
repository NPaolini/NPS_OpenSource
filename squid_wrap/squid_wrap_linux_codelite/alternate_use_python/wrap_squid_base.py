#!/usr/bin/python3
import os
import sys
import string
import base64
import time

from socket import *
#----------------------------------------
class connection :
  def __init__(self) :
    self.serverHost = ''
    self.serverPort = 0
    self.s = None
    self.part = ''
  #-----------
  def __enter__(self):
    return self

  def __del__(self):
    self.try_close()

  def __exit__(self, type, value, tb):
    self.try_close()
  #-----------
  def init(self, part) :
    self.part = part
    found = 0
    try:
      path = os.path.dirname(os.path.realpath(__file__))
      source = open(path + '/wrap_squid.txt', 'r')
      for row in source :
        row = row.strip()
        if(len(row)) :
          t = row.split('=')
          if('ip' == t[0]) :
            self.serverHost = t[1]
            found += 1
          elif('port' == t[0]) :
            self.serverPort = int(t[1])
            found += 1
        if(2 == found) :
          break   
    except:
      print ('ERR no open source')
      return False
    return True
  #-----------
  def try_close(self) :
    if(None == self.s):
      return
    try:
      self.s.shutdown(SHUT_RDWR)
    except: 
      pass
    self.s.close()
    self.s = None
  #-----------
  def connect(self) :   
    result = False
    self.try_close()
    if(self.serverPort and not '' == self.serverHost) :
      try:
        self.s = socket(AF_INET, SOCK_STREAM)
        self.s.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        self.s.connect((self.serverHost, self.serverPort))
        self.s.send(bytes(self.part + '\n', 'utf-8'))
        responce = self.s.recv(1024)
        responce = responce.decode("utf-8")
        if(not responce == 'ok-' + self.part + '\n') :
          self.try_close()
        else :
          result = True
      except Exception as e: 
        print("ERR " + str(e))
        self.try_close()
    return result
  #-----------
  def send(self, data) :
    if(None == self.s) :
      if(not self.connect()) :
        print("Err no Conn\n")
        return False
    try:
      sent = self.s.sendall(data)
      if(None == sent) :
        return True
      else:
        self.try_close()
        return False
    except:
      self.try_close()
    return False
  #-----------
  def recv(self) :
    try:
      data = self.s.recv(1024)
      if(not data) :
        self.try_close()
        return None      
      else :        
        return data
    except:
      self.try_close()
    return None      
#----------------------------------------
def conv_input(data) :
  data = data.encode("utf-8")
  #data = data.encode("iso8859-1")
  data = base64.b64encode(data) + b'\n';
  return data
#----------------------------------------
def conv_output(data) :
  return (data.decode("utf-8")).strip()
#----------------------------------------
def try_send(conn, line) :
  if(conn.send(line)) :
    data = conn.recv()
    if(not None == data) :
      data = conv_output(data)
      print(data)
      return True
  conn.try_close()
  return False
#----------------------------------------
def perform_wrap_squid(conn) :
  #conn.connect()
  for line in sys.stdin:
    if(not line) :
      break
    line = line.strip()
    if(len(line) > 0) :
      line = conv_input(line)
      try:
        if(not try_send(conn, line)) :
          time.sleep(0.2)
          if(not try_send(conn, line)) :
            print("ERR no send\n")
            conn.try_close()
            #return
      except:
        pass
#----------------------------------------
def wrap_squid_base(helper_text) :
  with connection() as conn :
    if(conn.init(helper_text)):
      perform_wrap_squid(conn)
      conn.try_close()
#----------------------------------------
