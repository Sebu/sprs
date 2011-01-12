#!/usr/bin/env ruby

# puts ARGF.read

jobsext = ARGV[0]
clientsext = ARGV[1]
other_args = ARGV[2..-1].join(" ")
clients = `cat #{clientsext}`.split 
jobs = `cat #{jobsext}`.split 

jobs.zip(clients).each do |job, client| 
  cmd = "ssh #{client} -f --  ~/git/diplom/scripts/sprs_client.rb --train #{job} --dict #{job}.dict #{other_args}"
  puts "submitting job #{job} to #{client}.. #{cmd}"
  system cmd
end
