#!/usr/bin/env perl

use strict;
use warnings;

my @lines;
while (<STDIN>) {
	chomp;
	push(@lines, $_);
}

print <<EOT;
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE Config SYSTEM "hector_config.dtd">
<!--
Split: split input file according to the TextResource TextId prefix.
-->
<Config>
	<Server id="split">
		<logConfig><![CDATA[
			log4j.rootLogger=DEBUG, LOGFILE
			log4j.appender.LOGFILE=org.apache.log4j.FileAppender
			log4j.appender.LOGFILE.File=deduplicated/split.log
			log4j.appender.LOGFILE.Append=false
			log4j.appender.LOGFILE.Threshold=DEBUG
			log4j.appender.LOGFILE.layout=org.apache.log4j.PatternLayout
			log4j.appender.LOGFILE.layout.ConversionPattern=%m%n ]]>
		</logConfig>
		<ProcessingEngine id="PE_main">
			<Processor id="P_main">
				<threads>1</threads>
				<modules>
					<Module id="M_input" lib="Load.la">
						<param name="filename" value="deduplicated/all.hr.gz"/>
						<param name="resourceType" value="TextResource"/>
						<param name="compress" value="true"/>
					</Module>
					<Module id="M_id" lib="IdPrefixToStatus.pm" type="perl">
						<param name="prefixRE" value="^data\/[^\/]+\/[^\/]+\/"/>
					</Module>
				</modules>
				<output>
EOT

for (my $i = 1; $i <= @lines; $i++) {
	print <<EOT;
					<nextProcessor ref="P_$i" filter="$i"/>
EOT
}
print <<EOT;
                                        <nextProcessor ref="P_default"/>
				</output>
			</Processor>
EOT

for (my $i = 0; $i < @lines; $i++) {
	my $j = $i+1;
	print <<EOT;
			<Processor id="P_$j">
				<input>
					<queue maxItems="100"/>
				</input>
				<threads>1</threads>
				<modules>
					<Module id="M_$j" lib="Save.la">
						<param name="filename" value="deduplicated/$lines[$i].hr.gz"/>
						<param name="saveResourceType" value="false"/>
						<param name="compress" value="true"/>
					</Module>
				</modules>
			</Processor>
EOT
}

print <<EOT;
			<Processor id="P_default">
                                <input>
                                        <queue maxItems="100"/>
                                </input>
				<threads>1</threads>
				<modules>
					<Module id="M_dump" lib="Dump.la"/>
					<Module id="M_output" lib="Null.la">
						<param name="moduleType" value="OUTPUT"/>
					</Module>
				</modules>
			</Processor>
		</ProcessingEngine>
	</Server>
</Config>
EOT
