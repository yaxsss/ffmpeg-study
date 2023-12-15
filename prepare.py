import os
import sys
executePath = os.getcwd()
rootDir = os.getcwd()
scriptPath = os.path.dirname(os.path.realpath(__file__))

def finish(code):
  global executePath
  os.chdir(executePath)
  sys.exit(code)

def error(text):
  print('[ERROR] ' + text)
  finish(1)

win = (sys.platform == 'win32')
mac = (sys.platform == 'darwin')
win32 = win and (os.environ['Platform'] == 'x86')
win64 = win and (os.environ['Platform'] == 'x64')

if win and not 'COMSPEC' in os.environ:
  error('COMSPEC environment variable is not set.')

if win and not win32 and not win64:
  error('Make sure to run from Native Tools Command Prompt.')

libsDir = os.path.realpath(os.path.join(rootDir, libsLoc))
thirdPartyDir = os.path.realpath(os.path.join(rootDir, 'ThirdParty'))
pathPrefixes = [
  'ThirdParty\\msys64\\mingw64\\bin'
] if win else [
  'ThirdParty/yasm'
]
pathSep = ';' if win else ':'
pathPrefix = ''
for prefix in pathPrefixes:
  pathPrefix = pathPrefix + os.path.join(rootDir, prefix) + pathSep

environment = {
  'USED_PREFIX': usedPrefix,
  'ROOT_DIR': rootDir,
  'LIBS_DIR': libsDir,
  'THIRDPARTY_DIR': thirdPartyDir,
  'PATH_PREFIX': pathPrefix,
}

def stage(name, commands, location = '.'):