#!/usr/bin/env ruby
exec "LD_LIBRARY_PATH=~/git/diplom/cpp/epicore:~/playground/lib:/usr/local/lib && ~/git/diplom/cpp/epicli/epicli #{ARGV.join(" ")}"

