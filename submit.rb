#!/usr/bin/env ruby

# puts ARGF.read

clients = `cat slaves`.split 
jobs = `cat jobs`.split 

clients.zip(jobs).each do |client, job| 
  cmd = "ssh #{client} -f --  ~/git/epitome/client.sh #{job}"
  puts "submitting job #{job} to #{client}.. #{cmd}"
  system cmd
end
