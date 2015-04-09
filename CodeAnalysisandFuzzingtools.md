# llvm for Ubuntu #

Download link is: http://llvm.org/releases/download.html.
Also one can install by typing "sudo apt-get install clang"
Another useful command is "scan-build make" (http://clang-analyzer.llvm.org/scan-build.html)

Note that GCC environmental variable in src/ae/daemon/Makefile.include should be set to /usr/bin/clang and run scan-build with the command line "scan-build -V --use-cc=/usr/bin/clang  make" in the src/ae/daemon directory to make just the daemon.  Using this static analysis tool found a few bugs in the source code.

# Android Lint #

The Android lint tool comes with the Android SDK.  It is a static code analysis tool that checks your Android project source files for potential bugs and optimization improvements for correctness, security, performance, usability, accessibility, and internationalization.

To run lint:
  1. _cd  ~/ae/all-eyes/src/aemgr_
  1. _lint -Wall -showall -html /tmp/AndroidLintReport.html aeManager_

Link: http://developer.android.com/tools/help/lint.html