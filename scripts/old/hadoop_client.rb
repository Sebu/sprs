#!/usr/bin/env ruby

filename = STDIN.read
exec "export LD_LIBRARY_PATH=~/git/diplom/cpp/epicore:~/playground/lib:/usr/local/lib; ~/git/diplom/cpp/epicli/epicli  -a 0.7 -e 0.3 -b 16 -i #{filename}"
puts "#{filename} -- done"


