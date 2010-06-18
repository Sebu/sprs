#!/usr/bin/env ruby

# puts ARGF.read

jobsext = ARGV[0] || "default"
clientsext = ARGV[1] || "schul"
other_args = ARGV[2..-1].join(" ")
clients = `cat clients.#{clientsext}`.split 
jobs = `cat jobs.#{jobsext}`.split 

jobs.zip(clients).each do |job, client| 
  cmd = "ssh #{client} -f --  ~/git/epitome/client.rb -i #{job} #{other_args}"
  puts "submitting job #{job} to #{client}.. #{cmd}"
  system cmd
end
