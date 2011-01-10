#!/bin/bash

HADOOP_HOME=~/uni/diplom/hadoop
$HADOOP_HOME/bin/hadoop  jar $HADOOP_HOME/contrib/streaming/hadoop-0.20.2-streaming.jar \
    -input input \
    -output output \
    -mapper ~/git/diplom/scripts/hadoop_client.rb \
    -jobconf mapred.map.tasks=1 \
    -jobconf mapred.job.name="epicli"
