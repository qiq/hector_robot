Package RobotCommon;

use warnings;
use strict;

# remove accents from the string
sub deaccent {
	my ($s) = @_;
	return $s unless ($s =~ m/[\xC0-\xFF]/);
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
