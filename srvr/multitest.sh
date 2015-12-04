#!/bin/bash
for(( c=0; c<100; c++ ))
do
	curl http://192.168.0.14/stest.php
	#sleep 1
done
