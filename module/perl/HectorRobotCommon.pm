package HectorRobotCommon;

use warnings;
use strict;
use utf8;

# remove accents from the string
sub unaccent {
	my ($s) = @_;
	return $s unless ($s =~ m/[^[:ascii:]]/);
	$s =~ tr/ÀÁÂÃÄÅàáâãäåÇçČčĎďÈÉÊËèéêëĚěÌÍÎÏìíîïĽľŇňÑñÒÓÔÕÖØòóôõöøŘřŠšŤťÙÚÛÜùúûüŮůÝÿýŽž/AAAAAAaaaaaaCcCcDdEEEEeeeeEeIIIIiiiiLlNnNnOOOOOOooooooRrSsTtUUUUuuuuUuYyyZz/;
	my %trans = (
		'Æ' => 'AE',
		'æ' => 'ae',
		'Þ' => 'TH',
		'þ' => 'th',
		'Ð' => 'TH',
		'ð' => 'th',
		'ß' => 'ss'
	);
	$s =~ s/([ÆæÞþÐðß])/$trans{$1}/g;
	return $s;
}

1;
