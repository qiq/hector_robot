#!/usr/bin/env bash

#for i in 0 0.01 0.05 0.06 0.07 0.08 0.09 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6; do
for i in 0.06 0.07 0.08 0.09; do
	hector_server -c Unaccented.xml -fb filter_unaccented_tagged cz_accented_tagged.hr.gz $i >/tmp/delme.$$ 2>&1;
	a=`grep "M_wc1\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
	b=`grep "M_wc2\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
	accented=`echo -e "scale=2\n100*$b/$a"|bc`
	hector_server -c Unaccented.xml -fb filter_unaccented_tagged cz_unaccented_tagged.hr.gz $i >/tmp/delme.$$ 2>&1;
	a=`grep "M_wc1\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
	b=`grep "M_wc2\[0\]: [0-9]" </tmp/delme.$$|cut -f 4`
	unaccented=`echo -e "scale=2\n100*$b/$a"|bc`
	echo -e "$i\t$accented\t$unaccented"
done
rm -f /tmp/delme.$$
