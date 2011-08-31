#!/usr/bin/env perl

use strict;
use warnings;

my $i = 1;
while (<STDIN>) {
	chomp;
	print <<EOT;
			<Processor id="P_$i">
				<input>
					<queue/>
				</input>
				<threads>1</threads>
				<modules>
					<Module id="M_$i" lib="Save.la">
						<param name="filename" value="$_.hr"/>
						<param name="saveResourceType" value="false"/>
					</Module>
				</modules>
			</Processor>
EOT
	$i++;
}

for (my $j = 1; $j < $i; $j++) {
	print <<EOT;
					<nextProcessor ref="P_$j" filter="$j"/>
EOT
}
