# -*- coding: utf-8 -*-

import argparse
import json
import os
import requests
import sys
import urlparse

def main():
  parser = argparse.ArgumentParser(description='Upload Pd source files to Tannhäuser and return the compiled results.')
  parser.add_argument('pdSrcDir', help='Pd source directory or file to compile.')
  parser.add_argument('libOutDir', help='The directory in which to save the compiled library.')
  parser.add_argument('--tannurl', default='http://tannhauserpd.section6.ch', help='The base url for Tannhäuser. Defaults to http://tannhauserpd.section6.ch.')
  parser.add_argument('--main', help='The top-level Pd patch. This option does nothing if pdSrcDir is a file.')
  parser.add_argument('--platform', default='apple', help='The platform for which to compile.')
  parser.add_argument('--name', default='tann', help='The name of the library.')
  parser.add_argument('--arch', default=['armv7'], help='The list of architectures to compile for.', nargs='+')
  parser.add_argument('--getsource', help='Also returns an archive of the original C source.', action='count')
  parser.add_argument('--verbose', '-v', action='count')
  args = parser.parse_args()
  pdSrcDir = args.pdSrcDir

  if not os.path.isdir(args.libOutDir):
    print >> sys.stderr, "{0} must be a directory.".format(args.libOutDir)
    return

  postData = {
    "name": args.name, # defaults to "tann"
    "platform": args.platform, # defaults to "apple"
    "arch": args.arch, # defaults to ["armv7"]
    "files": {}
  }
  files = postData["files"]

  # TODO(mhroth): do better error checking on input and outputs

  # if the path is a file
  if os.path.isfile(pdSrcDir):
    # read the file
    postData["main"] = os.path.basename(pdSrcDir)
    with open(pdSrcDir, "r") as x:
      files[os.path.basename(pdSrcDir)] = x.read()

  # if the path is a directory
  else:
    # add all pd files, recursively, in the given directory
    if args.main is None:
      raise Exception("No top-level Pd path given. Use --main option.")
    postData["main"] = os.path.relpath(args.main, start=pdSrcDir)

    for (dirPath, dirNames, filenames) in os.walk(pdSrcDir):
      for f in filenames:
        if f.endswith(".pd"):
          with open(os.path.join(dirPath, f) , "r") as x:
            files[os.path.join(os.path.relpath(dirPath, start=pdSrcDir), f)] = x.read()

  if args.verbose:
    print "Sending:\n{0}".format(json.dumps(postData, sort_keys=True, indent=4, separators=(',', ': ')))

  json_url = urlparse.urljoin(args.tannurl, "api")
  headers = {'content-type': 'application/json; charset=utf-8'}
  r = requests.post(json_url, data=json.dumps(postData), headers=headers)

  replyJson = r.json()

  # pretty print JSON response
  if args.verbose:
    print "Response:\n{0}".format(json.dumps(replyJson, sort_keys=True, indent=4, separators=(',', ': ')))

  if replyJson["status"] == "ok": # the reply from Rutger indicates that everything went well
    liburl = urlparse.urljoin(args.tannurl, replyJson["liburl"])
    r = requests.get(liburl)
    if r.status_code == 200:
      with open(os.path.join(args.libOutDir, replyJson["libname"]), 'wb') as f:
          for chunk in r.iter_content():
              f.write(chunk)
      if args.verbose:
        print "Result written to {0}".format(os.path.abspath(os.path.join(args.libOutDir, replyJson["libname"])))
    else:
      print "Failed to get {0} with HTTP status code {1}.".format(liburl, r.status_code)

    headerurl = urlparse.urljoin(args.tannurl, replyJson["headerurl"])
    r = requests.get(headerurl)
    if r.status_code == 200:
      with open(os.path.join(args.libOutDir, replyJson["headername"]), 'wb') as f:
          for chunk in r.iter_content():
              f.write(chunk)
      if args.verbose:
        print "Result written to {0}".format(os.path.abspath(os.path.join(args.libOutDir, replyJson["headername"])))
    else:
      print "Failed to get {0} with HTTP status code {1}.".format(headerurl, r.status_code)

    # allow the user to get the original C source
    if args.getsource:
      srcurl = urlparse.urljoin(args.tannurl, "/t/{0}/c_src.zip".format(replyJson["publicId"]))
      r = requests.get(srcurl)
      if r.status_code == 200:
        with open(os.path.join(args.libOutDir, "c_src.zip"), 'wb') as f:
            for chunk in r.iter_content():
                f.write(chunk)
      else:
        print "Failed to get C source with HTTP status code {1}.".format(r.status_code)
  elif not args.verbose:
    print "Tannhäuser response:\n{0}".format(json.dumps(replyJson, sort_keys=True, indent=4, separators=(',', ': ')))

if __name__ == "__main__":
  main()
