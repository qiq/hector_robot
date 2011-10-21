#!/usr/bin/env bash

>evaluate_gcld.txt
>evaluate_top_words.txt
for l in af ar az be bg bn ca cs cy da de el en es et eu fa fi fr gl he hi hr hu id is it ka lt lv mk ml ms nl pl pt ro ru sk sl sq sr sv ta te th tr uk zh; do
#for l in cs de en sk; do
	hector_server -c Language.xml -fb test_gcld language_data/${l}_test.hr.gz /dev/null 2>&1 | cat >>evaluate_gcld.txt
	hector_server -c Language.xml -fb test_top_words language_data/${l}_test.hr.gz /dev/null language_data/top_words $l >>evaluate_top_words.txt 2>&1
done
rm -f /tmp/delme.$$
