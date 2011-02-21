#!/usr/bin/env ruby
exec "cd ~/git/diplom/cpp/sparsecli; export LD_LIBRARY_PATH=../sprscode:/homes/wheel/seb/playground/lib:/usr/local/lib; nice -n10 ./sparsecli #{ARGV.join(" ")}"


