#!/usr/bin/env ruby

require 'yaml'

yaml = YAML.load_file( '../tests/tests.yaml' )

test_name = ARGV[0] || "default"
cmd = ARGV[1] || "start"

test = yaml[test_name]


jobs_file = test["jobs"]
clients_file = test["clients"]
other_args = test["params"]
mode = test["mode"] || "train"

if clients_file.kind_of?(String) && File.exists?(clients_file)
  clients = `cat #{clients_file}`.split 
else
  clients = test["clients"] || ['c11']
end  

if jobs_file.kind_of?(String) && File.exists?(jobs_file)
  jobs = `cat #{jobs_file}`.split 
  merge_file = "#{jobs_file}.merge"
else
  jobs = test['jobs']
end

puts test_name
p test
jobs.zip(clients).each_with_index do |(job, client), index|
  dict = test["dict_name"] || "../../output/dicts/#{test_name}_#{index}.dict"
  client_cmd = "ssh #{client} -f --  ~/git/diplom/scripts/sprs_client.rb  --#{mode} #{job} --dict #{dict} #{other_args}" if (cmd=="start")
  client_cmd = "ssh #{client} -f -- killall sparsecli" if (cmd=="stop")
  client_cmd = "ssh #{client} -f -- killall -9 sparsecli" if (cmd=="kill")
  puts "submitting job #{job} to #{client} .. #{client_cmd}"
  system client_cmd
end


