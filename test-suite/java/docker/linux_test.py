#!/usr/bin/env python

import logging
import os
import subprocess
import sys

log = logging.getLogger("djinni.linux_test")
log.setLevel(logging.INFO)
console_handler = logging.StreamHandler(sys.stderr)
log.addHandler(console_handler)

def run_in_shell(cmd):
  log.info("Running %s ..." % cmd)
  subprocess.check_call(cmd, shell=True)
  log.info("... done")

if __name__ == '__main__':
  BASE_PATH = os.path.abspath("test-suite/java/docker")
  assert \
    os.path.exists(os.path.join(BASE_PATH, "linux_test.py")), \
    "Please run this script from the root djinni directory"

  for entry in os.listdir(BASE_PATH):
    im_dir = os.path.join(BASE_PATH, entry)
    if not os.path.exists(os.path.join(im_dir, "Dockerfile")):
      continue

    im_name = "djinni_test." + entry
    run_in_shell("cd " + im_dir + " && docker build -t " + im_name + " .")
    run_in_shell(
	  "docker run -v " + os.path.abspath(".") + ":/opt/djinni " + im_name)

