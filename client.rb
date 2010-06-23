#!/usr/bin/env ruby
exec "LD_LIBRARY_PATH=~/git/epitome/epicore:~/playground/lib:/usr/local/lib; ~/git/epitome/epicli/epicli #{ARGV.join(" ")}"


