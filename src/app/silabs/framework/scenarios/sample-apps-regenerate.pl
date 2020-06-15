#!/usr/bin/perl -w

# system() will NOT capture output and thus allow the real script to print out
# normally.  Previously this used backticks, which is not what we want.
system("../sample-apps/sample-apps-regenerate.pl " . join(" ", @ARGV));
