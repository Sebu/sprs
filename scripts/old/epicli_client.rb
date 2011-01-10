#!/usr/bin/env ruby
exec "export DYLD_LIBRARY_PATH=~/git/diplom/cpp/epicore:~/playground/lib:/usr/local/lib; ~/git/diplom/cpp/epicli/epicli #{ARGV.join(" ")}"


