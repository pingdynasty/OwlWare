# -*- coding: utf-8 -*-

# Copyright 2014 Section6. All Rights Reserved.

import argparse
import json
import os
import requests
import sys
import urlparse

def main():
  parser = argparse.ArgumentParser(description='Upload Pd source files to Tannhäuser and return the compiled results.')
  parser.add_argument('srcDir', help='Pd source directory or file to compile.')
  parser.add_argument('outDir', help='The directory in which to save the compiled library.')
  parser.add_argument('--tannurl', default='http://tannhauserpd.section6.ch', help='The base url for Tannhäuser. Defaults to http://tannhauserpd.section6.ch.')
  parser.add_argument('--main', help='The top-level Pd patch. This option does nothing if srcDir is a file.')
  parser.add_argument('--platform', default='apple', help='The platform for which to compile.')
  parser.add_argument('--name', default='tann', help='The name of the library.')
  parser.add_argument('--arch', default=['armv7'], help='The list of architectures to compile for. May use "ios" and "osx", which also force the platform flag to "apple".', nargs='+')
  parser.add_argument('--getcsource', help='Also returns an archive of the original C source.', action='count')
  parser.add_argument('--getpdsource', help='Also returns an archive of the original Pd source.', action='count')
  parser.add_argument('-v', '--verbose', action='count')
  args = parser.parse_args()
  srcDir = args.srcDir

  if not os.path.isdir(args.outDir):
    print >> sys.stderr, "{0} must be a directory.".format(args.outDir)
    return


  # resolve macros for the arch argument in each case
  if args.platform == "compileonly":
    args.arch = [] # no architecture if we only compile
  else:
    if args.arch == ["ios"]:
      args.arch = ["armv7", "armv7s", "arm64"]
      args.platform = "apple"
    elif args.arch == ["osx"]:
      args.arch = ["i386", "x86_64"]
      args.platform = "apple"

  postData = {
    "name": args.name, # defaults to "tann"
    "platform": args.platform, # defaults to "apple"
    "arch": args.arch, # defaults to ["armv7"]
    "files": {}
  }
  files = postData["files"]

  # TODO(mhroth): do better error checking on input and outputs

  # if the path is a file
  if os.path.isfile(srcDir):
    # read the file
    postData["main"] = os.path.basename(srcDir)
    with open(srcDir, "r") as x:
      files[os.path.basename(srcDir)] = x.read()

  # if the path is a directory
  else:
    # add all pd files, recursively, in the given directory
    if args.main is None:
      raise Exception("No top-level Pd path given. Use --main option.")
    postData["main"] = os.path.relpath(args.main, start=srcDir)

    for (dirPath, dirNames, filenames) in os.walk(srcDir):
      for f in filenames:
        if f.endswith(".pd"):
          with open(os.path.join(dirPath, f) , "r") as x:
            files[os.path.join(os.path.relpath(dirPath, start=srcDir), f)] = x.read()

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
    if args.platform == "compileonly":
      get_file(args.tannurl, replyJson["publicId"], "c_src.zip", args.outDir)
    else:
      get_file(args.tannurl, replyJson["publicId"], replyJson["libname"], args.outDir)
      get_file(args.tannurl, replyJson["publicId"], replyJson["headername"], args.outDir)

      # allow the user to get the original C source
      if args.getcsource:
        get_file(args.tannurl, replyJson["publicId"], "c_src.pd", args.outDir)

    # allow the user to get the original Pd source
    if args.getpdsource:
      get_file(args.tannurl, replyJson["publicId"], "pd_src.pd", args.outDir)

  elif not args.verbose:
    print "Tannhäuser response:\n{0}".format(
        json.dumps(replyJson, sort_keys=True, indent=4, separators=(',', ': ')))

def get_file(base_url, public_id, file_name, out_dir):
  srcurl = urlparse.urljoin(base_url, "/t/{0}/{1}".format(public_id, file_name))
  r = requests.get(srcurl)
  if r.status_code == 200:
    with open(os.path.join(out_dir, file_name), 'wb') as f:
        for chunk in r.iter_content():
            f.write(chunk)
  else:
    print "Failed to get file \"{1}\" with HTTP status code {0}.".format(srcurl, r.status_code)

if __name__ == "__main__":
  main()
