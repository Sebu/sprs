#!/usr/bin/env ruby

# puts ARGF.read

ext = ARGV[0] || "schul"
max_error = ARGV[1] || 0.5
clients = `cat clients.#{ext}`.split 
jobs = `cat jobs`.split 

clients.zip(jobs).each do |client, job| 
  cmd = "ssh #{client} -f --  ~/git/epitome/client.rb #{job} #{max_error}"
  puts "submitting job #{job} to #{client}.. #{cmd}"
  system cmd
end
