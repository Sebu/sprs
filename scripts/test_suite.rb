#!/usr/bin/env ruby

require 'yaml'

yaml = YAML.load_file( '../tests/tests.yaml' )

test_name = ARGV[0] || "default"
cmd = ARGV[1] || "start"

test = yaml[test_name]

puts test_name
p test

jobs_file = test["params"]["train"]
clients_file = test["clients"]


if clients_file.kind_of?(String) && File.exists?(clients_file)
  clients = `cat #{clients_file}`.split 
else
  clients = test["clients"] || ['c11']
end  

if test['jobs']
	jobs = Array.new(test['jobs'])
elsif jobs_file.kind_of?(String) && File.exists?(jobs_file)
  jobs = test["params"]["train"] = `cat #{jobs_file}`.split 
	#  merge_file = "#{jobs_file}.merge"
end


jobs.zip(clients).each_with_index do |(job, client), index|
  if test["params"]["dict"].kind_of?(Array)
    dict = test["params"]["dict"][index]
  else
    dict = test["params"]["dict"] || "../../output/dicts_c31/#{test_name}_#{index}.dict"
  end
  
  if test["params"]["inputs"]
    st = test["params"]["inputs"].rindex("/")+1
    nd = test["params"]["inputs"].rindex(".")-1
    logfile = "../output/dicts/results/#{test_name}_#{index}_#{test["params"]["inputs"][st..nd]}.log"
  else
    logfile = "../output/dicts/#{test_name}_#{index}.log"
  end
   
  base_cmd = "ssh #{client} -f -- "
  other_args = "--dict #{dict}"
  # consume chains
  test["params"].each do |param, values|
    next if param=="dict" 
	  if values.kind_of?(Array) 
  		value = values[index] 
  	else
  		value = values 
  	end
  	other_args = "#{other_args} --#{param} #{value}"
  end
  client_cmd = "#{base_cmd}~/git/diplom/scripts/sprs_client.rb #{other_args} > #{logfile}" if (cmd=="start")
  client_cmd = "#{base_cmd}killall sparsecli" if (cmd=="stop")
  client_cmd = "#{base_cmd}killall -9 sparsecli" if (cmd=="kill")
  puts "submitting job #{job} to #{client} .. #{client_cmd}"
  system client_cmd
end


