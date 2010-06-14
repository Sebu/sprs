#!/usr/bin/env ruby

file_name = ARGV[0]
max_error = ARGV[1] || 0.5
other_args = ARGV[2..-1]
puts file_name
exec "LD_LIBRARY_PATH=~/git/epitome/epicore:~/playground/lib:/usr/local/lib; ~/git/epitome/epicli/epicli -i #{file_name} -e #{max_error} #{other_args}"

