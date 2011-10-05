#!/usr/bin/env bash

TOP_WORDS=mixed_top_words.txt

for i in 0 0.01 0.05 0.06 0.07 0.08 0.09 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6; do
	for j in 0.01 0.02; do
		hector_server -c Unaccented.xml -fb filter_unaccented cz_accented.hr.gz $TOP_WORDS $j $i >/tmp/delme.$$ 2>&1;
		a=`grep "M_wc1\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
		b=`grep "M_wc2\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
		accented=`echo -e "scale=2\n100*$b/$a"|bc`
		hector_server -c Unaccented.xml -fb filter_unaccented cz_unaccented.hr.gz $TOP_WORDS $j $i >/tmp/delme.$$ 2>&1;
		a=`grep "M_wc1\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
		b=`grep "M_wc2\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
		unaccented=`echo -e "scale=2\n100*$b/$a"|bc`
		echo -e "$i\t$accented\t$unaccented"
	done
done
rm -f /tmp/delme.$$
